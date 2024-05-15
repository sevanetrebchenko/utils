
#pragma once

#ifndef FORMAT_TPP
#define FORMAT_TPP

#include "utils/logging/logging.hpp"
#include "utils/constexpr.hpp"
#include "utils/string.hpp"
#include "utils/tuple.hpp"

#include <charconv> // std::to_chars
#include <queue> // std::priority_queue
#include <limits> // std::numeric_limits

namespace utils {

    namespace detail {
        
        template <typename T>
        struct is_named_argument : std::false_type { };
        
        template <typename T>
        struct is_named_argument<NamedArgument<T>> : std::true_type { };
        
        template <typename T>
        concept has_parse_method = requires(Formatter<T> formatter, const FormatString::Specification& spec) {
            { formatter.parse(spec) };
        };
        
        template <typename T>
        concept is_formattable = requires(Formatter<T> formatter, const FormatString::Specification& spec, const T& value) {
            { formatter.parse(spec) };
            { formatter.format(value) } -> std::same_as<std::string>;
        };
        
        template <typename T>
        concept is_formattable_to = requires(Formatter<typename std::decay<T>::type> formatter, const T& value, FormattingContext& context) {
            { formatter.reserve(value) } -> std::same_as<std::size_t>;
            { formatter.format_to(value, context) } -> std::same_as<void>;
        };
        
        template <typename T>
        struct FormatData {
            Formatter<T> formatter { };
            std::size_t capacity = 0u;
        };
        
    }
    
    template <typename ...Ts>
    FormatString FormatString::format(const Ts& ...args) {
        if constexpr (sizeof...(Ts) == 0u) {
            return { *this }; // Copy constructor
        }
        else {
            // Providing fewer arguments than the number of placeholders is valid for both structured and unstructured format strings (placeholders missing arguments are simplified and included as-is in the resulting format string)
            if (!m_placeholders.empty()) {
                // Contains true if argument is a named argument (NamedArgument<T>), false otherwise
                std::vector<bool> is_named_argument { detail::is_named_argument<Ts>::value... };
                
                if (m_placeholders[0].identifier.type == Identifier::Type::Auto) {
                    // Verify that argument list does not contain any NamedArgument types, as these are intended to only be used for named arguments (format string contains only auto-numbered placeholders)
                    for (std::size_t i = 0u; i < sizeof...(Ts); ++i) {
                        if (is_named_argument[i]) {
                            throw FormattedError("invalid argument at position {} - named arguments are not allowed in format strings that only contain auto-numbered placeholders", i);
                        }
                    }
                }
                else {
                    // Verify that all positional placeholder arguments come before any named placeholder arguments
                    bool positional_arguments_parsed = false;
                    for (std::size_t i = 0u; i < sizeof...(Ts); ++i) {
                        if (is_named_argument[i]) {
                            if (positional_arguments_parsed) {
                                throw FormattedError("invalid argument at position {} - arguments for positional placeholders must come before arguments for named placeholders", i);
                            }
                            positional_arguments_parsed = true;
                        }
                    }
                }
                
                // Must allocate at least enough capacity to hold the format string (without placeholders)
                std::size_t capacity = m_result.length();

                std::tuple<Ts...> tuple = std::make_tuple(args...);
                std::tuple<detail::FormatData<Ts>...> formatters { };
                
                utils::apply([&formatters, &capacity, this]<typename T, std::size_t I>(const T& value) {
                    using Type = std::decay<T>::type;
                    
                    detail::FormatData<Type>& format_data = std::get<I>(formatters);
                    Formatter<Type>& formatter = format_data.formatter;
                    
                    // Format all unique placeholders once to save on computation power
                    if constexpr (detail::is_named_argument<T>::value) {
                        // Named placeholder (NamedArgument<T>) must be retrieved by name
                        if (has_placeholder(value.name)) {
                            const Placeholder& placeholder = get_placeholder(value.name);
                            formatter.parse(placeholder.spec);
                        }
                        // Named placeholder is not referenced in the format string, computation time can be saved by skipping formatting
                        // else { ... }
                    }
                    else {
                        // Positional placeholder must be retrieved by index
                        if (has_placeholder(I)) {
                            const Placeholder& placeholder = get_placeholder(I);
                            formatter.parse(placeholder.spec);
                        }
                    }
                    
                    // Formatters that support reserve / format_to will have formatting memory pre-allocated in the output string for memory efficiency
                    if constexpr (detail::is_formattable_to<Type>) {
                        format_data.capacity = formatter.reserve(value);
                        capacity += format_data.capacity;
                    }
                }, tuple);
                
                std::string result;
                result.resize(capacity);
                
                std::size_t write_position = 0u;
                std::size_t read_position = 0u;
                
                if (m_placeholders[0].identifier.type == Identifier::Type::Auto) {
                    for (std::size_t i = 0u; i < std::min(sizeof...(Ts), m_insertion_points.size()); ++i) {
                        const InsertionPoint& insertion_point = m_insertion_points[i];
                        
                        utils::apply([&formatters, &result, &write_position, &read_position, &insertion_point, this]<typename T, std::size_t I>(const T& value) {
                            detail::FormatData<T>& format_data = std::get<I>(formatters);
                            Formatter<T>& formatter = format_data.formatter;
                            
                            // Insert all format string contents up until this placeholder
                            // offset, count, src, offset, count
                            std::size_t num_characters = insertion_point.position - read_position;
                            result.replace(write_position, num_characters, m_result, read_position, num_characters);
                            write_position += num_characters;
                            read_position += num_characters;
                            
                            if constexpr (detail::is_formattable_to<T>) {
                                if (format_data.capacity) {
                                    // std::string is guaranteed to be stored in contiguous memory
                                    FormattingContext context { format_data.capacity, &result[write_position] };
                                    formatter.format_to(value, context);
                                    
                                    write_position += format_data.capacity;
                                }
                            }
                            else {
                            
                            }
                        }, tuple, insertion_point.placeholder_index);
                        
                        
                    }
                }
                else {
                
                }
                
                return std::move(result);
            }

            return "";
        }
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