
#pragma once

#ifndef FORMAT_TPP
#define FORMAT_TPP

#include "utils/logging/logging.hpp"
#include "utils/constexpr.hpp"
#include "utils/string.hpp"
#include "utils/tuple.hpp"
#include "utils/assert.hpp"

#include <charconv> // std::to_chars
#include <queue> // std::priority_queue
#include <limits> // std::numeric_limits
#include <optional> // std::optional

namespace utils {

    namespace detail {
        
        template <typename T>
        struct is_named_argument : std::false_type { };
        
        template <typename T>
        struct is_named_argument<NamedArgument<T>> : std::true_type { };
        
        template <typename T>
        concept is_formattable = requires(Formatter<typename std::decay<T>::type> formatter, const FormatString::Specification& spec, const T& value) {
            { formatter.parse(spec) };
            { formatter.format(value) } -> std::same_as<std::string>;
        };
        
        template <typename T>
        concept is_formattable_to = requires(Formatter<typename std::decay<T>::type> formatter, const T& value, FormattingContext context) {
            { formatter.reserve(value) } -> std::same_as<std::size_t>;
            { formatter.format_to(value, context) };
        };
        
        template <typename T>
        struct PlaceholderFormatter : public Formatter<typename std::decay<T>::type> {
            PlaceholderFormatter() : length(0u),
                                     specification_index(0u),
                                     start(std::numeric_limits<std::size_t>::max()) {
            }
            
            PlaceholderFormatter(std::size_t spec_id) : length(0u),
                                                        specification_index(spec_id),
                                                        start(std::numeric_limits<std::size_t>::max()) {
            }
            
            bool initialized() const {
                return start != std::numeric_limits<std::size_t>::max();
            }
            
            std::size_t length;
            std::size_t specification_index;
            std::size_t start; // start + length is the formatted value
        };
        
        struct PlaceholderIndices {
            std::size_t argument_index;
            std::size_t formatter_index;
        };
        
        int round_up_to_multiple(int value, int multiple);
        
        template <typename T>
        std::string_view to_string_view(const T& value) {
            if constexpr (std::is_same<T, const char*>::value || std::is_same<T, char*>::value || std::is_same<T, std::string>::value) {
                return std::string_view(value);
            }
            else {
                return value;
            }
        }
        
        template <typename ...Ts>
        auto make_string_view_tuple(const Ts&... args) {
            return std::make_tuple(to_string_view(args)...);
        }
        
    }
    
    template <String T>
    FormatString::FormatString(T fmt, std::source_location source) : m_format(fmt),
                                                                     m_source(source) {
        parse();
    }
    
    template <typename ...Ts>
    FormatString FormatString::format(const Ts&... args) {
        // Note: arguments that are not used in the format string intentionally do not have warning messages emitted
        // This is a feature that can be used by other systems to insert additional data to format strings without requiring the user to explicitly provide values, which are instead provided by the internals of the system in question
        
        if constexpr (sizeof...(Ts) == 0u) {
            return { *this }; // Copy constructor
        }
        else {
            std::size_t argument_count = sizeof...(args);
            std::tuple<Ts...> tuple = std::make_tuple(args...);
            
            std::size_t placeholder_count = m_placeholders.size();
            
            // Providing fewer arguments than the number of placeholders is valid for both structured and unstructured format strings (placeholders missing arguments are simplified and included as-is in the resulting format string)
            if (!m_placeholders.empty()) {
                if (m_identifiers[m_placeholders[0].identifier_index].type == Identifier::Type::Auto) {
                    // Check: argument list must not contain any NamedArgument<T> types, as the format string is composed of only auto-numbered placeholders
                    utils::apply([]<typename T, std::size_t I>(const T& value) {
                        if constexpr (detail::is_named_argument<T>::value) {
                            throw FormattedError("invalid argument at position {} - named arguments are not allowed in format strings that only contain auto-numbered placeholders", I);
                        }
                    }, tuple);
                    
                    std::tuple<detail::PlaceholderFormatter<Ts>...> formatters { };
                    std::size_t capacity = m_format.size();
                    
                    // Initialize formatters
                    for (std::size_t i = 0u; i < argument_count; ++i) {
                        const Identifier& identifier = m_identifiers[m_placeholders[i].identifier_index];
                        const Specification& spec = m_specifications[m_placeholders[i].specification_index];
                        
                        utils::apply([&capacity, &formatters, &spec]<typename T, std::size_t I>(const T& value) {
                            detail::PlaceholderFormatter<T>& formatter = std::get<I>(formatters);
                            formatter.parse(spec);
                            if constexpr (detail::is_formattable_to<T>) {
                                formatter.length = formatter.reserve(value);
                                capacity += formatter.length;
                            }
                        }, tuple, i);
                    }
                    
                    // Increase capacity so that inserts can be done with as little additional memory allocations as possible
                    // Prefer inserts (despite needing to shift over characters) over allocating an entirely new buffer - this may need deeper profiling for optimizing runtime performance
                    m_format.reserve(capacity);
                    std::size_t inserted_placeholder_offset = 0u;
                    
                    // Format placeholders
                    for (std::size_t i = 0u; i < argument_count; ++i) {
                        Placeholder& placeholder = m_placeholders[i];
                        
                        // Insert formatted placeholder value
                        utils::apply([this, &formatters, &inserted_placeholder_offset, write_position = placeholder.position + inserted_placeholder_offset] <typename T, std::size_t I>(const T& value) {
                            detail::PlaceholderFormatter<T>& formatter = std::get<I>(formatters);
                            
                            std::size_t length;
                            if constexpr (detail::is_formattable_to<T>) {
                                length = formatter.length;
                                if (length > 0u) {
                                    // If a Formatter returns a valid capacity, adequate space for it will be reserved in the output string
                                    m_format.insert(write_position, length, '\0');
                                    FormattingContext context { length, &m_format[write_position] };
                                    formatter.format_to(value, context);
                                }
                                // Skip over formatting values for which the expected capacity is 0 characters
                                // else { ... }
                            }
                            else if constexpr (detail::is_formattable<T>) {
                                // The Formatter<T>::format function serves as a quick and dirty solution
                                // For optimal performance, Formatters should provide reserve / format_to, so write a log message to remind the user :)
                                logging::warning("performance implication: cannot find reserve(...) / format_to(...) functions that match the expected syntax, using format(...) as a fallback");
                                
                                std::string result = std::move(formatter.format(value));
                                length = result.length();
                                m_format.insert(write_position, result);
                            }
                            else {
                                // Well-defined custom Formatters must provide (at least) the Formatter<T>::format function
                                throw FormattedError("custom Formatter<T> type must provide (at least) a format(...) function");
                            }
                            
                            // Auto-numbered placeholder values do not share formatter data, no point in caching the start and end of the formatted values
                            inserted_placeholder_offset += length;
                        }, tuple, i);
                        
                        placeholder.formatted = true;
                    }
                    
                    // Offset any remaining placeholders that were not formatted
                    for (std::size_t i = argument_count; i < placeholder_count; ++i) {
                        m_placeholders[i].position += inserted_placeholder_offset;
                    }
                }
                else {
                    std::size_t positional_argument_count = 0u;
                    
                    // Check: arguments for positional placeholders must come before any arguments for named placeholders
                    utils::apply([&positional_argument_count, positional_arguments_parsed = false]<typename T>(const T&, std::size_t index) mutable {
                        if constexpr (detail::is_named_argument<T>::value) {
                            if (!positional_arguments_parsed) {
                                positional_arguments_parsed = true;
                            }
                        }
                        else {
                            if (positional_arguments_parsed) {
                                // Encountered positional argument after named argument cutoff
                                throw FormattedError("invalid argument at position {} - arguments for positional placeholders must come before arguments for named placeholders", index);
                            }
                            
                            ++positional_argument_count;
                        }
                    }, tuple);
                    
                    // Check: two NamedArgument<T> arguments should not reference the same named placeholder
                    utils::apply_for([&tuple, argument_count]<typename T>(const T& outer, std::size_t i) {
                        ASSERT(detail::is_named_argument<T>::value, "argument is not of type NamedArgument<T>");
                        
                        if constexpr (detail::is_named_argument<T>::value) {
                            utils::apply_for([&tuple, &outer, i]<typename U>(const U& inner, std::size_t j) {
                                ASSERT(detail::is_named_argument<U>::value, "argument is not of type NamedArgument<U>");
                                
                                if constexpr (detail::is_named_argument<U>::value) {
                                    if (outer.name == inner.name) {
                                        throw FormattedError("invalid argument at position {} - named arguments must be unique (argument for placeholder '{}' first encountered at position {})", j, inner.name, i);
                                    }
                                }
                            }, tuple, i + 1u, argument_count);
                        }
                    }, tuple, positional_argument_count, argument_count);
                    
                    std::size_t capacity = m_format.size();
                    
                    std::vector<detail::PlaceholderIndices> placeholder_indices;
                    placeholder_indices.resize(placeholder_count);
                    
                    for (const Placeholder& placeholder : m_placeholders) {
                        const Identifier& identifier = m_identifiers[placeholder.identifier_index];
                        const Specification& spec = m_specifications[placeholder.specification_index];
                        
                        std::size_t argument_index = argument_count; // Invalid index, represents an argument position or name that was not provided to format(...)
                        
                        if (identifier.type == Identifier::Type::Position) {
                            // Identifier (position) indicates the argument index to use when formatting
                            // It is possible that not all positional arguments will have values associated with them
                            if (identifier.position < positional_argument_count) {
                                argument_index = identifier.position;
                            }
                        }
                        else {
                            // Named arguments can be passed in an order that is different that how they are referenced in the format string, so it is first necessary to determine which placeholder is being referenced
                            utils::apply_for([&argument_index, &identifier]<typename T>(const T& value, std::size_t index) {
                                if constexpr (detail::is_named_argument<T>::value) {
                                    if (value.name == identifier.name) {
                                        argument_index = index;
                                    }
                                }
                            }, tuple, positional_argument_count, argument_count);
                        }
                        
                        detail::PlaceholderIndices& indices = placeholder_indices.emplace_back();
                        indices.argument_index = argument_index;
                    }
                    
                    // A placeholder can be referenced multiple times in the same format string with different format specifications
                    // Key into 'formatters' tuple is the argument index as provided to format(...)
                    // Key into the Formatter vector is the index of the Formatter to use (described below)
                    std::tuple<std::vector<detail::PlaceholderFormatter<Ts>>...> formatters { };
                    
                    // "this is a format string example: {0:representation=[binary]}, {0:[representation=[hexadecimal]}, {0:representation=[binary]}"
                    // The above format string requires two unique Formatters for positional placeholder 0
                    // The 'placeholder_formatter_indices' vector keeps track of the index into the FormatterGroup to use for the argument referenced by the placeholder
                    
                    // Initialize formatters
                    for (std::size_t i = 0u; i < placeholder_count; ++i) {
                        detail::PlaceholderIndices& indices = placeholder_indices[i];
                        
                        if (indices.argument_index == argument_count) {
                            // A value was not provided for this placeholder, no need to initialize a Formatter
                            continue;
                        }
                        
                        // Uniqueness for formatters (per placeholder) are determined by the specification index
                        std::size_t specification_index = m_placeholders[i].specification_index;
                        
                        const Identifier& identifier = m_identifiers[m_placeholders[i].identifier_index];
                        const Specification& spec = m_specifications[specification_index];
                        
                        utils::apply([&capacity, &formatters, &indices, &spec, specification_index] <typename T, std::size_t I>(const T& value) {
                            std::vector<detail::PlaceholderFormatter<T>>& placeholder_formatters = std::get<I>(formatters);
                            
                            for (std::size_t i = 0u; i < placeholder_formatters.size(); ++i) {
                                if (specification_index == placeholder_formatters[i].specification_index) {
                                    // Custom formatter for this format specification already exists, use it
                                    indices.formatter_index = i;
                                    return;
                                }
                            }
                            
                            indices.formatter_index = placeholder_formatters.size();
                            
                            // Initialize new formatter
                            detail::PlaceholderFormatter<T>& formatter = placeholder_formatters.emplace_back(specification_index);
                            formatter.parse(spec);
                            if constexpr (detail::is_formattable_to<T>) {
                                formatter.length = formatter.reserve(value);
                                capacity += formatter.length;
                            }
                        }, tuple, indices.argument_index);
                    }
                    
                    // Increase capacity so that inserts can be done with as little additional memory allocations as possible
                    m_format.reserve(capacity);
                    std::size_t inserted_placeholder_offset = 0u;
                    
                    // Format placeholders
                    for (std::size_t i = 0u; i < placeholder_count; ++i) {
                        detail::PlaceholderIndices& indices = placeholder_indices[i];
                        Placeholder& placeholder = m_placeholders[i];
                        
                        if (indices.argument_index == argument_count) {
                            // A value was not provided for this placeholder, formatting is a no-op
                            // Positions of placeholders that have not yet been formatted need to be adjusted so that future calls to format write placeholder values to the correct locations
                            placeholder.position += inserted_placeholder_offset;
                            continue;
                        }
                        
                        // Insert formatted placeholder value
                        utils::apply([this, &formatters, &inserted_placeholder_offset, write_position = placeholder.position + inserted_placeholder_offset, formatter_index = indices.formatter_index] <typename T, std::size_t I>(const T& value) {
                            std::vector<detail::PlaceholderFormatter<T>>& placeholder_formatters = std::get<I>(formatters);
                            detail::PlaceholderFormatter<T>& formatter = placeholder_formatters[formatter_index];
                            
                            if constexpr (detail::is_formattable_to<T>) {
                                if (formatter.length > 0u) {
                                    if (formatter.initialized()) {
                                        // Use cached result to avoid re-formatting, which is a potentially expensive operation
                                        m_format.insert(write_position, m_format.substr(formatter.start, formatter.length));
                                    }
                                    else {
                                        // Formatter<T>::format_to expects a valid buffer to write to
                                        // Memory for this buffer is already be accounted for, so this should not result in any additional memory allocations
                                        m_format.insert(write_position, formatter.length, '\0');
                                        FormattingContext context { formatter.length, &m_format[write_position] };
                                        formatter.format_to(value, context);
                                        
                                        // Cache start and end positions of resulting string for future accesses
                                        formatter.start = write_position;
                                    }
                                }
                                // Skip over formatting values for which the expected capacity is 0 characters
                                // else { ... }
                            }
                            else if constexpr (detail::is_formattable<T>){
                                // The Formatter<T>::format function serves as a quick and dirty solution
                                // For optimal performance, Formatters should provide reserve / format_to (write a log message to remind the user :) )
                                logging::warning("performance implication: cannot find reserve(...) / format_to(...) functions that match the expected syntax, using format(...) as a fallback");
                                
                                if (formatter.initialized()) {
                                    // Use cached result to avoid re-formatting, which is a potentially expensive operation
                                    m_format.insert(write_position, m_format.substr(formatter.start, formatter.length));
                                }
                                else {
                                    std::string result = std::move(formatter.format(value));
                                    m_format.insert(write_position, result);
                                    formatter.length = result.length(); // Cache the length of the result in Formatter<T>::length for later reuse
                                }
                            }
                            else {
                                // Well-defined custom Formatters must provide (at least) the Formatter<T>::format function
                                throw FormattedError("custom Formatter<T> type must provide (at least) a format(...) function");
                            }
                            
                            // The position a cached result is read from does not matter as it will always point to the same substring
                            formatter.start = write_position;
                            inserted_placeholder_offset += formatter.length;
                        }, tuple, indices.argument_index);
                        
                        placeholder.formatted = true;
                    }
                }
            }
            
            // Remove placeholders that have been formatted
            // This cannot be done with std::remove_if as it does not conserve the order of the elements
            auto it = std::remove_if(m_placeholders.begin(), m_placeholders.end(), [](const Placeholder& placeholder) {
                return placeholder.formatted;
            });
            m_placeholders.erase(it, m_placeholders.end());
        }
        
        return *this;
    }
    
    // Specification implementation
    
    template <typename T, typename ...Ts>
    bool FormatString::Specification::has_specifier(const T& first, const Ts& ...rest) const {
        if (std::holds_alternative<FormattingGroupList>(m_spec)) {
            // TODO: calling has_group on a formatting group list makes no sense, throw exception?
            return false;
        }
        
        auto tuple = detail::make_string_view_tuple(first, rest...);
        bool has_specifier = false;
        
        utils::apply([this, &has_specifier](std::string_view name, std::size_t index) {
            for (const Specifier& specifier : std::get<SpecifierList>(m_spec)) {
                if (casecmp(specifier.name, name)) {
                    has_specifier = true;
                }
            }
        }, tuple);
        
        return has_specifier;
    }
    
    template <typename T, typename ...Ts>
    FormatString::Specification::SpecifierView FormatString::Specification::one_of(const T& first, const Ts&... rest) const {
        constexpr std::size_t argument_count = sizeof...(Ts) + 1u;
        
        auto tuple = detail::make_string_view_tuple(first, rest...);
        SpecifierView specifier_views[argument_count];
        
        utils::apply([this, &specifier_views](std::string_view name, std::size_t index) {
            specifier_views[index].name = name;
            if (has_specifier(name)) {
                specifier_views[index].value = get_specifier(name);
            }
        }, tuple);
        
        // Remove duplicates
//        std::remove_if(std::begin(specifier_views), std::end(specifier_views), [](const SpecifierView& first, const SpecifierView& second) {
//            return first.name == second.name;
//        });
        
        std::size_t index = argument_count; // Invalid index
        bool multiple_definitions = false;
        for (std::size_t i = 0u; i < argument_count; ++i) {
            if (!specifier_views[i].value.empty()) {
                if (index == argument_count) {
                    index = i;
                }
                else {
                    multiple_definitions = true;
                    break;
                }
            }
        }
        
        if (multiple_definitions) {
            std::size_t capacity = 0u;
            utils::apply([&capacity](std::string_view name) {
                // Include space for quotes
                capacity += name.length();
            }, tuple);
            
            std::string error;
            error.reserve(capacity + (argument_count - 1u) * 2u);
            
            utils::apply([&error, argument_count](std::string_view name, std::size_t index) {
                error.append("\"").append(name.data(), name.length()).append("\"");
                if (index < argument_count) {
                    error.append(", ");
                }
            }, tuple);
        
            throw FormattedError("bad and/or ambiguous format specification access - specification contains values for more than one of the following specifiers: {}", error);
        }

        return specifier_views[index];
    }
    
    template <typename T>
    NamedArgument<T>::NamedArgument(std::string name, const T& value) : name(std::move(name)),
                                                                        value(value) {
    }

    template <typename T>
    NamedArgument<T>::~NamedArgument() = default;
    
    template <typename ...Ts>
    FormatString format(FormatString fmt, const Ts&... args) {
        return std::move(fmt.format(args...));
    }
    
    template <typename ...Ts>
    FormattedError::FormattedError(FormatString fmt, const Ts&... args) : std::runtime_error(fmt.format(args...)) {
    }
    
    // IntegerFormatter implementation
    
    template <typename T>
    IntegerFormatter<T>::IntegerFormatter() : m_representation(Representation::Decimal),
                                              m_sign(Sign::NegativeOnly),
                                              m_justification(Justification::Left),
                                              m_width(0u),
                                              m_fill(0),
                                              m_padding(0),
                                              m_separator(0),
                                              m_use_base_prefix(false),
                                              m_group_size(0u),
                                              m_precision(0u) {
        static_assert(is_integer_type<T>::value, "value must be an integer type");
    }
    
    template <typename T>
    IntegerFormatter<T>::~IntegerFormatter() = default;
    
    template <typename T>
    void IntegerFormatter<T>::parse(const FormatString::Specification& spec) {
        if (spec.type() == FormatString::Specification::Type::FormattingGroupList) {
            throw FormattedError("format specification for integer values must be a list of specifiers");
        }
        
        if (spec.has_specifier("representation")) {
            std::string_view value = spec["representation"];
            if (casecmp(value, "decimal")) {
                set_representation(Representation::Decimal);
            }
            else if (casecmp(value, "binary")) {
                set_representation(Representation::Binary);
            }
            else if (casecmp(value, "hexadecimal")) {
                set_representation(Representation::Hexadecimal);
            }
            else if (casecmp(value, "bitset")) {
                set_representation(Representation::Bitset);
            }
            else {
                logging::warning("ignoring unknown 'representation' specifier value: '{}'", value);
            }
        }
        
        if (spec.has_specifier("sign")) {
            std::string_view value = spec["sign"];
            if (casecmp(value.data(), "negative only") || casecmp(value.data(), "negative_only")) {
                set_sign(Sign::NegativeOnly);
            }
            else if (casecmp(value, "aligned")) {
                set_sign(Sign::Aligned);
            }
            else if (casecmp(value, "both")) {
                set_sign(Sign::Both);
            }
            else if (casecmp(value, "none")) {
                set_sign(Sign::None);
            }
            else {
                logging::warning("ignoring unknown 'sign' specifier value: '{}'", value);
            }
        }
        
        if (spec.has_specifier("justification", "justify", "align")) {
            FormatString::Specification::SpecifierView view = spec.one_of("justification", "justify", "align");
            
            if (casecmp(view.value, "left")) {
                set_justification(Justification::Left);
            }
            else if (casecmp(view.value, "right")) {
                set_justification(Justification::Right);
            }
            else if (casecmp(view.value, "center")) {
                set_justification(Justification::Center);
            }
            else {
                logging::warning("ignoring unknown '{}' specifier value: '{}'", view.name, view.value);
            }
        }
        
        if (spec.has_specifier("use_base_prefix")) {
            std::string_view value = spec["use_base_prefix"];
            if (casecmp(value, "true") || casecmp(value, "1")) {
                m_use_base_prefix = true;
            }
            else if (casecmp(value, "false") || casecmp(value, "0")) {
                // Support explicitly disabling base prefix
                m_use_base_prefix = false;
            }
            else {
                logging::warning("ignoring unknown '' specifier value: '{}'", value);
            }
        }
        
        if (spec.has_specifier("width")) {
            std::string_view value = spec["width"];
            std::size_t num_characters_read = from_string(value, m_width);
            
            if (num_characters_read < value.length()) {
                // Specifier value includes a non-integer character
                logging::warning("encountered invalid character '{}' at position {} of 'width' specifier value - using {} for width", value, num_characters_read, m_width);
            }
        }
        
        if (spec.has_specifier("group", "group_size")) {
            std::string_view value = spec["group_size"];
            std::size_t num_characters_read = from_string(value, m_group_size);
            
            if (num_characters_read < value.length()) {
                // Specifier value includes a non-integer character
                logging::warning("encountered invalid character '{}' at position {} of 'group_size' specifier value - using {} for group size", value, num_characters_read, m_width);
            }
        }
        
        if (spec.has_specifier("precision")) {
            std::string_view value = spec["precision"];
            std::size_t num_characters_read = from_string(value, m_precision);
            
            if (num_characters_read < value.length()) {
                // Specifier value includes a non-integer character
                logging::warning("encountered invalid character '{}' at position {} of 'precision' specifier value - using {} for precision", value, num_characters_read, m_width);
            }
        }

        if (spec.has_specifier("fill", "fill_character")) {
            FormatString::Specification::SpecifierView view = spec.one_of("fill", "fill_character");
            
            if (view.value.length() > 1u) {
                logging::warning("too many characters in '{}' specifier value '{}' - using '{}' as fill character", view.name, view.value, view.value[0]);
            }

            m_fill = view.value[0];
        }
        
        if (spec.has_specifier("padding", "padding_character")) {
            FormatString::Specification::SpecifierView view = spec.one_of("padding", "padding_character");
            
            if (view.value.length() > 1u) {
                logging::warning("too many characters in '{}' specifier value '{}' - using '{}' as padding character", view.name, view.value, view.value[0]);
            }

            m_padding = view.value[0];
        }
        
        if (spec.has_specifier("separator", "separator_character")) {
            FormatString::Specification::SpecifierView view = spec.one_of("separator", "separator_character");
            
            if (view.value.length() > 1u) {
                logging::warning("too many characters in '{}' specifier value '{}' - using '{}' as separator character", view.name, view.value, view.value[0]);
            }

            m_separator = view.value[0];
        }
    }

    template <typename T>
    std::string IntegerFormatter<T>::format(T value) const {
        int base = get_base();
        
        std::size_t capacity = to_base(value, base, nullptr);
        std::string result(capacity, 0);
        
        FormattingContext context { capacity, &result[0] };
        to_base(value, get_base(), &context);
        
        return std::move(result);
    }

    template <typename T>
    std::size_t IntegerFormatter<T>::reserve(T value) const {
        return to_base(value, get_base(), nullptr);
    }
    
    template <typename T>
    void IntegerFormatter<T>::format_to(T value, FormattingContext& context) const {
        to_base(value, get_base(), &context);
    }
    
    template <typename T>
    void IntegerFormatter<T>::set_representation(Representation representation) {
        m_representation = representation;
        
        if (m_representation == Representation::Bitset) {
            m_sign = Sign::None;
            m_use_base_prefix = false;
        }
    }
    
    template <typename T>
    IntegerFormatter<T>::Representation IntegerFormatter<T>::get_representation() const {
        return m_representation;
    }
    
    template <typename T>
    void IntegerFormatter<T>::set_sign(Sign sign) {
    
    }
    
    template <typename T>
    IntegerFormatter<T>::Sign IntegerFormatter<T>::get_sign() const {
        return m_sign;
    }
    
    template <typename T>
    void IntegerFormatter<T>::set_justification(Justification justification) {
    }
    
    template <typename T>
    IntegerFormatter<T>::Justification IntegerFormatter<T>::get_justification() const {
        return m_justification;
    }
    
    template <typename T>
    void IntegerFormatter<T>::set_width() const {
    }

    template <typename T>
    std::size_t IntegerFormatter<T>::get_width() const {
        return m_width;
    }
    
    template <typename T>
    void IntegerFormatter<T>::set_fill_character(char fill) {
    }

    template <typename T>
    char IntegerFormatter<T>::get_fill_character() const {
        return m_fill;
    }

    template <typename T>
    void IntegerFormatter<T>::set_padding_character(char padding) {
    }

    template <typename T>
    char IntegerFormatter<T>::get_padding_character() const {
        return m_padding;
    }
    
    template <typename T>
    void IntegerFormatter<T>::set_separator_character(char padding) {
    }

    template <typename T>
    char IntegerFormatter<T>::get_separator_character() const {
        return m_separator;
    }

    template <typename T>
    void IntegerFormatter<T>::use_base_prefix(bool use) {
    }
    
    template <typename T>
    bool IntegerFormatter<T>::use_base_prefix() const {
        return m_use_base_prefix;
    }
    
    template <typename T>
    void IntegerFormatter<T>::set_group_size(std::size_t group_size) {
    }

    template <typename T>
    std::size_t IntegerFormatter<T>::get_group_size() const {
        return m_group_size;
    }

    template <typename T>
    void IntegerFormatter<T>::set_precision(std::size_t precision) {
    }

    template <typename T>
    std::size_t IntegerFormatter<T>::get_precision() const {
        return m_precision;
    }
    
    template <typename T>
    std::size_t IntegerFormatter<T>::to_base(T value, int base, FormattingContext* context) const {
        std::size_t capacity = 0u;
        std::size_t read_position = 0u;
        
        char sign_character = 0;
        if (value < 0) {
            read_position = 1u; // Do not read negative sign in resulting buffer
            if (m_sign != Sign::None) {
                sign_character = '-';
                ++capacity;
            }
        }
        else {
            switch (m_sign) {
                case Sign::Aligned:
                    sign_character = ' ';
                    ++capacity;
                    break;
                case Sign::Both:
                    sign_character = '+';
                    ++capacity;
                    break;
                default:
                    break;
            }
        }
        
        // Architecture + space for negative sign
        char buffer[sizeof(unsigned long long) * 8 + 1] { 0 };
        char* start = buffer;
        char* end = buffer + sizeof(buffer) / sizeof(buffer[0]);

        const auto& [ptr, error_code] = std::to_chars(start, end, value, base);
        if (error_code == std::errc::value_too_large) {
            throw FormattedError("value too large to serialize (integer overflow)");
        }

        std::size_t num_characters_written = ptr - (start + read_position);
        capacity += num_characters_written;
        
        // Simplified case for decimal representations, since this representation does not support many of the available formatting specifiers
        if (base == 10) {
            // Group size is always 3 for decimal representation
            std::size_t group_size = 3u;
            if (m_separator) {
                capacity += num_characters_written / group_size - int(num_characters_written % group_size == 0);
            }
            
            // Resulting formatted string should only be generated if a valid context is provided
            if (context) {
                FormattingContext& result = *context;
                std::size_t write_position = apply_justification(capacity, result);
                
                if (sign_character) {
                    result[write_position++] = sign_character;
                }
                
                if (m_separator) {
                    std::size_t current = m_group_size - (num_characters_written % m_group_size);
                    for (std::size_t i = 0u; i < num_characters_written; ++i, ++current) {
                        if (i && (current % m_group_size) == 0u) {
                            result[write_position++] = m_separator;
                        }
    
                        result[write_position++] = *(buffer + read_position + i);
                    }
                }
                else {
                    for (start = buffer + read_position; start != ptr; ++start) {
                        result[write_position++] = *start;
                    }
                }
            }
        }
        else {
            std::size_t num_padding_characters = 0u;
            
            if (m_group_size) {
                // Last group may not be the same size as the other groups
                num_padding_characters += m_group_size - (num_characters_written % m_group_size);
                
                // Add characters to reach the desired precision
                if (num_characters_written + num_padding_characters < m_precision) {
                    num_padding_characters += detail::round_up_to_multiple(m_precision - (num_characters_written + num_padding_characters), m_group_size);
                }
                
                // Separator character is inserted between two groups
                capacity += (num_characters_written + num_padding_characters) / m_group_size - 1u;
            }
            else {
                if (num_characters_written < m_precision) {
                    num_padding_characters = m_precision - num_characters_written;
                }
            }

            capacity += num_padding_characters;
            
            if (m_use_base_prefix) {
                ASSERT(representation != Representation::Bitset, "bitset representations should not use base prefixes");
                
                // +2 characters for base prefix
                capacity += 2u;
                
                if (m_group_size) {
                    // +1 character for a separator between the groups and the base prefix
                    capacity += 1u;
                }
            }
            
            if (context) {
                FormattingContext& result = *context;
                std::size_t write_position = apply_justification(capacity, result);
                
                char padding_character = '.';
                if (m_padding) {
                    padding_character = m_padding;
                }
                
                char separator_character = ' ';
                if (m_separator) {
                    separator_character = m_separator;
                }
                
                if (sign_character) {
                    result[write_position++] = sign_character;
                }
                
                if (m_use_base_prefix) {
                    result[write_position++] = '0';
                    
                    if (base == 2) {
                        result[write_position++] = 'b';
                    }
                    else {
                        result[write_position++] = 'x';
                    }
    
                    if (m_group_size) {
                        result[write_position++] = separator_character;
                    }
                }
                
                if (m_group_size) {
                    std::size_t current = 0u;
    
                    for (std::size_t i = 0u; i < num_padding_characters; ++i, ++current) {
                        if (current && current % m_group_size == 0u) {
                            result[write_position++] = separator_character;
                        }
                        result[write_position++] = padding_character;
                    }
    
                    for (start = buffer + read_position; start != ptr; ++start, ++current) {
                        if (current && current % m_group_size == 0u) {
                            result[write_position++] = separator_character;
                        }
    
                        result[write_position++] = *start;
                    }
                }
                else {
                    for (std::size_t i = 0u; i < num_padding_characters; ++i) {
                        result[write_position++] = padding_character;
                    }
    
                    for (start = buffer + read_position; start != ptr; ++start) {
                        result[write_position++] = *start;
                    }
                }
            }
        }
        
        if (capacity < m_width) {
            capacity = m_width;
        }
        
        return capacity;
    }
    
    template <typename T>
    int IntegerFormatter<T>::get_base() const {
        switch (m_representation) {
            case Representation::Decimal:
                return 10;
            case Representation::Binary:
            case Representation::Bitset:
                return 2;
            case Representation::Hexadecimal:
                return 16;
            default:
                // Should never happen
                throw FormattedError("unknown representation in IntegerFormatter");
        }
    }
    
    template <typename T>
    std::size_t IntegerFormatter<T>::apply_justification(std::size_t capacity, FormattingContext& context) const {
        std::size_t start = 0u;

        char fill_character = ' ';
        if (m_fill) {
            fill_character = m_fill;
        }
        
        if (capacity < m_width) {
            switch (m_justification) {
                case Justification::Left:
                    for (std::size_t i = capacity; i < m_width; ++i) {
                        context[i] = fill_character;
                    }
                    break;
                case Justification::Right:
                    start = (m_width - 1u) - capacity;
                    for (std::size_t i = 0u; i < start; ++i) {
                        context[i] = fill_character;
                    }
                    break;
                case Justification::Center:
                    start = (m_width - capacity) / 2;
                    
                    // Left side
                    for (std::size_t i = 0u; i < start; ++i) {
                        context[i] = fill_character;
                    }
                    
                    // Right side (account for additional character in the case that width is odd)
                    std::size_t last = context.length() - 1u;
                    for (std::size_t i = 0u; i < start + ((m_width - capacity) % 2); ++i) {
                        context[last - i] = fill_character;
                    }
                    break;
            }
        }
        
        return start;
    }
 
    // FloatingPointFormatter implementation
    
    template <typename T>
    FloatingPointFormatter<T>::FloatingPointFormatter() : m_representation(Representation::Fixed),
                                                          m_sign(Sign::NegativeOnly),
                                                          m_justification(Justification::Left),
                                                          m_width(0u),
                                                          m_fill(0),
                                                          m_precision(0u),
                                                          m_separator(0) {
        static_assert(is_floating_point_type<T>::value, "value must be a floating point type");
    }
    
    template <typename T>
    FloatingPointFormatter<T>::~FloatingPointFormatter() = default;
    
    template <typename T>
    void FloatingPointFormatter<T>::parse(const FormatString::Specification& spec) {
    }

    template <typename T>
    std::string FloatingPointFormatter<T>::format(T value) {
        std::size_t capacity = 0u;
        std::size_t read_offset = 0u;

        const char* sign = nullptr;
        if (value < 0) {
            read_offset = 1u; // Do not read negative sign in resulting buffer

            if (m_sign != Sign::None) {
                sign = "-";
                ++capacity;
            }
        }
        else {
            switch (m_sign) {
                case Sign::Aligned:
                    sign = " ";
                    ++capacity;
                    break;
                case Sign::Both:
                    sign = "+";
                    ++capacity;
                    break;
                default:
                    break;
            }
        }

        int precision = 6;
        if (m_precision) {
            precision = m_precision;
        }

        std::chars_format format_flags = std::chars_format::fixed;
        if (m_representation == Representation::Scientific) {
            format_flags = std::chars_format::scientific;
        }

        // Buffer must be large enough to store:
        //  - the number of digits in the largest representable number (max_exponent10)
        //  - decimal point
        //  - highest supported precision for the given type (max_digits10)
        char buffer[std::numeric_limits<T>::max_exponent10 + 1 + std::numeric_limits<T>::max_digits10];
        char* start = buffer;
        char* end = buffer + sizeof(buffer) / sizeof(buffer[0]);

        // std::numeric_limits<T>::digits10 represents the number of decimal places that are guaranteed to be preserved when converted to text
        // Note: last decimal place will be rounded
        int conversion_precision = std::clamp(precision, 0, std::numeric_limits<T>::digits10);
        const auto& [ptr, error_code] = std::to_chars(start, end, value, format_flags, conversion_precision);

        if (error_code == std::errc::value_too_large) {
            return "too large";
        }

        std::size_t num_characters_written = ptr - (start + read_offset);
        capacity += num_characters_written;

        // Additional precision
        capacity += std::max(0, precision - conversion_precision);

        std::size_t decimal_position = num_characters_written;
        if (m_separator) {
            char* decimal = std::find(start + read_offset, ptr, '.');
            decimal_position = decimal - (start + read_offset);

            // Separators get inserted every 3 characters up until the position of the decimal point
            capacity += (decimal_position - 1) / 3;
        }

        char fill_character = ' ';
        if (m_fill) {
            fill_character = m_fill;
        }

        std::size_t write_offset = 0u;
        if (capacity < m_width) {
            switch (m_justification) {
                case Justification::Right:
                    write_offset = m_width - capacity;
                    break;
                case Justification::Center:
                    write_offset = (m_width - capacity) / 2;
                    break;
                default:
                    break;
            }

            capacity = m_width;
        }

        std::string result;
        result.resize(capacity, fill_character);

        if (sign) {
            result[write_offset++] = sign[0];
        }

        if (m_representation == Representation::Scientific) {
            char* e = std::find(buffer, ptr, 'e');
            std::size_t e_position = e - (start + read_offset);

            for (std::size_t i = 0u; i < e_position; ++i) {
                result[write_offset++] = *(buffer + read_offset + i);
            }

            // For scientific notation, fake precision must be appended before the 'e' denoting the exponent
            for (std::size_t i = conversion_precision; i < precision; ++i) {
                result[write_offset++] = '0';
            }

            for (start = buffer + read_offset + e_position; start != ptr; ++start) {
                result[write_offset++] = *start;
            }
        }
        else {
            // Separator character only makes sense for fixed floating point values
            char separator_character = ' ';
            if (m_separator) {
                separator_character = m_separator;

                // Separators get inserted every 3 characters up until the position of the decimal point
                std::size_t group_size = 3;
                std::size_t counter = group_size - (decimal_position % group_size);

                // Write the number portion, up until the decimal point (with separators)
                for (std::size_t i = 0; i < decimal_position; ++i, ++counter) {
                    if (i && counter % group_size == 0u) {
                        result[write_offset++] = separator_character;
                    }

                    result[write_offset++] = *(buffer + read_offset + i);
                }

                // Write decimal portion
                for (start = buffer + read_offset + decimal_position; start != ptr; ++start) {
                    result[write_offset++] = *start;
                }
            }

            // For regular floating point values, fake higher precision by appending the remaining decimal places as 0
            for (std::size_t i = conversion_precision; i < precision; ++i) {
                result[write_offset++] = '0';
            }
        }

        return std::move(result);
    }
    
    // StringFormatter
    template <typename T>
    StringFormatter<T>::StringFormatter() : m_justification(Justification::Left),
                                            m_width(0u),
                                            m_fill(0) {
        static_assert(is_string_type<T>::value, "value must be a string type");
    }
    
    template <typename T>
    StringFormatter<T>::~StringFormatter() = default;
    
    template <typename T>
    void StringFormatter<T>::parse(const FormatString::Specification& spec) {
    }
    
    template <typename T>
    std::string StringFormatter<T>::format(const T& value) const {
        std::size_t length = 0u;
        
        if constexpr (std::is_same<typename std::decay<T>::type, const char*>::value) {
            length = strlen(value);
        }
        else {
            // std::string_view, std::string
            length = value.length();
        }
        
        if (m_width < length) {
            // Justification is a noop
            return std::string(value);
        }
        else {
            std::size_t write_offset;
            switch (m_justification) {
                case Justification::Left:
                    write_offset = 0u; // Default is left-justified
                    break;
                case Justification::Right:
                    write_offset = m_width - length;
                    break;
                case Justification::Center:
                    write_offset = (m_width - length) / 2;
                    break;
            }

            char fill_character = ' ';
            if (m_fill) {
                fill_character = m_fill;
            }

            std::string result;
            result.resize(m_width, fill_character);

            for (std::size_t i = 0u; i < length; ++i) {
                result[write_offset + i] = value[i];
            }

            return std::move(result);
        }
    }
    
}


#endif // FORMAT_TPP