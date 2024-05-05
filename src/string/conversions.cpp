//
//#include "utils/string/format.hpp"
//#include "utils/string.hpp"
//#include "utils/constexpr.hpp"
//
//#include <string> // std::string
//#include <filesystem> // std::filesystem::path
//
//namespace utils {
//
//    int round_up_to_multiple(int value, int multiple) {
//        if (multiple == 0)
//            return value;
//
//        int remainder = value % multiple;
//        if (remainder == 0) {
//            return value;
//        }
//
//        return value + multiple - remainder;
//    }
//
//    template <typename T>
//    constexpr int count_digits(T num) {
//        return (num < 10) ? 1 : 1 + count_digits(num / 10);
//    }
//
//    template <typename T>
//    std::string integer_to_base(T value, int base, const Formatting& formatting) {
//        static_assert(is_integer_type<T>::value, "value must be of an integer type");
//
//        std::size_t capacity = 0u;
//        std::size_t read_offset = 0u;
//        std::size_t write_offset = 0u;
//
//        std::string result;
//
//        const char* sign = nullptr;
//        if (value < 0) {
//            read_offset = 1u; // Do not read negative sign in resulting buffer
//
//            if (formatting.sign != Formatting::Sign::None) {
//                sign = "-";
//                ++capacity;
//            }
//        }
//        else {
//            switch (formatting.sign) {
//                case Formatting::Sign::Aligned:
//                    sign = " ";
//                    ++capacity;
//                    break;
//                case Formatting::Sign::Both:
//                    sign = "+";
//                    ++capacity;
//                    break;
//                default:
//                    break;
//            }
//        }
//
//        char buffer[sizeof(unsigned long long) * 8 + 1];
//        char* start = buffer;
//        char* end = buffer + sizeof(buffer) / sizeof(buffer[0]);
//
//        const auto& [ptr, error_code] = std::to_chars(start, end, value, base);
//        if (error_code == std::errc::value_too_large) {
//            return "too large";
//        }
//
//        std::size_t num_characters_written = ptr - (start + read_offset);
//        capacity += num_characters_written;
//
//        char fill_character = ' ';
//        if (formatting.fill) {
//            fill_character = formatting.fill;
//        }
//
//        if (base == 10) {
//            std::size_t group_size = 3u;
//            if (formatting.separator) {
//                capacity += num_characters_written / group_size - bool(num_characters_written % group_size == 0);
//            }
//
//            // Simplified case for decimal representations, since this does not support many of the formatting specifiers.
//            if (capacity < formatting.width) {
//                switch (formatting.justification) {
//                    case Formatting::Justification::Right:
//                        write_offset = formatting.width - capacity;
//                        break;
//                    case Formatting::Justification::Center:
//                        write_offset = (formatting.width - capacity) / 2;
//                        break;
//                    default:
//                        break;
//                }
//
//                capacity = formatting.width;
//            }
//
//            result.resize(capacity, fill_character);
//
//            if (sign) {
//                result[write_offset++] = sign[0];
//            }
//
//            if (formatting.separator) {
//                std::size_t current = group_size - (num_characters_written % group_size);
//                for (std::size_t i = 0u; i < num_characters_written; ++i, ++current) {
//                    if (i && (current % group_size) == 0u) {
//                        result[write_offset++] = formatting.separator;
//                    }
//
//                    result[write_offset++] = *(buffer + read_offset + i);
//                }
//            }
//            else {
//                for (start = buffer + read_offset; start != ptr; ++start) {
//                    result[write_offset++] = *start;
//                }
//            }
//        }
//        else {
//            std::size_t num_padding_characters = 0u;
//            if (formatting.group_size) {
//                // The final group may not be the same size as the ones that come before it
//                std::size_t remainder = (num_characters_written % formatting.group_size);
//                if (remainder) {
//                    num_padding_characters += formatting.group_size - (num_characters_written % formatting.group_size);
//                }
//
//                std::size_t precision = round_up_to_multiple(formatting.precision, formatting.group_size);
//
//                // Add an arbitrary number of formatting.padding characters to reach the value of formatting.precision
//                if (num_characters_written + num_padding_characters < precision) {
//                    num_padding_characters += precision - (num_characters_written + num_padding_characters);
//                }
//
//                // The formatting.separator character is inserted before every group and must be accounted for
//                // All except the first group
//                capacity += (num_characters_written + num_padding_characters) / formatting.group_size - 1;
//            }
//            else {
//                if (num_characters_written < formatting.precision) {
//                    num_padding_characters = formatting.precision - num_characters_written;
//                }
//            }
//
//            capacity += num_padding_characters;
//
//            if (formatting.use_base_prefix) {
//                // +2 characters for base prefix '0b'
//                capacity += 2;
//
//                if (formatting.group_size) {
//                    // +1 character for a formatting.separator between the groups and the base prefix
//                    capacity += 1;
//                }
//            }
//
//            if (capacity < formatting.width) {
//                switch (formatting.justification) {
//                    case Formatting::Justification::Right:
//                        write_offset = formatting.width - capacity;
//                        break;
//                    case Formatting::Justification::Center:
//                        write_offset = (formatting.width - capacity) / 2;
//                        break;
//                    default:
//                        break;
//                }
//
//                capacity = formatting.width;
//            }
//
//            result.resize(capacity, fill_character);
//
//            char padding_character = '.';
//            if (formatting.padding) {
//                padding_character = formatting.padding;
//            }
//
//            char separator_character = ' ';
//            if (formatting.separator) {
//                separator_character = formatting.separator;
//            }
//
//            if (sign) {
//                result[write_offset++] = sign[0];
//            }
//
//            if (formatting.use_base_prefix) {
//                result[write_offset++] = '0';
//                result[write_offset++] = 'b';
//
//                if (formatting.group_size) {
//                    result[write_offset++] = separator_character;
//                }
//            }
//
//            if (formatting.group_size) {
//                std::size_t current = 0u;
//
//                for (std::size_t i = 0u; i < num_padding_characters; ++i, ++current) {
//                    if (current && current % formatting.group_size == 0u) {
//                        result[write_offset++] = separator_character;
//                    }
//                    result[write_offset++] = padding_character;
//                }
//
//                for (start = buffer + read_offset; start != ptr; ++start, ++current) {
//                    if (current && current % formatting.group_size == 0u) {
//                        result[write_offset++] = separator_character;
//                    }
//
//                    result[write_offset++] = *start;
//                }
//            }
//            else {
//                for (std::size_t i = 0u; i < num_padding_characters; ++i) {
//                    result[write_offset++] = padding_character;
//                }
//
//                for (start = buffer + read_offset; start != ptr; ++start) {
//                    result[write_offset++] = *start;
//                }
//            }
//        }
//
//        return std::move(result);
//    }
//
//    template <typename T>
//    inline std::string integer_to_string(T value, const Formatting& formatting) {
//        int base;
//        switch (formatting.representation) {
//            case Formatting::Representation::Fixed:
//            case Formatting::Representation::Scientific:
//            case Formatting::Representation::Decimal:
//                base = 10;
//                break;
//            case Formatting::Representation::Binary:
//                base = 2;
//                break;
//            case Formatting::Representation::Hexadecimal:
//                base = 16;
//                break;
//        }
//
//        return integer_to_base(value, base, formatting);
//    }
//
//    template <typename T>
//    std::string floating_point_to_string(T value, const Formatting& formatting) {
//        static_assert(is_floating_point_type<T>::value, "value must be of a floating point type");
//
//        std::size_t capacity = 0u;
//        std::size_t read_offset = 0u;
//
//        const char* sign = nullptr;
//        if (value < 0) {
//            read_offset = 1u; // Do not read negative sign in resulting buffer
//
//            if (formatting.sign != Formatting::Sign::None) {
//                sign = "-";
//                ++capacity;
//            }
//        }
//        else {
//            switch (formatting.sign) {
//                case Formatting::Sign::Aligned:
//                    sign = " ";
//                    ++capacity;
//                    break;
//                case Formatting::Sign::Both:
//                    sign = "+";
//                    ++capacity;
//                    break;
//                default:
//                    break;
//            }
//        }
//
//        int precision = 6;
//        if (formatting.precision) {
//            precision = formatting.precision;
//        }
//
//        std::chars_format format_flags = std::chars_format::fixed;
//        if (formatting.representation == Formatting::Representation::Scientific) {
//            format_flags = std::chars_format::scientific;
//        }
//
//        // Buffer must be large enough to store:
//        //  - the number of digits in the largest representable number (max_exponent10)
//        //  - decimal point
//        //  - highest supported precision for the given type (max_digits10)
//        char buffer[std::numeric_limits<T>::max_exponent10 + 1 + std::numeric_limits<T>::max_digits10];
//        char* start = buffer;
//        char* end = buffer + sizeof(buffer) / sizeof(buffer[0]);
//
//        // std::numeric_limits<T>::digits10 represents the number of decimal places that are guaranteed to be preserved when converted to text
//        // Note: last decimal place will be rounded
//        int conversion_precision = std::clamp(precision, 0, std::numeric_limits<T>::digits10);
//        const auto& [ptr, error_code] = std::to_chars(start, end, value, format_flags, conversion_precision);
//
//        if (error_code == std::errc::value_too_large) {
//            return "too large";
//        }
//
//        std::size_t num_characters_written = ptr - (start + read_offset);
//        capacity += num_characters_written;
//
//        // Additional precision
//        capacity += std::max(0, precision - conversion_precision);
//
//        std::size_t decimal_position = num_characters_written;
//        if (formatting.separator) {
//            char* decimal = std::find(start + read_offset, ptr, '.');
//            decimal_position = decimal - (start + read_offset);
//
//            // Separators get inserted every 3 characters up until the position of the decimal point
//            capacity += (decimal_position - 1) / 3;
//        }
//
//        char fill_character = ' ';
//        if (formatting.fill) {
//            fill_character = formatting.fill;
//        }
//
//        std::size_t write_offset = 0u;
//        if (capacity < formatting.width) {
//            switch (formatting.justification) {
//                case Formatting::Justification::Right:
//                    write_offset = formatting.width - capacity;
//                    break;
//                case Formatting::Justification::Center:
//                    write_offset = (formatting.width - capacity) / 2;
//                    break;
//                default:
//                    break;
//            }
//
//            capacity = formatting.width;
//        }
//
//        std::string result;
//        result.resize(capacity, fill_character);
//
//        if (sign) {
//            result[write_offset++] = sign[0];
//        }
//
//        if (formatting.representation == Formatting::Representation::Scientific) {
//            char* e = std::find(buffer, ptr, 'e');
//            std::size_t e_position = e - (start + read_offset);
//
//            for (std::size_t i = 0u; i < e_position; ++i) {
//                result[write_offset++] = *(buffer + read_offset + i);
//            }
//
//            // For scientific notation, fake precision must be appended before the 'e' denoting the exponent
//            for (std::size_t i = conversion_precision; i < precision; ++i) {
//                result[write_offset++] = '0';
//            }
//
//            for (start = buffer + read_offset + e_position; start != ptr; ++start) {
//                result[write_offset++] = *start;
//            }
//        }
//        else {
//            // Separation only makes sense for fixed floating point values
//            char separator_character = ' ';
//            if (formatting.separator) {
//                separator_character = formatting.separator;
//
//                // Separators get inserted every 3 characters up until the position of the decimal point
//                std::size_t group_size = 3;
//                std::size_t counter = group_size - (decimal_position % group_size);
//
//                // Write the number portion, up until the decimal point (with separators)
//                for (std::size_t i = 0; i < decimal_position; ++i, ++counter) {
//                    if (i && counter % group_size == 0u) {
//                        result[write_offset++] = separator_character;
//                    }
//
//                    result[write_offset++] = *(buffer + read_offset + i);
//                }
//
//                // Write decimal portion
//                for (start = buffer + read_offset + decimal_position; start != ptr; ++start) {
//                    result[write_offset++] = *start;
//                }
//            }
//
//            // For regular floating point values, fake higher precision by appending the remaining decimal places as 0
//            for (std::size_t i = conversion_precision; i < precision; ++i) {
//                result[write_offset++] = '0';
//            }
//        }
//
//        return std::move(result);
//    }
//
//    inline std::string string_to_string(const char* value, std::size_t length, const Formatting& formatting) {
//        if (formatting.width < length) {
//            // Justification is a noop
//            return { value, length };
//        }
//        else {
//            std::size_t write_offset;
//            switch (formatting.justification) {
//                case Formatting::Justification::Left:
//                    write_offset = 0u; // Default is left-justified
//                    break;
//                case Formatting::Justification::Right:
//                    write_offset = formatting.width - length;
//                    break;
//                case Formatting::Justification::Center:
//                    write_offset = (formatting.width - length) / 2;
//                    break;
//            }
//
//            char fill_character = ' ';
//            if (formatting.fill) {
//                fill_character = formatting.fill;
//            }
//
//            std::string result;
//            result.resize(formatting.width, fill_character);
//
//            for (std::size_t i = 0u; i < length; ++i) {
//                result[write_offset + i] = value[i];
//            }
//
//            return std::move(result);
//        }
//    }
//
//    std::string to_string(unsigned char value, Formatting formatting) {
//        return integer_to_string(value, formatting);
//    }
//
//    std::string to_string(short value, Formatting formatting) {
//        return integer_to_string(value, formatting);
//    }
//
//    std::string to_string(unsigned short value, Formatting formatting) {
//        return integer_to_string(value, formatting);
//    }
//
//    std::string to_string(int value, Formatting formatting) {
//        return integer_to_string(value, formatting);
//    }
//    std::string to_string(unsigned value, Formatting formatting) {
//        return integer_to_string(value, formatting);
//    }
//
//    std::string to_string(long value, Formatting formatting) {
//        return integer_to_string(value, formatting);
//    }
//
//    std::string to_string(unsigned long value, Formatting formatting) {
//        return integer_to_string(value, formatting);
//    }
//
//    std::string to_string(long long value, Formatting formatting) {
//        return integer_to_string(value, formatting);
//    }
//
//    std::string to_string(unsigned long long value, Formatting formatting) {
//        return integer_to_string(value, formatting);
//    }
//
//    std::string to_string(float value, Formatting formatting) {
//        return floating_point_to_string(value, formatting);
//    }
//
//    std::string to_string(double value, Formatting formatting) {
//        return floating_point_to_string(value, formatting);
//    }
//
//    std::string to_string(long double value, Formatting formatting) {
//        return floating_point_to_string(value, formatting);
//    }
//
//    std::string to_string(const char* value, Formatting formatting) {
//        return string_to_string(value, strlen(value), formatting);
//    }
//
//    std::string to_string(std::string_view value, Formatting formatting) {
//        return string_to_string(value.data(), value.length(), formatting);
//    }
//
//    std::string to_string(const std::string& value, Formatting formatting) {
//        return string_to_string(value.data(), value.length(), formatting);
//    }
//
//    std::string to_string(std::nullptr_t, Formatting formatting) {
//        if (formatting.representation == Formatting::Representation::Hexadecimal) {
//            // Use pointer representation
//            formatting.precision = sizeof(void*);
//            return integer_to_string(0, formatting);
//        }
//        else {
//            return string_to_string("nullptr", 7, formatting);
//        }
//    }
//
//    std::string to_string(const std::source_location& value, Formatting formatting) {
//        const std::string& filepath = value.file_name();
//        const std::string& filename = std::filesystem::path(filepath).filename().string();
//
//        if (formatting.next) {
//            return string_to_string(filename.data(), filename.length(), formatting) + ':' + integer_to_string(value.line(), formatting);
//        }
//        else {
//            return string_to_string(filename.data(), filename.length(), formatting) + ':' + integer_to_string(value.line(), formatting);
//        }
//    }
//
//    template <typename T>
//    T fundamental_from_string(std::string_view in) {
//        // Leading whitespace is not ignored
//        std::string_view str = trim(in);
//
//        if (str.empty()) {
//            throw FormattedError("");
//        }
//
//        // Only a leading '-' is permitted at the beginning
//        if (str[0] == '+') {
//            str = str.substr(1);
//        }
//
//        // Leading base prefixes are not recognized
//        int base = 10;
//        bool has_base = false;
//
//        if (str.length() > 1) {
//            if (str[0] == '0') {
//                if (str[1] == 'x' || str[1] == 'X') {
//                    // Hexadecimal
//                    base = 16;
//                    has_base = true;
//                }
//                else if (str[1] == 'b' || str[1] == 'B') {
//                    // Binary
//                    base = 2;
//                    has_base = true;
//                }
//            }
//        }
//
//        if (has_base) {
//            str = str.substr(2);
//        }
//
//        auto* first = str.data();
//        auto* last = first + str.length();
//
//        T value;
//        std::from_chars_result result { };
//
//        if constexpr (is_integer_type<T>::value) {
//            result = std::from_chars(first, last, value, base);
//        }
//        else {
//            // std::format_chars::general supports both scientific and fixed representations
//            result = std::from_chars(first, last, value, std::chars_format::general);
//        }
//
//        const auto& [ptr, error_code] = result;
//
//        if (ptr != last) {
//            // not all characters processed, consider this an error
//            std::size_t offset = std::distance(first, ptr);
//            // logging::warning("processing stopped at {}", offset, in.substr(offset));
//        }
//
//        if (error_code == std::errc::invalid_argument) {
//            // failed to convert
//            logging::fatal("");
//            throw FormattedError("");
//        }
//
//        if (error_code == std::errc::result_out_of_range) {
//            logging::error("");
//            return std::numeric_limits<T>::max();
//        }
//
//        return value;
//    }
//
//}