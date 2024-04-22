
#include "utils/string/format.hpp"
#include "utils/string.hpp"
#include "utils/constexpr.hpp"

#include <string> // std::string
#include <filesystem> // std::filesystem::path

namespace utils {
    
    std::string justify(const std::string& value, const Formatting& formatting) {
        std::size_t length = value.length();

        // By default, values are left justified.
        std::string justification = "left";
        if (formatting.has_specifier("justification")) {
            justification = formatting.get_specifier("justification").convert_to<std::string>();
        }
        
        // 'width' represents the minimum width that the output string should be
        unsigned width = 0u;
        if (formatting.has_specifier("width")) {
            width = formatting.get_specifier("width").convert_to<unsigned>();
        }
        
        if (length >= width) {
            // Minimum width is less than the current width, justification is a noop
            return value;
        }
        
        char fill = ' ';
        if (formatting.has_specifier("fill")) {
            // fill = formatting["fill"];
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
    [[nodiscard]] std::string to_binary(T value, const Formatting& formatting) {
        static_assert(is_integer_type<T>::value, "value must be an integer type");

        std::string result;

        if (formatting.has_specifier("group_size")) {
            unsigned group_size = 0u;// = formatting["group_size"].to<unsigned>();
            
            char separator = ' ';
            if (formatting.has_specifier("separator")) {
//                separator = formatting["separator"].to<char>();
            }
            
            unsigned count = 0u;

            while (value > 0) {
                ++count;
                result += static_cast<char>((value % 2) + '0');
                value /= 2;

                if (count % group_size == 0u) {
                    result += separator;
                }
            }

            // Make sure all groups contain the same number of characters.
            while (count % group_size != 0u) {
                result += '0';
                ++count;
            }
        }
        else {
            while (value > 0) {
                result += static_cast<char>((value % 2) + '0');
                value /= 2;
            }
        }

        std::reverse(result.begin(), result.end());
        return std::move(result);
    }
    
    template <typename T>
    std::string integer_to_string(T value, const Formatting& formatting) {
        std::stringstream builder;
        
        bool negative = value < 0;
        std::string sign = "negativeonly";
        if (formatting.has_specifier("sign")) {
            // sign = formatting["sign"].value();
        }
        if (negative) {
            builder << '-';
        }
        else {
            if (sign == "aligned") {
                builder << ' ';
            }
            else if (sign == "both") {
                builder << '+';
            }
        }
        
        std::string representation = "";
        if (formatting.has_specifier("representation")) {
            // representation = formatting["representation"].value();
        }

        if (representation.empty()) {
        
        }
        
        return "";
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
        return std::string();
    }
    
    std::string to_string(double value, const Formatting& formatting) {
        return std::to_string(value);
    }
    
    std::string to_string(long double value, const Formatting& formatting) {
        return std::to_string(value);
    }
    
    std::string to_string(const char* value, const Formatting& formatting) {
        return std::string(value);
    }
    
    std::string to_string(std::string_view value, const Formatting& formatting) {
        return std::string(value);
    }
    
    std::string to_string(const std::string& value, const Formatting& formatting) {
        return value;
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
    struct Parser {
        
        Parser(std::string_view in);
        ~Parser();
        
        
        
    };

    template <typename T>
    T fundamental_from_string(std::string_view in) {
        // Leading whitespace is not ignored
        std::string_view str = trim(in);

        // Only a leading '-' is permitted at the beginning
        if (str[0] == '+') {
            str = str.substr(1);
        }
        
        // Leading base prefixes are not recognized
        int base = 10;
        
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
            logging::warning("processing stopped at {}", offset, in.substr(offset));
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
    
    [[nodiscard]] NamedArgumentList<std::string, std::string, std::uint32_t> to_placeholder_list(const std::source_location& value) {
        const std::string& filepath = value.file_name();
        return {
            { "filepath", filepath },
            { "filename", std::filesystem::path(filepath).filename().string() },
            { "line", value.line() }
        };
    }
    
}