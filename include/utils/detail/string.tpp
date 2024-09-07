
#pragma once

#ifndef STRING_TPP
#define STRING_TPP

#include "utils/tuple.hpp"
#include "utils/logging.hpp"
#include "utils/assert.hpp"

#include <type_traits> // std::false_type, std::true_type
#include <limits> // std::numeric_limits
#include <charconv> // std::to_chars
#include <cmath> // std::log2, std::floor
#include <typeindex> // std::type_index

namespace utils {
    
    namespace detail {
        
        template <typename T>
        struct is_named_argument : std::false_type {
        };
        
        template <typename T>
        struct is_named_argument<NamedArgument<T>> : std::true_type {
        };
        
        template <typename T>
        struct PlaceholderFormatter : Formatter<T> {
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
        
        // 0 - left
        // 1 - right
        // 2 - center
        std::size_t apply_justification(std::uint8_t justification, char fill_character, std::size_t length, FormattingContext context);
        
        int round_up_to_multiple(int value, int multiple);
        char nibble_to_hexadecimal(const char nibble[4]);
        
        template <typename T>
        constexpr std::size_t count_digits(T num) {
            return (num < 10) ? 1 : 1 + count_digits(num / 10);
        }
        
        // Interface for global format strings
        void set_format(std::type_index type, const char* format);
        inline const char* get_format(std::type_index type);
        void clear_format(std::type_index type);
        
        template <typename T>
        const char* get_format() {
            return get_format(typeid(T));
        }
        
    }
    
    template <String T, String U>
    [[nodiscard]] bool icasecmp(const T& first, const U& second) {
        std::string_view a = first;
        std::string_view b = second;
        
        if (a.length() != b.length()) {
            return false;
        }
        
        for (std::size_t i = 0u; i < a.length(); ++i) {
            if (std::tolower(a[i]) != std::tolower(b[i])) {
                return false;
            }
        }
        
        return true;
    }
    
    template <String T, String U>
    [[nodiscard]] bool operator==(const T& first, const U& second) {
        return false;
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
            // Forward source location of the original string
            return { *this, m_source };
        }
        else {
            std::size_t argument_count = sizeof...(args);
            std::tuple<typename std::decay<const Ts>::type...> tuple = std::make_tuple(args...);
            
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
                    
                    std::tuple<Formatter<typename std::decay<const Ts>::type>...> formatters { };
                    std::size_t capacity = m_format.size();
                    
                    // Initialize formatters
                    for (std::size_t i = 0u; i < argument_count; ++i) {
                        const Identifier& identifier = m_identifiers[m_placeholders[i].identifier_index];
                        const Specification& spec = m_specifications[m_placeholders[i].specification_index];
                        
                        utils::apply([&capacity, &formatters, &parent = spec]<typename T, std::size_t I>(const T& value) {
                            Formatter<T>& formatter = std::get<I>(formatters);
                            
                            const FormatString& fmt = formatter.get_format_string();
                            for (const Placeholder& placeholder : fmt.m_placeholders) {
                                const Identifier& identifier = fmt.m_identifiers[placeholder.identifier_index];
                                const Specification& spec = fmt.m_specifications[placeholder.specification_index];
                                
                                formatter.parse(parent + spec, identifier);
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
                            if constexpr (is_formattable_to<T>) {
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
                            else if constexpr (is_formattable<T>) {
                                // The Formatter<T>::format function serves as a quick and dirty solution
                                // For optimal performance, Formatters should provide reserve / format_to, so write a log message to remind the user :)
                                logging::warning(FormatString("performance implication: cannot find reserve(...) / format_to(...) functions that match the expected syntax, using format(...) as a fallback", m_source));
                                
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
                            utils::apply_for([&outer, i]<typename U>(const U& inner, std::size_t j) {
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
                    std::tuple<std::vector<detail::PlaceholderFormatter<typename std::decay<const Ts>::type>>...> formatters { };
                    
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
                            if constexpr (is_formattable_to<T>) {
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
                            
                            if constexpr (is_formattable_to<T>) {
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
                            else if constexpr (is_formattable<T>){
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
        
        return { *this, m_source };
    }
    
    template <typename ...Ts>
    auto make_string_view_tuple(const Ts&... args) {
        return std::make_tuple(std::string_view(args)...);
    }
    
    template <String T, String ...Ts>
    const FormatString::Specification::Specifier& FormatString::Specification::one_of(const T& first, const Ts&... rest) const {
        if (sizeof...(Ts) == 1u) {
            return get_specifier(first);
        }
        
        constexpr std::size_t argument_count = sizeof...(Ts) + 1u;
        
        // Stored as a pointer to avoid copying specifiers
        std::pair<std::string_view, const Specifier*> specifiers[argument_count];
        
        // Index of the first valid specifier
        std::size_t index = argument_count; // Invalid index
        std::size_t valid_specifier_count = 0u;
        
        utils::apply([this, &specifiers, &index, &valid_specifier_count](std::string_view name, std::size_t i) {
            specifiers[i].first = name;
            
            if (has_specifier(name)) {
                if (index == argument_count) {
                    // Found the first valid specifier
                    // This index should only be set once
                    index = i;
                }
                
                specifiers[i].second = &get_specifier(name);
                ++valid_specifier_count;
            }
        }, make_string_view_tuple(first, rest...));
        
        if (valid_specifier_count == 0u) {
            // No valid specifiers found
            // Error message format:
            //   bad format specification access - no specifier values found for any of the following specifiers: {} (97 characters, not including placeholder braces)
            std::size_t capacity = 97u + (argument_count - 1u) * 2u;
            for (const auto& specifier : specifiers) {
                capacity += specifier.first.length();
            }

            std::string error;
            error.reserve(capacity);
            
            error += "bad format specification access - no specifier values found for any of the following specifiers: ";
            
            for (std::size_t i = 0u; i < argument_count; ++i) {
                const auto& specifier = specifiers[i];
                error += specifier.first;
                
                // Do not add a trailing comma
                if (i != argument_count) {
                    error += ", ";
                }
            }
            
            throw FormattedError(error);
        }
        else if (valid_specifier_count > 1u) {
            // Found multiple valid specifiers
            // Error message format:
            //   ambiguous format specification access - specification contains values for more than one of the following specifiers: {} (not found: {}) (131 characters, not including placeholder braces)
            std::size_t capacity = 131u + (argument_count - 1u) * 2u;
            for (const auto& specifier : specifiers) {
                if (specifier.second) {
                    capacity += specifier.first.length();
                }
            }
            
            std::string error;
            error.reserve(capacity);

            error += "ambiguous format specification access - specification contains values for more than one of the following specifiers: ";

            // Append comma-separated list of valid specifiers
            unsigned count = 0u;
            for (const auto& specifier : specifiers) {
                if (specifier.second) {
                    error += specifier.first;
                    ++count;
                    
                    // Do not add a trailing comma
                    if (count != valid_specifier_count) {
                        error += ", ";
                    }
                }
            }
            
            count = 0u;
            
            // Append comma-separated list of separators for which definitions were not found
            error += "(not found: ";
            for (const auto& specifier : specifiers) {
                if (!specifier.second) {
                    error += specifier.first;
                    ++count;
                    
                    // Do not add a trailing comma
                    if (count != argument_count - valid_specifier_count) {
                        error += ", ";
                    }
                }
            }
            error += ")";
            
            throw FormattedError(error);
        }

        return *specifiers[index].second;
    }
    
    template <String ...Ts>
    bool FormatString::Specification::has_specifier(const Ts& ...specifiers) const {
        return (has_specifier(std::string_view(specifiers)) || ...);
    }
    
    template <typename T>
    NamedArgument<T>::NamedArgument(std::string_view name, const T& value) : name(name), value(value) {
    }
    
    template <typename T>
    NamedArgument<T>::~NamedArgument() {
    }
    
    template <typename ...Ts>
    FormattedError::FormattedError(FormatString fmt, const Ts&... args) : std::runtime_error(fmt.format(args...)) {
    }

    template <typename ...Ts>
    FormatString format(FormatString fmt, const Ts&... args) {
        return fmt.format(args...);
    }
    
    // Section: IntegerFormatter
    
    template <typename T>
    IntegerFormatter<T>::IntegerFormatter() {
        static_assert(is_integer_type<T>::value, "value must be an integer type");
    }
    
    template <typename T>
    IntegerFormatter<T>::~IntegerFormatter() = default;
    
    template <typename T>
    void IntegerFormatter<T>::parse(const FormatString::Specification& spec) {
        ASSERT(spec.type() == FormatString::Specification::Type::SpecifierList, "format specification for integer values must be a list of specifiers");
        
        if (spec.has_specifier("representation")) {
            std::string_view value = trim(spec.get_specifier("representation").value);
            if (icasecmp(value, "decimal")) {
                representation = Representation::Decimal;
            }
            else if (icasecmp(value, "binary")) {
                representation = Representation::Binary;
            }
            else if (icasecmp(value, "hexadecimal")) {
                representation = Representation::Hexadecimal;
            }
            else {
                logging::warning("ignoring unknown representation specifier value: '{}' - expecting one of: decimal, binary, or hexadecimal (case-insensitive)", value);
            }
        }
        
        if (spec.has_specifier("sign")) {
            std::string_view value = trim(spec.get_specifier("sign").value);
            if (icasecmp(value, "negative only") || icasecmp(value, "negative_only") || icasecmp(value, "negativeonly")) {
                sign = Sign::NegativeOnly;
            }
            else if (icasecmp(value, "aligned")) {
                sign = Sign::Aligned;
            }
            else if (icasecmp(value, "both")) {
                sign = Sign::Both;
            }
            else {
                logging::warning("ignoring unknown sign specifier value: '{}' - expecting one of: negative only (variants: negative_only, negativeonly), aligned, or both (case-insensitive)", value);
            }
        }
        
        if (spec.has_specifier("justification", "justify", "alignment", "align")) {
            std::string_view value = trim(spec.one_of("justification", "justify", "alignment", "align").value);
            if (icasecmp(value, "left")) {
                justification = Justification::Left;
            }
            else if (icasecmp(value, "right")) {
                justification = Justification::Right;
            }
            else if (icasecmp(value, "center")) {
                justification = Justification::Center;
            }
            else {
                logging::warning("ignoring unknown justification specifier value: '{}' - expecting one of: left, right, or center (case-insensitive)", value);
            }
        }
        
        if (spec.has_specifier("width")) {
            std::string_view value = trim(spec.get_specifier("width").value);
            
            unsigned w;
            std::size_t num_characters_read = from_string(value, w);
            
            if (num_characters_read < value.length()) {
                logging::warning("ignoring invalid width specifier value: '{}' - specifier value must be an integer", value);
            }
            else {
                width = w;
            }
        }
        
        if (spec.has_specifier("fill", "fill_character", "fillcharacter")) {
            std::string_view value = trim(spec.one_of("fill", "fill_character", "fillcharacter").value);
            if (value.length() > 1u) {
                logging::warning("ignoring invalid fill character specifier value: '{}' - specifier value must be a single character", value);
            }
            else {
                fill_character = value[0];
            }
        }
        
        if (spec.has_specifier("use_separator", "useseparator", "use_separator_character", "useseparatorcharacter")) {
            std::string_view value = trim(spec.one_of("use_separator", "useseparator", "use_separator_character", "useseparatorcharacter").value);
            if (icasecmp(value, "true") || icasecmp(value, "1")) {
                use_separator_character = true;
            }
            else if (icasecmp(value, "false") || icasecmp(value, "0")) {
                use_separator_character = false;
            }
            else {
                logging::warning("ignoring unknown use_separator_character specifier value: '{}' - expecting one of: true / 1, false / 0 (case-insensitive)", value);
            }
        }
        
        if (spec.has_specifier("group_size", "groupsize")) {
            std::string_view value = trim(spec.one_of("group_size", "groupsize").value);
            
            unsigned gs;
            std::size_t num_characters_read = from_string(value, gs);
            
            if (num_characters_read < value.length()) {
                logging::warning("ignoring invalid group_size specifier value: '{}' - specifier value must be an integer", value);
            }
            else {
                group_size = gs;
            }
        }
        
        if (spec.has_specifier("use_base_prefix", "usebaseprefix")) {
            std::string_view value = trim(spec.one_of("use_base_prefix", "usebaseprefix").value);
            if (icasecmp(value, "true") || icasecmp(value, "1")) {
                use_base_prefix = true;
            }
            else if (icasecmp(value, "false") || icasecmp(value, "0")) {
                use_base_prefix = false;
            }
            else {
                logging::warning("ignoring unknown use_base_prefix specifier value: '{}' - expecting one of: true / 1, false / 0 (case-insensitive)", value);
            }
        }
        
        if (spec.has_specifier("digits")) {
            std::string_view value = trim(spec.get_specifier("digits").value);
            
            unsigned d;
            std::size_t num_characters_read = from_string(value, d);
            
            if (num_characters_read < value.length()) {
                logging::warning("ignoring invalid digits specifier value: '{}' - specifier value must be an integer", value);
            }
            else {
                digits = d;
            }
        }
    }

    template <typename T>
    std::string IntegerFormatter<T>::format(T value) const {
        switch (representation) {
            case Representation::Decimal:
                return to_decimal(value);
            case Representation::Binary:
                return to_binary(value);
            case Representation::Hexadecimal:
                return to_hexadecimal(value);
        }
    }
    
    template <typename T>
    std::string IntegerFormatter<T>::to_decimal(T value) const {
        // Compute the minimum number of characters to hold the formatted value
        std::size_t num_characters;
        
        // +1 character for sign
        if (value < 0) {
            num_characters = (std::log10(-value) + 1u) + 1u;
        }
        else {
            num_characters = (std::log10(value) + 1u) + int(sign != Sign::NegativeOnly);
        }
        
        bool _use_separator_character = use_separator_character && *use_separator_character;
        std::uint8_t _group_size = 3u;
        
        // Reserve capacity for separator characters (inserted between two groups)
        std::size_t num_separator_characters = 0u;
        if (_use_separator_character) {
            num_separator_characters = num_characters / _group_size;
            
            // Do not include a leading separator character if the number of characters is an even multiple of the group size
            if (num_characters && num_characters % _group_size == 0) {
                num_separator_characters -= 1u;
            }
        }
        
        std::size_t length = num_characters + num_separator_characters;
        std::size_t capacity = std::max(length, (std::size_t) width);
        std::string result(capacity, fill_character);
        
        std::size_t write_position;
        switch (justification) {
            case Justification::Left:
                write_position = 0u;
                break;
            case Justification::Right:
                write_position = capacity - length;
                break;
            case Justification::Center:
                write_position = (capacity - length) / 2;
                break;
        }
        
        char buffer[std::numeric_limits<T>::digits10 + (std::is_signed<T>::value ? 1 : 0)];
        char* start = buffer;
        char* end = buffer + sizeof(buffer) / sizeof(buffer[0]);
        const auto& [ptr, error_code] = std::to_chars(start, end, value, 10);
        
        if (value < 0) {
            result[write_position++] = '-';
            ++start; // Do not read negative sign from buffer
        }
        else {
            switch (sign) {
                case Sign::Aligned:
                    result[write_position++] = ' ';
                    break;
                case Sign::Both:
                    result[write_position++] = '+';
                    break;
                case Sign::NegativeOnly:
                default:
                    break;
            }
        }
        
        std::size_t num_characters_written = ptr - start;
        
        if (_use_separator_character) {
            std::size_t current = 3 - (num_characters_written % 3);
            for (std::size_t i = 0u; i < num_characters_written; ++i, ++current) {
                if (i && (current % 3) == 0u) {
                    result[write_position++] = ',';
                }

                result[write_position++] = *(start + i);
            }
        }
        else {
            // Copy contents directly
            result.replace(write_position, num_characters_written, start);
        }
    }
    
    template <typename T>
    std::string IntegerFormatter<T>::to_binary(T value) const {
        std::size_t num_characters;
        
        // Compute the minimum number of characters to hold the formatted value
        if (value < 0) {
            // Twos complement is used for formatting negative values, which by default uses as many digits as required by the system architecture
            num_characters = sizeof(T) * CHAR_BIT;
        }
        else {
            // The minimum number of digits required to format a binary number is log2(n) + 1
            num_characters = std::floor(std::log2(value)) + 1u;
        }
        
        std::size_t num_padding_characters = 0u;
        
        // The number of characters can be overridden by a user-specified 'digits' value
        // If the desired number of digits is smaller than the required number of digits, remove digits starting from the front (most significant) bits
        // If the desired number of digits is larger than the required number of digits, append digits to the front (1 for negative integers, 0 for positive integers)
        if (digits) {
            if (num_characters >= digits) {
                num_characters = digits;
            }
            else {
                // Append leading padding characters to reach the desired number of digits
                num_padding_characters = digits - num_characters;
            }
        }
        
        bool _use_separator_character = false;
        std::uint8_t _group_size = 0u;
        
        if (use_separator_character) {
            if (*use_separator_character) {
                if (group_size) {
                    _group_size = *group_size;
                    
                    if (*group_size) {
                        _use_separator_character = true;
                    }
                    else {
                        // Group size explicitly provided as 0, use of separator character is disabled
                        _use_separator_character = false;
                    }
                }
                else {
                    // Group size is 4 by default (if not specified)
                    _group_size = 4u;
                    _use_separator_character = true;
                }
            }
            else {
                // Use of separator character explicitly disabled
                _use_separator_character = false;
            }
        }
        else {
            // Use of separator character is disabled by default
            _use_separator_character = false;
        }
        
        // Reserve capacity for separator characters (inserted between two groups)
        std::size_t num_separator_characters = 0u;
        if (_use_separator_character) {
            num_separator_characters = num_characters / _group_size;
            
            // Do not include a leading separator character if the number of characters is an even multiple of the group size
            // Example: 0b'0000 should be 0b0000
            if (num_characters && num_characters % _group_size == 0) {
                num_separator_characters -= 1u;
            }
        }
        
        std::size_t length = num_characters + num_padding_characters + num_separator_characters;
        if (use_base_prefix) {
            // +2 characters for base prefix '0b'
            length += 2u;
        }

        std::size_t capacity = std::max(length, (std::size_t) width);
        std::string result(capacity, fill_character);
        
        std::size_t write_position;
        switch (justification) {
            case Justification::Left:
                write_position = 0u;
                break;
            case Justification::Right:
                write_position = capacity - length;
                break;
            case Justification::Center:
                write_position = (capacity - length) / 2;
                break;
        }
        
        // Convert value to binary
        char buffer[sizeof(T) * CHAR_BIT] { 0 };
        char* end = buffer;
        for (int i = 0; i < (int) num_characters; ++i, ++end) {
            int bit = (value >> (num_characters - 1 - i)) & 1;
            *end = (char) (48 + bit); // 48 is the ASCII code for '0'
        }
        
        if (use_base_prefix) {
            result[write_position++] = '0';
            result[write_position++] = 'b';
        }
        
        if (_use_separator_character) {
            std::size_t current = 0u;

            for (std::size_t i = 0u; i < num_padding_characters; ++i, ++current) {
                if (current && (num_characters - current) % _group_size == 0u) {
                    result[write_position++] = '\'';
                }
                
                result[write_position++] = value < 0 ? '1' : '0';
            }
            
            for (char* start = buffer; start != end; ++start, ++current) {
                if (current && (num_characters - current) % _group_size == 0u) {
                    result[write_position++] = '\'';
                }

                result[write_position++] = *start;
            }
        }
        else {
            for (std::size_t i = 0u; i < num_padding_characters; ++i) {
                result[write_position++] = value < 0 ? '1' : '0';
            }

            // Copy contents directly
            result.replace(write_position, end - buffer, buffer);
        }
        
        return std::move(result);
    }
    
    template <typename T>
    std::string IntegerFormatter<T>::to_hexadecimal(T value) const {
        std::size_t num_characters;
        
        // Compute the minimum number of characters to hold the formatted value
        if (value < 0) {
            // Twos complement is used for formatting negative values, which by default uses as many digits as required by the system architecture
            num_characters = sizeof(T) * CHAR_BIT / 4;
        }
        else {
            // The minimum number of digits required to format a binary number is log2(n) + 1
            // Each hexadecimal character represents 4 bits
            num_characters = std::floor(std::log2(value) / 4) + 1u;
        }
        
        std::size_t num_padding_characters = 0u;
        
        // The number of characters can be overridden by a user-specified 'digits' value
        // If the desired number of digits is smaller than the required number of digits, remove digits starting from the front (most significant) bits
        // If the desired number of digits is larger than the required number of digits, append digits to the front (1 for negative integers, 0 for positive integers)
        if (digits) {
            if (num_characters >= digits) {
                num_characters = digits;
            }
            else {
                // Append leading padding characters to reach the desired number of digits
                num_padding_characters = digits - num_characters;
            }
        }
        
        bool _use_separator_character = false;
        std::uint8_t _group_size = 0u;
        
        if (use_separator_character) {
            if (*use_separator_character) {
                if (group_size) {
                    _group_size = *group_size;
                    
                    if (*group_size) {
                        _use_separator_character = true;
                    }
                    else {
                        // Group size explicitly provided as 0, use of separator character is disabled
                        _use_separator_character = false;
                    }
                }
                else {
                    // Group size is 4 by default (if not specified)
                    _group_size = 4u;
                    _use_separator_character = true;
                }
            }
            else {
                // Use of separator character explicitly disabled
                _use_separator_character = false;
            }
        }
        else {
            // Use of separator character is disabled by default
            _use_separator_character = false;
        }
        
        // Reserve capacity for separator characters (inserted between two groups)
        std::size_t num_separator_characters = 0u;
        if (_use_separator_character) {
            num_separator_characters = (num_characters + num_padding_characters) / _group_size;
            
            // Do not include a leading separator character if the number of characters is an even multiple of the group size
            // Example: 0b'0000 should be 0b0000
            if ((num_characters + num_padding_characters) % _group_size == 0) {
                num_separator_characters -= 1u;
            }
        }
        
        std::size_t length = num_characters + num_padding_characters + num_separator_characters;
        
        if (use_base_prefix) {
            // +2 characters for base prefix '0x'
            length += 2u;
        }

        std::size_t capacity = std::max(length, (std::size_t) width);
        std::string result(capacity, fill_character);
        
        std::size_t write_position;
        switch (justification) {
            case Justification::Left:
                write_position = 0u;
                break;
            case Justification::Right:
                write_position = capacity - length;
                break;
            case Justification::Center:
                write_position = (capacity - length) / 2;
                break;
        }
        
        // Convert value to binary
        std::size_t num_characters_binary = num_characters * 4u;
        char buffer[sizeof(T) * CHAR_BIT] { '0' };
        char* end = buffer;
        for (int i = 0; i < (int) num_characters_binary; ++i, ++end) {
            int bit = (value >> (num_characters_binary - 1 - i)) & 1;
            *end = (char) (48 + bit); // 48 is the ASCII code for '0'
        }
        
        if (use_base_prefix) {
            result[write_position++] = '0';
            result[write_position++] = 'x';
        }
        
        char nibble[4] = { '0' };
        
        // Convert binary representation to hexadecimal
        // Pad first group binary representation if necessary, as this group might not be the same size as the rest
        // Example: 0b01001 has two groups (000)1 and 1001
        unsigned num_groups = num_characters_binary / 4u;
        unsigned remainder = num_characters_binary % 4u;
        unsigned num_padding_characters_binary = 0u;
        
        if (remainder) {
            // Number of binary characters does not equally divide into a hexadecimal representation, first group will require padding characters
            num_groups += 1u;
            num_padding_characters_binary = 4u - remainder;
        }
        
        if (_use_separator_character) {
            std::size_t current = 0u;

            // Append any extra padding characters
            for (std::size_t i = 0u; i < num_padding_characters; ++i, ++current) {
                if (current && (num_characters - current) % _group_size == 0u) {
                    result[write_position++] = '\'';
                }
                
                result[write_position++] = value < 0 ? 'F' : '0';
            }
            
            char* ptr = buffer;
            for (int group = 0; group < num_groups; ++group, ++current) {
                // Insert separator
                if (current && (num_characters - current) % _group_size == 0u) {
                    result[write_position++] = '\'';
                }

                // Convert binary to hexadecimal
                int i = 0;
                
                // Append any necessary padding characters to the front of the nibble
                if (num_padding_characters_binary) {
                    for (; i < num_padding_characters_binary; ++i) {
                        nibble[i] = value < 0 ? '1' : '0';
                    }
                    // Padding is only applicable to the first group
                    num_padding_characters_binary = 0u;
                }
                
                while (i != 4) {
                    nibble[i++] = *ptr++;
                }
                result[write_position++] = detail::nibble_to_hexadecimal(nibble);
            }
        }
        else {
            for (std::size_t i = 0u; i < num_padding_characters; ++i) {
                result[write_position++] = value < 0 ? 'F' : '0';
            }
            
            char* ptr = buffer;
            for (int group = 0; group < num_groups; ++group) {
                // Convert binary to hexadecimal
                int i = 0;
                
                // Append any necessary padding characters to the front of the nibble
                if (num_padding_characters_binary) {
                    for (; i < num_padding_characters_binary; ++i) {
                        nibble[i] = value < 0 ? '1' : '0';
                    }
                    // Padding is only applicable to the first group
                    num_padding_characters_binary = 0u;
                }
                
                while (i != 4) {
                    nibble[i++] = *ptr++;
                }
                result[write_position++] = detail::nibble_to_hexadecimal(nibble);
            }
        }
        
        return std::move(result);
    }
    
    template <typename T>
    FloatingPointFormatter<T>::FloatingPointFormatter() : representation(Representation::Fixed),
                                                          sign(Sign::NegativeOnly),
                                                          justification(Justification::Left),
                                                          width(0u),
                                                          fill_character(' '),
                                                          precision(0u){
        static_assert(is_floating_point_type<T>::value, "value must be a floating point type");
    }
    
    template <typename T>
    FloatingPointFormatter<T>::~FloatingPointFormatter() = default;
    
    template <typename T>
    void FloatingPointFormatter<T>::parse(const FormatString::Specification& spec) {
        if (spec.type() == FormatString::Specification::Type::FormattingGroupList) {
            throw FormattedError("format specification for floating point values must be a list of specifiers");
        }
        
        if (spec.has_specifier("representation")) {
            std::string_view value = trim(spec.get_specifier("representation").value);
            if (icasecmp(value, "fixed")) {
                representation = Representation::Fixed;
            }
            else if (icasecmp(value, "scientific")) {
                representation = Representation::Scientific;
            }
            else {
                logging::warning("ignoring unknown representation specifier value: '{}' - expecting one of: fixed, scientific (case-insensitive)", value);
            }
        }
        
        
        if (spec.has_specifier("sign")) {
            std::string_view value = trim(spec.get_specifier("sign").value);
            if (icasecmp(value, "negative only") || icasecmp(value, "negative_only") || icasecmp(value, "negativeonly")) {
                sign = Sign::NegativeOnly;
            }
            else if (icasecmp(value, "aligned")) {
                sign = Sign::Aligned;
            }
            else if (icasecmp(value, "both")) {
                sign = Sign::Both;
            }
            else {
                logging::warning("ignoring unknown sign specifier value: '{}' - expecting one of: negative only (variants: negative_only, negativeonly), aligned, or both (case-insensitive)", value);
            }
        }
        
        if (spec.has_specifier("justification", "justify", "alignment", "align")) {
            std::string_view value = trim(spec.one_of("justification", "justify", "alignment", "align").value);
            if (icasecmp(value, "left")) {
                justification = Justification::Left;
            }
            else if (icasecmp(value, "right")) {
                justification = Justification::Right;
            }
            else if (icasecmp(value, "center")) {
                justification = Justification::Center;
            }
            else {
                logging::warning("ignoring unknown justification specifier value: '{}' - expecting one of: left, right, or center (case-insensitive)", value);
            }
        }
        
        if (spec.has_specifier("width")) {
            std::string_view value = trim(spec.get_specifier("width").value);
            
            unsigned _width;
            std::size_t num_characters_read = from_string(value, _width);
            
            if (num_characters_read < value.length()) {
                logging::warning("ignoring invalid width specifier value: '{}' - specifier value must be an integer", value);
            }
            else {
                width = _width;
            }
        }
        
        if (spec.has_specifier("fill", "fill_character", "fillcharacter")) {
            std::string_view value = trim(spec.one_of("fill", "fill_character", "fillcharacter").value);
            if (value.length() > 1u) {
                logging::warning("ignoring invalid fill character specifier value: '{}' - specifier value must be a single character", value);
            }
            else {
                fill_character = value[0];
            }
        }
        
        if (spec.has_specifier("precision")) {
            std::string_view value = trim(spec.get_specifier("precision").value);
            
            unsigned _precision;
            std::size_t num_characters_read = from_string(value, _precision);
            
            if (num_characters_read < value.length()) {
                logging::warning("ignoring invalid precision specifier value: '{}' - specifier value must be an integer", value);
            }
            else {
                precision = _precision;
            }
        }
        
        if (spec.has_specifier("use_separator", "useseparator", "use_separator_character", "useseparatorcharacter")) {
            std::string_view value = trim(spec.one_of("use_separator", "useseparator", "use_separator_character", "useseparatorcharacter").value);
            if (icasecmp(value, "true") || icasecmp(value, "1")) {
                use_separator_character = true;
            }
            else if (icasecmp(value, "false") || icasecmp(value, "0")) {
                use_separator_character = false;
            }
            else {
                logging::warning("ignoring unknown use_separator_character specifier value: '{}' - expecting one of: true / 1, false / 0 (case-insensitive)", value);
            }
        }
    }
    
    template <typename T>
    std::string FloatingPointFormatter<T>::format(T value) const {
        std::size_t length = 0u;
        std::size_t read_offset = 0u;

        // Sign character
        length += value < 0 ? 1u : unsigned(sign != Sign::NegativeOnly);

        int num_significant_figures = std::numeric_limits<T>::digits10 + 1;
        if (precision) {
            num_significant_figures = precision;
        }

        std::chars_format format_flags = std::chars_format::fixed;
        if (representation == Representation::Scientific) {
            format_flags = std::chars_format::scientific;
        }
        constexpr int num_significant_digits = std::numeric_limits<T>::max_digits10; // Max number of significant digits
        constexpr int sign_character = 1; // 1 character for sign
        constexpr int decimal_character = 1; // 1 character for the decimal point
        constexpr int exponent_character = 1; // 'e' character
        constexpr int exponent_sign = 1; // '+' or '-' sign for exponent
        constexpr int exponent_digits = std::numeric_limits<T>::max_exponent10; // Max number of digits in the exponent
        
        char buffer[sign_character + num_significant_digits + decimal_character + exponent_character + exponent_sign + exponent_digits];
        char* start = buffer;
        char* end = buffer + sizeof(buffer) / sizeof(buffer[0]);

        // std::numeric_limits<T>::digits10 represents the number of decimal places that are guaranteed to be preserved when converted to text
        // Note: last decimal place will be rounded
        int conversion_precision = std::clamp(num_significant_figures, 0, std::numeric_limits<T>::digits10 + 1);
        const auto& [ptr, error_code] = std::to_chars(start, end, value, format_flags, conversion_precision);

        std::size_t num_characters_written = ptr - (start + read_offset);
        length += num_characters_written;

        // Additional precision
        length += std::max(0, num_significant_figures - conversion_precision);

        std::size_t decimal_position = num_characters_written;
        if (use_separator_character) {
            char* decimal = std::find(start + read_offset, ptr, '.');
            decimal_position = decimal - (start + read_offset);

            // Separators get inserted every 3 characters up until the position of the decimal point
            length += (decimal_position - 1) / 3;
        }
        
        std::size_t capacity = std::max(length, (std::size_t) width);
        std::string result(capacity, fill_character);
        
        std::size_t write_position;
        switch (justification) {
            case Justification::Left:
                write_position = 0u;
                break;
            case Justification::Right:
                write_position = capacity - length;
                break;
            case Justification::Center:
                write_position = (capacity - length) / 2;
                break;
        }
        
        if (value < 0) {
            result[write_position++] = '-';
        }
        else {
            switch (sign) {
                case Sign::Aligned:
                    result[write_position++] = ' ';
                    break;
                case Sign::Both:
                    result[write_position++] = '+';
                    break;
                case Sign::NegativeOnly:
                default:
                    break;
            }
        }

        if (representation == Representation::Scientific) {
            char* e = std::find(buffer, ptr, 'e');
            std::size_t e_position = e - (start + read_offset);

            for (std::size_t i = 0u; i < e_position; ++i) {
                result[write_position++] = *(buffer + read_offset + i);
            }

            // For scientific notation, fake precision must be appended before the 'e' denoting the exponent
            for (std::size_t i = conversion_precision; i < num_significant_figures; ++i) {
                result[write_position++] = '0';
            }

            for (start = buffer + read_offset + e_position; start != ptr; ++start) {
                result[write_position++] = *start;
            }
        }
        else {
            // Separator character only makes sense for fixed floating point values
            if (use_separator_character) {
                // Separators get inserted every 3 characters up until the position of the decimal point
                std::size_t group_size = 3;
                std::size_t counter = group_size - (decimal_position % group_size);

                // Write the number portion, up until the decimal point (with separators)
                for (std::size_t i = 0; i < decimal_position; ++i, ++counter) {
                    if (i && counter % group_size == 0u) {
                        result[write_position++] = ',';
                    }

                    result[write_position++] = *(buffer + read_offset + i);
                }

                // Write decimal portion
                for (start = buffer + read_offset + decimal_position; start != ptr; ++start) {
                    result[write_position++] = *start;
                }
            }
            else {
                for (start = buffer + read_offset; start != ptr; ++start) {
                    result[write_position++] = *start;
                }
            }

            // For regular floating point values, fake higher precision by appending the remaining decimal places as 0
            for (std::size_t i = conversion_precision; i < num_significant_figures; ++i) {
                result[write_position++] = '0';
            }
        }
        
        return std::move(result);
    }
    
    // StringFormatter
    template <typename T>
    StringFormatter<T>::StringFormatter() : justification(Justification::Left),
                                            width(0u),
                                            fill_character(' ') {
        static_assert(is_string_type<T>::value || is_character_type<T>::value, "value must be a string / character type");
    }
    
    template <typename T>
    StringFormatter<T>::~StringFormatter() = default;
    
    template <typename T>
    void StringFormatter<T>::parse(const FormatString::Specification& spec) {
        if (spec.type() == FormatString::Specification::Type::FormattingGroupList) {
            throw FormattedError("format specification for floating point values must be a list of specifiers");
        }
        
        if (spec.has_specifier("justification", "justify", "alignment", "align")) {
            std::string_view value = trim(spec.one_of("justification", "justify", "alignment", "align").value);
            if (icasecmp(value, "left")) {
                justification = Justification::Left;
            }
            else if (icasecmp(value, "right")) {
                justification = Justification::Right;
            }
            else if (icasecmp(value, "center")) {
                justification = Justification::Center;
            }
            else {
                logging::warning("ignoring unknown justification specifier value: '{}' - expecting one of: left, right, or center (case-insensitive)", value);
            }
        }
        
        if (spec.has_specifier("width")) {
            std::string_view value = trim(spec.get_specifier("width").value);
            
            unsigned w;
            std::size_t num_characters_read = from_string(value, w);
            
            if (num_characters_read < value.length()) {
                logging::warning("ignoring invalid width specifier value: '{}' - specifier value must be an integer", value);
            }
            else {
                width = w;
            }
        }
        
        if (spec.has_specifier("fill", "fill_character", "fillcharacter")) {
            std::string_view value = trim(spec.one_of("fill", "fill_character", "fillcharacter").value);
            if (value.length() > 1u) {
                logging::warning("ignoring invalid fill character specifier value: '{}' - specifier value must be a single character", value);
            }
            else {
                fill_character = value[0];
            }
        }
    }
    
    template <typename T>
    std::string StringFormatter<T>::format(const T& value) const {
        std::size_t capacity = format_to(value, nullptr);
        
        std::string result;
        result.resize(capacity);
        
        // Format indirectly to std::string to avoid double allocations
        FormattingContext context { capacity, result.data() };
        format_to(value, &context);
        
        return std::move(result);
    }
    
    template <typename T>
    std::size_t StringFormatter<T>::reserve(const T& value) const {
        return format_to(value, nullptr);
    }
    
    template <typename T>
    void StringFormatter<T>::format_to(const T& value, FormattingContext context) const {
        format_to(value, &context);
    }
    
    template <typename T>
    std::size_t StringFormatter<T>::format_to(const T& value, FormattingContext* context) const {
        std::size_t length;
        if constexpr (is_character_type<T>::value) {
            length = 1u;
        }
        else {
            length = std::string_view(value).length();
        }
        
        if (context) {
            FormattingContext result = *context;
            std::size_t write_position = detail::apply_justification(static_cast<typename std::underlying_type<StringFormatter<T>::Justification>::type>(justification), fill_character, length, result);
            
            if constexpr (is_character_type<T>::value) {
                result[write_position] = value; // Write char directly
            }
            else {
                for (std::size_t i = 0u; i < length; ++i) {
                    result[write_position + i] = value[i];
                }
            }
        }
        
        return std::max(length, (std::size_t) width);
    }
    
    // Section: standard containers
    
    // Default element formatting: { key: value }
    template <typename K, typename V, typename H, typename P, typename A>
    FormatString Formatter<std::unordered_map<K, V, H, P, A>>::m_default_format = "{{ {key}: {value} }}";
    
    template <typename K, typename V, typename H, typename P, typename A>
    Formatter<std::unordered_map<K, V, H, P, A>>::Formatter() : m_formatter() {
    }
    
    template <typename K, typename V, typename H, typename P, typename A>
    Formatter<std::unordered_map<K, V, H, P, A>>::~Formatter() = default;
    
    template <typename K, typename V, typename H, typename P, typename A>
    void Formatter<std::unordered_map<K, V, H, P, A>>::push_format(FormatString fmt) {
        // Remap placeholder names from std::unordered_map naming convention to std::pair naming convention
        fmt.rename_placeholder("key", "first");
        fmt.rename_placeholder("value", "second");
        m_formatter.set_format(fmt);
    }
    
    template <typename K, typename V, typename H, typename P, typename A>
    void Formatter<std::unordered_map<K, V, H, P, A>>::pop_format() {
    }
    
    template <typename K, typename V, typename H, typename P, typename A>
    void Formatter<std::unordered_map<K, V, H, P, A>>::parse(const FormatString::Specification& spec) {
        if (spec.empty()) {
            return;
        }
        
        if (spec.type() == FormatString::Specification::Type::SpecifierList) {
        }
        else {
            // Global formatting group is applied to the container
            if (spec.has_group(0)) {
                const FormatString::Specification& global = spec.get_formatting_group(0);
            }

            if (spec.has_group(1)) {
                m_formatter.parse(spec.get_formatting_group(1));
            }
        }
    }
    
    template <typename K, typename V, typename H, typename P, typename A>
    std::string Formatter<std::unordered_map<K, V, H, P, A>>::format(const T& value) const {
        if (value.empty()) {
            return "{ }";
        }
        
        // Format: { { key: value }, { key: value }, { key: value }, { key: value } }
        
        std::size_t num_elements = value.size();
        
        // Format std::unordered_map elements
        std::vector<std::string> elements;
        elements.reserve(num_elements);
        for (auto iter = std::begin(value); iter != std::end(value); ++iter) {
            elements.emplace_back(m_formatter.format(*iter));
        }

        // 2 characters for container opening / closing braces { }
        // 2 characters for leading space before the first element and trailing space after the last element
        // 2 characters for comma + space (between two elements)
        std::size_t length = 4u + (num_elements - 1u) * 2u;

        for (const std::string& element : elements) {
            length += element.length();
        }
        
        std::size_t capacity = std::max(length, width);
        std::string result(capacity, fill_character);
        
        // Apply justification
        std::size_t write_position;
        switch (justification) {
            case Justification::Left:
                write_position = 0u;
                break;
            case Justification::Right:
                write_position = capacity - length;
                break;
            case Justification::Center:
                write_position = (capacity - length) / 2;
                break;
        }
        
        result[write_position++] = '{';
        result[write_position++] = ' ';

        for (const std::string& element : elements) {
            length = element.length();
            result.replace(write_position, length, element);
            write_position += length;
    
            result[write_position++] = ',';
            result[write_position++] = ' ';
        }
        
        // Overwrite the last two characters to remove the trailing comma
        result[capacity - 2u] = ' ';
        result[capacity - 1u] = '}';
        
        return std::move(result);
    }
    
    template <typename K, typename V, typename H, typename P, typename A>
    void Formatter<std::unordered_map<K, V, H, P, A>>::format_to(const T& value, FormattingContext context) const {
        if (value.empty()) {
            context.insert(0, "{ }", 3);
        }
        else {
            std::size_t offset = 0u;
            
            // Opening brace
            context[offset++] = '{';
            context[offset++] = ' ';
            
            for (auto iter = std::begin(value); iter != std::end(value); ++iter) {
                std::size_t length = m_formatter.reserve(*iter);
                
                FormattingContext slice { length, &context[offset] };
                m_formatter.format_to(*iter, slice);
                
                offset += length;
                
                context[offset++] = ',';
                context[offset++] = ' ';
            }
            
            // Overwrite the last two characters to avoid having a trailing comma
            context[offset - 2u] = ' ';
            context[offset - 1u] = '}';
        }
    }
    
    template <typename T, typename U>
    Formatter<std::pair<T, U>>::Formatter() : m_formatters(),
                                              m_formats(1) {
        static_assert(is_formattable<T> || is_formattable_to<T>, "std::pair<T, U>: T must (at least) provide implementations for parse() and format()");
        static_assert(is_formattable<U> || is_formattable_to<U>, "std::pair<T, U>: U must (at least) provide implementations for parse() and format()");

        if constexpr (!is_formattable_to<T>) {
            // TODO: performance log message
        }
        if constexpr (!is_formattable_to<T>) {
            // TODO: performance log message
        }
    }
    
    template <typename T, typename U>
    Formatter<std::pair<T, U>>::~Formatter() = default;
    
    template <typename T, typename U>
    void Formatter<std::pair<T, U>>::parse(const FormatString::Specification& spec) {
        
        if (spec.empty()) {
            return;
        }
        
        FormatString::Specification::Type type = spec.type();
        
        if (type == FormatString::Specification::Type::SpecifierList) {
            // Specifier list provided, apply specifiers globally
        }
        else {
            m_format.update_placeholder("first", [](FormatString::Identifier& identifier, FormatString::Specification& spec, std::size_t position) {
                spec +=
            });
            
            if (spec.has_group(0)) {
                m_formatters.first.parse(spec.get_formatting_group(0));
            }
            
            if (spec.has_group(1)) {
                m_formatters.first.parse(spec.get_formatting_group(1));
            }
            
            // TODO: additional groups are not used
        }
    }
    
    template <typename T, typename U>
    std::string Formatter<std::pair<T, U>>::format(const std::pair<T, U>& value) const {
        FormatString fmt = m_format.empty() ? m_format : m_default;
        std::string formatted = fmt.format(NamedArgument<T>("first", value.first), NamedArgument<T>("second", value.second));
        
        // Apply justification, width, etc
        
        return std::move(formatted);
    }
    
    template <typename T>
    Formatter<NamedArgument<T>>::Formatter() : Formatter<T>() {
    }
    
    template <typename T>
    Formatter<NamedArgument<T>>::~Formatter() = default;
    
    template <typename T>
    void Formatter<NamedArgument<T>>::parse(const FormatString::Specification& spec) {
        Formatter<T>::parse(spec);
    }
    
    template <typename T>
    std::string Formatter<NamedArgument<T>>::format(const NamedArgument<T>& value) const requires is_formattable<T> {
        return Formatter<T>::format(value.value);
    }
    
    template <typename T>
    std::size_t Formatter<NamedArgument<T>>::reserve(const NamedArgument<T>& value) const requires is_formattable_to<T> {
        return Formatter<T>::reserve(value.value);
    }
    
    template <typename T>
    void Formatter<NamedArgument<T>>::format_to(const NamedArgument<T>& value, FormattingContext context) const requires is_formattable_to<T> {
        return Formatter<T>::format_to(value.value, context);
    }
    
}

#endif
