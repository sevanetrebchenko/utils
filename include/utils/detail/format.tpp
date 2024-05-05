
#pragma once

#ifndef FORMAT_TPP
#define FORMAT_TPP

#include "utils/logging/logging.hpp"
#include "utils/constexpr.hpp"

#include <charconv> // std::to_chars
#include <queue> // std::priority_queue

namespace utils {

    namespace detail {
        
        template <typename T>
        struct is_named_argument : std::false_type { };
        
        template <typename T>
        struct is_named_argument<NamedArgument<T>> : std::true_type { };
        
        template <typename T>
        concept is_formattable = requires(Formatter<T> formatter, const FormatString::Specification& spec) {
            { formatter.parse(spec) };
        };
        
    }
    
    template <typename ...Ts>
    std::string FormatString::format(const Ts& ...args) const {
        if constexpr (sizeof...(Ts) == 0u) {
            return m_result;
        }
        else {
            std::string result = m_result;
            
            if (!m_identifiers.empty()) {
                auto tuple = std::make_tuple(args...);
        
                if (m_identifiers[0].type == Identifier::Type::Auto) {
                    // For unstructured format strings, there is a 1:1 correlation between a placeholder value and its insertion point.
                    // Hence, the number of arguments provided to format(...) should be at least as many as the number of placeholders.
                    // Note: while it is valid to provide more arguments than necessary, these arguments will be ignored.
                    std::size_t placeholder_count = get_placeholder_count();
                    if (placeholder_count > sizeof...(args)) {
                        // Not enough arguments provided to format(...);
                        throw FormattedError("expecting {} arguments, but received {}", placeholder_count, sizeof...(args));
                    }
                }
                else {
                    // Format string should only contain positional / named placeholders.
                    std::size_t required_positional_placeholder_count = get_positional_placeholder_count();
                    std::size_t required_named_placeholder_count = get_named_placeholder_count();
        
                    if (required_positional_placeholder_count + required_named_placeholder_count > sizeof...(args)) {
                        // Not enough arguments provided to format(...);
                        throw FormattedError("expecting {} arguments, but received {}", required_positional_placeholder_count + required_named_placeholder_count, sizeof...(args));
                    }
        
                    // All positional placeholders must come before any named placeholders.
                    for (std::size_t i = 0u; i < required_positional_placeholder_count; ++i) {
                        auto is_structured_type = []<typename T>(const T& value) -> std::string_view {
                            if constexpr (detail::is_named_argument<T>::value) {
                                return value.name;
                            }
                            return ""; // Empty names are not allowed.
                        };
                        
//                        std::string_view name = runtime_get(tuple, i, is_structured_type);
//                        if (!name.empty()) {
//                            throw FormattedError("expecting value for positional placeholder {}, but received value for named placeholder '{}' - all positional placeholders values must come before any named placeholder values", i, name);
//                        }
                    }
        
                    // Check positional placeholder indices and warn on arguments not being used due to gaps in the positional placeholder values.
                    std::vector<bool> is_placeholder_used { };
                    is_placeholder_used.resize(required_positional_placeholder_count, false);
                    
                    for (const Identifier& identifier : m_identifiers) {
                        if (identifier.type == Identifier::Type::Position) {
                            is_placeholder_used[identifier.position] = true;
                        }
                    }
                    
                    for (std::size_t i = 0u; i < required_positional_placeholder_count; ++i) {
                        if (!is_placeholder_used[i]) {
                            logging::warning("positional placeholder '{}' is never referenced (value is never used)", i);
                        }
                    }
                    
                    // Verify that all named placeholders have values provided
                    for (const Identifier& identifier : m_identifiers) {
                        if (identifier.type == Identifier::Type::Name) {
                            std::string_view name = identifier.name;
                            bool found = false;
                            
                            for (std::size_t i = 0u; i < sizeof...(args); ++i) {
//                                found |= runtime_get(tuple, i, [name]<typename T>(const T& value) -> bool {
//                                    if constexpr (detail::is_named_argument<T>::value) {
//                                        return value.name == name;
//                                    }
//                                    return false;
//                                });
                            }
                            
                            if (!found) {
                                throw FormattedError("missing value for named placeholder '{}'", name);
                            }
                        }
                    }
                }
                
                // Format all unique placeholders once to save on computation power.
                // This is only really applicable for positional / named placeholder values, since these can be referenced multiple times
                // in the format string, but this logic is near identical for auto-numbered placeholder values.
                std::size_t unique_placeholder_count = get_placeholder_count();
        
                std::vector<std::string> formatted_placeholders;
                formatted_placeholders.reserve(unique_placeholder_count);
        
                for (std::size_t i = 0u; i < unique_placeholder_count; ++i) {
                    const Placeholder& placeholder = m_placeholders[i];
//                    formatted_placeholders.emplace_back(runtime_get(tuple, placeholder.identifier_index, [&placeholder, this] <typename T>(const T& value) -> std::string {
//                        // TODO: deconstructing types into named placeholders for custom type formatters (git:feature-customization branch)
//                        using Type = std::decay<T>::type;
//
//                        Formatter<Type> formatter { };
//
//                        if constexpr (detail::is_formattable<Type>) {
//                            formatter.parse(placeholder.spec);
//                            return formatter.format(value);
//                        }
//                        else {
//                            // TODO: better warning message
//                            logging::warning("ignoring format specifiers for placeholder {} (originally called from {})", placeholder.identifier_index, m_source);
//                            return formatter.format(value);
//                        }
//                    }));
                }
        
                struct InsertionPoint {
                    std::size_t placeholder_index; // Index of the placeholder to insert.
                    std::size_t position; // Position at which the placeholder should be inserted.
                };
        
                // Comparator to create a min heap based on the insertion point position.
                static auto comparator = [](const InsertionPoint& a, const InsertionPoint& b) -> bool {
                    return a.position > b.position;
                };
        
                std::priority_queue<InsertionPoint, std::vector<InsertionPoint>, decltype(comparator)> insertion_points(comparator);
                for (std::size_t i = 0u; i < unique_placeholder_count; ++i) {
                    const Placeholder& placeholder = m_placeholders[i];
        
                    for (std::size_t position : placeholder.insertion_points) {
                        insertion_points.emplace(i, position);
                    }
                }
        
                // Placeholder values are inserted into the string front to back. This allows an easier way of handling insertions for
                // placeholders that are directly adjacent due to peculiarities with the std::string::insert() function inserting starting at
                // the character right before the indicated position. By keeping track of an offset and adjusting the insertion point of
                // subsequent placeholders accordingly, we can insert values for adjacent placeholders without any extra whitespace.
        
                std::size_t offset = 0u;
        
                while (!insertion_points.empty()) {
                    const InsertionPoint& insertion_point = insertion_points.top();
                    const std::string& placeholder_value = formatted_placeholders[insertion_point.placeholder_index];
        
                    result.insert(insertion_point.position + offset, placeholder_value);
                    offset += placeholder_value.length();
        
                    insertion_points.pop();
                }
            }
        
            return std::move(result);
        }
    }
    
    template <typename T>
    NamedArgument<T>::NamedArgument(std::string name, const T& value) : name(std::move(name)),
                                                                        value(value) {
    }

    template <typename T>
    NamedArgument<T>::~NamedArgument() = default;
    
    template <typename ...Ts>
    std::string format(const FormatString& fmt, const Ts&... args) {
        return fmt.format(args...);
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
    void parse(const FormatString::Specification& spec) {
    }
    
    template <typename T>
    std::string IntegerFormatter<T>::format(T value) {
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