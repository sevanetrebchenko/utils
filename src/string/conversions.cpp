
#include "utils/string/format.hpp"
#include "utils/string.hpp"
#include "utils/constexpr.hpp"

#include <string> // std::string
#include <filesystem> // std::filesystem::path

namespace utils {
    
    std::string justify(const std::string& value, const Formatting& formatting) {
        std::size_t length = value.length();

        // By default, values are left justified.
        std::string_view justification = "left";
        if (formatting.has_specifier("justification")) {
            justification = formatting.get_specifier("justification");
        }
        
        // 'width' represents the minimum width that the output string should be
        unsigned width = 0u;
        if (formatting.has_specifier("width")) {
            width = from_string<unsigned>(formatting.get_specifier("width"));
        }
        
        if (length >= width) {
            // Minimum width is less than the current width, justification is a noop
            return value;
        }
        
        char fill = ' ';
        if (formatting.has_specifier("fill")) {
            fill = from_string<char>(formatting.get_specifier("fill"));
        }
        
        std::string result;
        result.resize(width, fill);
        
        std::size_t offset = 0u;
        if (justification == "right") {
            offset = width - length;
        }
        else if (justification == "center") {
            offset = (width - length) / 2;
        }
        
        for (std::size_t i = 0u; i < length; ++i) {
            result[i + offset] = value[i];
        }

        return std::move(result);
    }
    
    template <typename T>
    std::string integer_to_string(T value, const Formatting& formatting) {
        static_assert(is_integer_type<T>::value, "value must be of an integer type");
        
        // TODO: precompute final string capacity
        
        std::string result;
        
        if (value < 0) {
            result.push_back('-');
            
            if constexpr (std::is_signed<T>::value) {
                // Prevent warning from being emitted, since this is called for both signed and unsigned integer types.
                value = -value;
            }
        }
        else {
            if (formatting.has_specifier("sign")) {
                std::string_view sign = formatting.get_specifier("sign");
                if (sign == "aligned") {
                    result.push_back(' ');
                }
                else if (sign == "both") {
                    result.push_back('+');
                }
            }
        }
        
        int base = 10;
        std::size_t group_size = 0u;
        char separator;
        
        if (formatting.has_specifier("representation")) {
            std::string_view representation = formatting.get_specifier("representation");
            
            bool use_base_prefix = formatting.has_specifier("use_base_prefix");
            
            if (representation == "binary") {
                base = 2;
                if (use_base_prefix) {
                    result.append("0b");
                }
            }
            else if (representation == "hexadecimal") {
                base = 16;
                if (use_base_prefix) {
                    result.append("0x");
                }
            }
            
            if (formatting.has_specifier("group_size")) {
                group_size = from_string<std::size_t>(formatting.get_specifier("group_size"));
            }
            
            // By default, custom representations use a whitespace separator character
            separator = ' ';
        }
        else {
            // Default integer representations use a comma as a separator character
            separator = ',';
        }
    
        bool use_separator;
        if (formatting.has_specifier("separator")) {
            separator = from_string<char>(formatting.get_specifier("separator"));
            use_separator = true;
        }
        else if (formatting.has_specifier("use_separator")) {
            use_separator = true;
        }
        
        char buffer[64];
        char* start = buffer;
        char* end = buffer + sizeof(buffer) / sizeof(buffer[0]);
        
        const auto& [ptr, error_code] = std::to_chars(start, end, value, base);
        
        if (error_code == std::errc::value_too_large) {
            // Out of range error
            return "";
        }
        
        std::size_t num_written = std::distance(start, ptr);
        std::size_t counter = 0u;
        
        if (group_size) {
            result.push_back(separator);
            
            // Extra padding for the left-most group
            for (std::size_t i = 0u; i < (group_size - (num_written % group_size)); ++i, ++counter) {
                result.push_back('0');
            }
            
            for (std::size_t i = 0u; i < num_written; ++i, ++counter) {
                if (counter % group_size == 0u) {
                    result.push_back(separator);
                }
                
                result.push_back(buffer[i]);
            }
        }
        else {
            if (use_separator) {
                // Separators get inserted every 3 characters
                group_size = 3;
                counter = (group_size - (num_written % group_size));
                
                for (std::size_t i = 0u; i < num_written; ++i, ++counter) {
                    if (i != 0u && counter % group_size == 0u) {
                        result.push_back(separator);
                    }
                    
                    result.push_back(buffer[i]);
                }
            }
            else {
                result.append(buffer, num_written);
            }
        }
        
        return justify(result, formatting);
    }
    
    // Returns the number of digits required to store the given value
    template <typename T>
    constexpr int count_digits(T num) {
        return (num < 10) ? 1 : 1 + count_digits(num / 10);
    }
    
    template <typename T>
    std::string floating_point_to_string(T value, const Formatting& formatting) {
        static_assert(is_floating_point_type<T>::value, "value must be of a floating point type");
        
        std::string result;
        
        if (value < 0) {
            value = -value;
            result.push_back('-');
        }
        else {
            if (formatting.has_specifier("sign")) {
                std::string_view sign = formatting.get_specifier("sign");
                if (sign == "aligned") {
                    result.push_back(' ');
                }
                else if (sign == "both") {
                    result.push_back('+');
                }
            }
        }

        int precision = 6;
        if (formatting.has_specifier("precision")) {
            precision = from_string<int>(formatting.get_specifier("precision"));
        }
        
        std::chars_format format_flags = std::chars_format::fixed;
        if (formatting.has_specifier("representation")) {
            std::string_view representation = formatting.get_specifier("representation");
            
            if (representation == "scientific") {
                format_flags = std::chars_format::scientific;
            }
        }
        
        bool use_separator = false;
        char separator = ',';
        if (formatting.has_specifier("separator")) {
            separator = from_string<char>(formatting.get_specifier("separator"));
            use_separator = true;
        }
        else if (formatting.has_specifier("use_separator")) {
            use_separator = true;
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
            return "";
        }
        
        std::size_t num_written = std::distance(start, ptr);
        
        if (use_separator) {
            char* decimal = std::find(buffer, ptr, '.');
            std::size_t decimal_position = std::distance(start, decimal);
            
            // Separators get inserted every 3 characters up until the position of the decimal point
            std::size_t group_size = 3;
            std::size_t counter = (group_size - (decimal_position % group_size));
            
            // Write the number portion, up until the decimal point (with separators)
            for (std::size_t i = 0u; i < decimal_position; ++i, ++counter) {
                if (i != 0u && counter % group_size == 0u) {
                    result.push_back(separator);
                }
                
                result.push_back(buffer[i]);
            }
            
            // Write fractional portion
            for (std::size_t i = decimal_position; i < num_written; ++i) {
                result.push_back(buffer[i]);
            }
        }
        else {
            result.append(buffer, num_written);
        }
        
        // Fake higher precision values by appending the remaining decimal places as 0
        for (std::size_t i = conversion_precision; i < precision; ++i) {
            result.push_back('0');
        }
        
        return justify(result, formatting);
    }
    
    std::string to_string(char value, const Formatting& formatting) {
        return justify(std::string(1, value), formatting);
    }
    
    std::string to_string(short value, const Formatting& formatting) {
        return integer_to_string(value, formatting);
    }
    
    std::string to_string(int value, const Formatting& formatting) {
        return integer_to_string(value, formatting);
    }
    
    std::string to_string(long value, const Formatting& formatting) {
        return integer_to_string(value, formatting);
    }
    
    std::string to_string(long long value, const Formatting& formatting) {
        return integer_to_string(value, formatting);
    }
    
    std::string to_string(unsigned char value, const Formatting& formatting) {
        return integer_to_string(value, formatting);
    }
    
    std::string to_string(unsigned short value, const Formatting& formatting) {
        return integer_to_string(value, formatting);
    }
    
    std::string to_string(unsigned value, const Formatting& formatting) {
        return integer_to_string(value, formatting);
    }
    
    std::string to_string(unsigned long long value, const Formatting& formatting) {
        return integer_to_string(value, formatting);
    }
    
    std::string to_string(float value, const Formatting& formatting) {
        return floating_point_to_string(value, formatting);
    }
    
    std::string to_string(double value, const Formatting& formatting) {
        return floating_point_to_string(value, formatting);
    }
    
    std::string to_string(long double value, const Formatting& formatting) {
        return floating_point_to_string(value, formatting);
    }
    
    std::string to_string(const char* value, const Formatting& formatting) {
        return justify(std::string(value), formatting);
    }
    
    std::string to_string(std::string_view value, const Formatting& formatting) {
        return justify(std::string(value), formatting);
    }
    
    std::string to_string(const std::string& value, const Formatting& formatting) {
        return justify(std::string(value), formatting);
    }
    
    std::string to_string(std::nullptr_t value, const Formatting& formatting) {
        return justify("nullptr", formatting);
    }
    
    std::string to_string(const std::source_location& value, const Formatting& formatting) {
        return to_string(value.file_name(), formatting) + ":" + to_string(value.line(), formatting);
    }
    
//    template <typename T>
//    auto deconstruct(const T& value) {
//        const std::string& filepath = value.file_name();
//
//        auto tup = std::make_tuple(NamedArgument<std::string>("filepath", filepath));
//        return tup;
//
////        return std::tuple<
////            NamedArgument<std::string> {  },
////            NamedArgument<std::string> { "filename", std::filesystem::path(filepath).filename().string() },
////            NamedArgument<unsigned> { "line", value.line() },
////            NamedArgument<std::string> { "function", value.function_name() }
////        );
//    }
    
//    template <>
//    auto deconstruct(const std::source_location& value) {
//        const std::string& filepath = value.file_name();
//
//        auto tup = std::make_tuple(NamedArgument<std::string>("filepath", filepath));
//        return tup;
//
////        return std::tuple<
////            NamedArgument<std::string> {  },
////            NamedArgument<std::string> { "filename", std::filesystem::path(filepath).filename().string() },
////            NamedArgument<unsigned> { "line", value.line() },
////            NamedArgument<std::string> { "function", value.function_name() }
////        );
//    }

//    template <>
//    auto deconstruct(const std::source_location& value) {
//        const std::string& filepath = value.file_name();
//
//        auto tup = std::make_tuple(NamedArgument<std::string>("filepath", filepath));
//        return tup;
//
//        return std::tuple<
//            NamedArgument<std::string> {  },
//            NamedArgument<std::string> { "filename", std::filesystem::path(filepath).filename().string() },
//            NamedArgument<unsigned> { "line", value.line() },
//            NamedArgument<std::string> { "function", value.function_name() }
//        );
//    }

    template <typename T>
    T fundamental_from_string(std::string_view in) {
        // Leading whitespace is not ignored
        std::string_view str = trim(in);
        
        if (str.empty()) {
            throw FormattedError("");
        }
        
        // Only a leading '-' is permitted at the beginning
        if (str[0] == '+') {
            str = str.substr(1);
        }
        
        // Leading base prefixes are not recognized
        int base = 10;
        
        if (str.length() > 1) {
            if (str[0] == '0') {
                if (str[1] == 'x' || str[1] == 'X') {
                    // Hexadecimal
                    base = 16;
                }
                else if (str[1] == 'b' || str[1] == 'B') {
                    // Binary
                    base = 2;
                }
                else if (str[1] == 'o' || str[1] == 'O') {
                    // Octal
                    base = 8;
                }
            }
        }

        if (base != 10) {
            str = str.substr(2);
        }
        
        auto* first = str.data();
        auto* last = first + str.length();

        T value;
        std::from_chars_result result { };
        
        if constexpr (is_integer_type<T>::value) {
            result = std::from_chars(first, last, value, base);
        }
        else {
            // std::format_chars::general supports both scientific and fixed representations
            result = std::from_chars(first, last, value, std::chars_format::general);
        }
        
        const auto& [ptr, error_code] = result;
        
        if (ptr != last) {
            // not all characters processed, consider this an error
            std::size_t offset = std::distance(first, ptr);
            // logging::warning("processing stopped at {}", offset, in.substr(offset));
        }
        
        if (error_code == std::errc::invalid_argument) {
            // failed to convert
            logging::fatal("");
            throw FormattedError("");
        }
        
        if (error_code == std::errc::result_out_of_range) {
            logging::error("");
            return std::numeric_limits<T>::max();
        }
        
        return value;
    }

    template <>
    [[nodiscard]] char from_string(std::string_view str) {
        if (str.empty()) {
            throw FormattedError("");
        }
        
        if (str.length() > 1) {
        }

        return str[0];
    }
    
    template <> unsigned char from_string(std::string_view str) {
        return fundamental_from_string<unsigned char>(str);
    }
    
    template <> short from_string(std::string_view str) {
        return fundamental_from_string<short>(str);
    }
    
    template <> unsigned short from_string(std::string_view str) {
        return fundamental_from_string<unsigned short>(str);
    }
    
    template <> int from_string(std::string_view str) {
        return fundamental_from_string<int>(str);
    }
    
    template <> unsigned from_string(std::string_view str) {
        return fundamental_from_string<unsigned>(str);
    }
    
    template <> long from_string(std::string_view str) {
        return fundamental_from_string<long>(str);
    }
    
    template <> unsigned long from_string(std::string_view str) {
        return fundamental_from_string<unsigned long>(str);
    }
    
    template <> long long from_string(std::string_view str) {
        return fundamental_from_string<long long>(str);
    }
    
    template <> unsigned long long from_string(std::string_view str) {
        return fundamental_from_string<unsigned long long>(str);
    }
    
    template <> float from_string(std::string_view str) {
        return fundamental_from_string<float>(str);
    }
    
    template <> double from_string(std::string_view str) {
        return fundamental_from_string<double>(str);
    }
    
    template <> long double from_string(std::string_view str) {
        return fundamental_from_string<long double>(str);
    }
    
}