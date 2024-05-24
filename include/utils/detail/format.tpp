
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
        struct PlaceholderFormatter : public Formatter<T> {
            PlaceholderFormatter() : capacity(0u),
                                     start(1u),
                                     end(0u),
                                     specification_index(0u) {
            }
            
            PlaceholderFormatter(std::size_t spec_id) : capacity(0u),
                                                        start(1u),
                                                        end(0u),
                                                        specification_index(spec_id) {
            }
            
            bool initialized() const {
                return start <= end;
            }
            
            std::size_t capacity;
            
            std::size_t start;
            std::size_t end;

            std::size_t specification_index;
        };
        
        struct PlaceholderIndices {
            std::size_t argument_index;
            std::size_t formatter_index;
        };
    }
    
    template <typename ...Ts>
    FormatString FormatString::format(const Ts&... args) {
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
                        
                        utils::apply([&capacity, &formatters, &spec] <typename T, std::size_t I>(const T& value) {
                            detail::PlaceholderFormatter<T>& formatter = std::get<I>(formatters);
                            formatter.parse(spec);
                            if constexpr (detail::is_formattable_to<T>) {
                                formatter.capacity = formatter.reserve(value);
                                capacity += formatter.capacity;
                            }
                        }, tuple, i);
                    }
                    
                    // Increase capacity so that inserts can be done with as little additional memory allocations as possible
                    m_format.reserve(capacity);
                    std::size_t placeholder_offset = 0u;
                    
                    // Format placeholders
                    for (std::size_t i = 0u; i < argument_count; ++i) {
                        Placeholder& placeholder = m_placeholders[i];
                        
                        // Insert formatted placeholder
                        utils::apply([this, &formatters, &placeholder_offset, write_position = placeholder.position + placeholder_offset] <typename T, std::size_t I>(const T& value) {
                            detail::PlaceholderFormatter<T>& formatter = std::get<I>(formatters);
                            
                            if constexpr (detail::is_formattable_to<T>) {
                                std::size_t capacity = formatter.capacity;
                                
                                if (capacity > 0u) {
                                    // format_to needs a valid memory buffer to write to
                                    m_format.insert(write_position, capacity, '\0');
                                    FormattingContext context { capacity, &m_format[write_position] };
                                    formatter.format_to(value, context);
                                    
                                    // Cache start and end positions of resulting string so that future accesses to this formatter do not require re-formatting the value
                                    formatter.start = write_position;
                                    formatter.end = write_position + capacity;
                                    
                                    placeholder_offset += capacity;
                                }
                                else {
                                    // Formatter<T>::reserve returned 0, which is an invalid capacity, so fall back to using Formatter<T>::format instead
                                    logging::warning("performance implication");
                                    
                                    std::string result = std::move(formatter.format(value));
                                    std::size_t length = result.length();
                                    result.insert(write_position, result);
                                    
                                    formatter.start = write_position;
                                    formatter.end = write_position + length;
                                    
                                    placeholder_offset += length;
                                }
                            }
                            else {
                                // Formatter does not support reserve / format_to
                                logging::warning("performance implication");
                                
                                std::string result = std::move(formatter.format(value));
                                std::size_t length = result.length();
                                result.insert(write_position, result);
                                
                                formatter.start = write_position;
                                formatter.end = write_position + length;
                                
                                placeholder_offset += length;
                            }
                        }, tuple, i);
                        
                        placeholder.formatted = true;
                    }
                    
                    // Offset any remaining placeholders that were not formatted
                    for (std::size_t i = argument_count; i < placeholder_count; ++i) {
                        m_placeholders[i].position += placeholder_offset;
                    }
                }
                else {
                    std::size_t positional_argument_count = 0u;
                    
                    // Check: arguments for positional placeholders must come before any arguments for named placeholders
                    utils::apply([&positional_argument_count, positional_arguments_parsed = false]<typename T>(const T&, std::size_t index) mutable {
                        if constexpr (detail::is_named_argument<T>::value) {
                            if (!positional_arguments_parsed) {
                                positional_arguments_parsed = true;
                                positional_argument_count = index;
                            }
                            else {
                                // Encountered positional argument after named argument cutoff
                                throw FormattedError("invalid argument at position {} - arguments for positional placeholders must come before arguments for named placeholders", index);
                            }
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
                    // Key into FormatterGroup is the index of the Formatter to use (described below)
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
                                formatter.capacity = formatter.reserve(value);
                                capacity += formatter.capacity;
                            }
                        }, tuple, indices.argument_index);
                    }
                    
                    // Increase capacity so that inserts can be done with as little additional memory allocations as possible
                    m_format.reserve(capacity);
                    std::size_t placeholder_offset = 0u;
                    
                    // Format placeholders
                    for (std::size_t i = 0u; i < placeholder_count; ++i) {
                        detail::PlaceholderIndices& indices = placeholder_indices[i];
                        Placeholder& placeholder = m_placeholders[i];
                        
                        if (indices.argument_index == argument_count) {
                            // A value was not provided for this placeholder, formatting is a no-op
                            // Positions of placeholders that have not yet been formatted need to be adjusted so that future calls to format write placeholder values to the correct locations
                            placeholder.position += placeholder_offset;
                            continue;
                        }
                        
                        // Insert formatted placeholder
                        utils::apply([this, &formatters, &placeholder_offset, write_position = placeholder.position + placeholder_offset, formatter_index = indices.formatter_index] <typename T, std::size_t I>(const T& value) {
                            std::vector<detail::PlaceholderFormatter<T>>& placeholder_formatters = std::get<I>(formatters);
                            detail::PlaceholderFormatter<T>& formatter = placeholder_formatters[formatter_index];
                            
                            if constexpr (detail::is_formattable_to<T>) {
                                std::size_t capacity = formatter.capacity;
                                
                                if (capacity > 0u) {
                                    // Formatter<T>::format_to does not work properly when capacity is 0
                                    if (formatter.initialized()) {
                                        std::size_t num_characters = formatter.end - formatter.start;
                                        m_format.insert(write_position, m_format.substr(formatter.start, num_characters));
                                        placeholder_offset += num_characters;
                                    }
                                    else {
                                        // format_to needs a valid memory buffer to write to
                                        m_format.insert(write_position, capacity, '\0');
                                        FormattingContext context { capacity, &m_format[write_position] };
                                        formatter.format_to(value, context);
                                        
                                        // Cache start and end positions of resulting string so that future accesses to this formatter do not require re-formatting the value
                                        formatter.start = write_position;
                                        formatter.end = write_position + capacity;
                                        
                                        placeholder_offset += capacity;
                                    }
                                }
                                else {
                                    // Formatter<T>::reserve returned 0, which is an invalid capacity, so fall back to using Formatter<T>::format instead
                                    logging::warning("performance implication");
                                    
                                    std::string result = std::move(formatter.format(value));
                                    std::size_t length = result.length();
                                    result.insert(write_position, result);
                                    
                                    formatter.start = write_position;
                                    formatter.end = write_position + length;
                                    
                                    placeholder_offset += length;
                                }
                            }
                            else {
                                // Formatter does not support reserve / format_to
                                logging::warning("performance implication");
                                
                                std::string result = std::move(formatter.format(value));
                                std::size_t length = result.length();
                                result.insert(write_position, result);
                                
                                formatter.start = write_position;
                                formatter.end = write_position + length;
                                
                                placeholder_offset += length;
                            }
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
                                              m_group_size(0u) {
        static_assert(is_integer_type<T>::value, "value must be an integer type");
    }
    
    template <typename T>
    IntegerFormatter<T>::~IntegerFormatter() = default;
    
    template <typename T>
    void IntegerFormatter<T>::parse(const FormatString::Specification& spec) {
        if (spec.has_specifier("representation")) {
            std::string_view representation = spec["representation"];
        }
        
        if (spec.has_specifier("sign")) {
            std::string_view sign = spec["sign"];
        }
        
        if (spec.has_specifier("justification")) {
            std::string_view justification = spec["justification"];
        }
        
        if (spec.has_specifier("width")) {
        
        }

        if (spec.has_specifier("fill")) {
        
        }
        
        if (spec.has_specifier("padding")) {
        
        }
    }
    
    template <typename T>
    std::size_t IntegerFormatter<T>::reserve(T value) const {
        return 0;
    }
    
    template <typename T>
    std::string IntegerFormatter<T>::format(T value) const {
        int base;
        switch (m_representation) {
            case Representation::Decimal:
                base = 10;
                break;
            case Representation::Binary:
                base = 2;
                break;
            case Representation::Hexadecimal:
                base = 16;
                break;
        }

        return to_base(value, base);
    }
    
    template <typename T>
    std::string IntegerFormatter<T>::to_base(T value, int base) const {
        std::size_t capacity = 0u;
        std::size_t read_offset = 0u;
        std::size_t write_offset = 0u;

        std::string result;

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

        char buffer[sizeof(unsigned long long) * 8 + 1];
        char* start = buffer;
        char* end = buffer + sizeof(buffer) / sizeof(buffer[0]);

        const auto& [ptr, error_code] = std::to_chars(start, end, value, base);
        if (error_code == std::errc::value_too_large) {
            return "too large";
        }

        std::size_t num_characters_written = ptr - (start + read_offset);
        capacity += num_characters_written;

        char fill_character = ' ';
        if (m_fill) {
            fill_character = m_fill;
        }

        if (base == 10) {
            std::size_t group_size = 3u;
            if (m_separator) {
                capacity += num_characters_written / group_size - bool(num_characters_written % group_size == 0);
            }

            // Simplified case for decimal representations, since this representation does not support many of the available formatting specifiers.
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

            result.resize(capacity, fill_character);

            if (sign) {
                result[write_offset++] = sign[0];
            }

            if (m_separator) {
                std::size_t current = group_size - (num_characters_written % group_size);
                for (std::size_t i = 0u; i < num_characters_written; ++i, ++current) {
                    if (i && (current % group_size) == 0u) {
                        result[write_offset++] = m_separator;
                    }

                    result[write_offset++] = *(buffer + read_offset + i);
                }
            }
            else {
                for (start = buffer + read_offset; start != ptr; ++start) {
                    result[write_offset++] = *start;
                }
            }
        }
        else {
            std::size_t num_padding_characters = 0u;
            if (m_group_size) {
                // The final group may not be the same size as the ones that come before it
                std::size_t remainder = (num_characters_written % m_group_size);
                if (remainder) {
                    num_padding_characters += m_group_size - (num_characters_written % m_group_size);
                }

                // TODO: investigate
//                std::size_t precision = round_up_to_multiple(formatting.precision, formatting.group_size);
//
//                // Add an arbitrary number of padding characters to reach the value of precision
//                if (num_characters_written + num_padding_characters < precision) {
//                    num_padding_characters += precision - (num_characters_written + num_padding_characters);
//                }

                // The separator character is inserted before every group and must be accounted for
                // All except the first group
                capacity += (num_characters_written + num_padding_characters) / m_group_size - 1;
            }
            else {
//                if (num_characters_written < formatting.precision) {
//                    num_padding_characters = formatting.precision - num_characters_written;
//                }
            }

            capacity += num_padding_characters;

            if (m_use_base_prefix) {
                // +2 characters for base prefix '0b'
                capacity += 2;

                if (m_group_size) {
                    // +1 character for a separator between the groups and the base prefix
                    capacity += 1;
                }
            }

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

            result.resize(capacity, fill_character);

            char padding_character = '.';
            if (m_padding) {
                padding_character = m_padding;
            }

            char separator_character = ' ';
            if (m_separator) {
                separator_character = m_separator;
            }

            if (sign) {
                result[write_offset++] = sign[0];
            }

            if (m_use_base_prefix) {
                result[write_offset++] = '0';
                result[write_offset++] = 'b';

                if (m_group_size) {
                    result[write_offset++] = separator_character;
                }
            }

            if (m_group_size) {
                std::size_t current = 0u;

                for (std::size_t i = 0u; i < num_padding_characters; ++i, ++current) {
                    if (current && current % m_group_size == 0u) {
                        result[write_offset++] = separator_character;
                    }
                    result[write_offset++] = padding_character;
                }

                for (start = buffer + read_offset; start != ptr; ++start, ++current) {
                    if (current && current % m_group_size == 0u) {
                        result[write_offset++] = separator_character;
                    }

                    result[write_offset++] = *start;
                }
            }
            else {
                for (std::size_t i = 0u; i < num_padding_characters; ++i) {
                    result[write_offset++] = padding_character;
                }

                for (start = buffer + read_offset; start != ptr; ++start) {
                    result[write_offset++] = *start;
                }
            }
        }

        return std::move(result);
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