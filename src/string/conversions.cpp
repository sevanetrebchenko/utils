
#include "utils/string/format.hpp"
#include "utils/string/specifiers.hpp"

#include <string> // std::string
#include <filesystem> // std::filesystem::path

namespace utils {
    
    [[nodiscard]] std::string justify(const std::string& value, Justification justification, unsigned minimum_width, char fill_character) {
        std::size_t length = value.length();
        if (length >= minimum_width) {
            // Minimum width is less than the current width, justification is a noop.
            return value;
        }

        std::string result;
        result.resize(minimum_width, fill_character);

        std::size_t offset;

        if (justification == Justification::Left) {
            offset = 0u;
        }
        else if (justification == Justification::Right) {
            offset = minimum_width - length;
        }
        else {
            // Center justification.
            offset = (minimum_width - length) / 2;
        }

        for (std::size_t i = 0u; i < length; ++i) {
            result[i + offset] = value[i];
        }

        return std::move(result);
    }
    
    std::string to_string(char value, const Formatting& formatting) {
        Justification justification = formatting["justify"].to<Justification>();
        unsigned width = formatting["width"].to<unsigned>();
        char fill = formatting["fill"].to<char>();
        Sign sign = formatting["sign"].to<Sign>();
        
        return justify(std::string(1, value), justification, width, fill);
    }
    
    std::string utils::to_string(short value, const Formatting& formatting) {
        return std::string();
    }
    
    std::string utils::to_string(int value, const Formatting& formatting) {
        return std::to_string(value);
    }
    
    std::string utils::to_string(long value, const Formatting& formatting) {
        return std::to_string(value);
    }
    
    std::string utils::to_string(long long int value, const Formatting& formatting) {
        return std::to_string(value);
    }
    
    std::string utils::to_string(unsigned char value, const Formatting& formatting) {
        return std::string();
    }
    
    std::string utils::to_string(unsigned short value, const Formatting& formatting) {
        return std::string();
    }
    
    std::string utils::to_string(unsigned int value, const Formatting& formatting) {
        return std::to_string(value);
    }
    
    std::string utils::to_string(unsigned long long int value, const Formatting& formatting) {
        return std::to_string(value);
    }
    
    std::string utils::to_string(float value, const Formatting& formatting) {
        return std::string();
    }
    
    std::string utils::to_string(double value, const Formatting& formatting) {
        return std::to_string(value);
    }
    
    std::string utils::to_string(long double value, const Formatting& formatting) {
        return std::to_string(value);
    }
    
    std::string utils::to_string(const char* value, const Formatting& formatting) {
        return value;
    }
    
    std::string utils::to_string(std::string_view value, const Formatting& formatting) {
        return std::string(value);
    }
    
    std::string utils::to_string(const std::string& value, const Formatting& formatting) {
        return value;
    }
    
    std::string utils::to_string(std::nullptr_t value, const Formatting& formatting) {
        return std::string();
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

    template <>
    [[nodiscard]] char from_string<char>(std::string_view value) {
        return '0';
    }
    
    template <>
    [[nodiscard]] short from_string<short>(std::string_view value) {
        return 0;
    }
    
    template <>
    [[nodiscard]] int from_string<int>(std::string_view value) {
        return 0;
    }
    
    template <>
    [[nodiscard]] long int from_string<long int>(std::string_view value) {
        return 0;
    }
    
    template <>
    [[nodiscard]] long long int from_string<long long int>(std::string_view value) {
        return 0;
    }
    
    template <>
    [[nodiscard]] unsigned char from_string<unsigned char>(std::string_view value) {
        return 0;
    }
    
    template <>
    [[nodiscard]] unsigned short from_string<unsigned short>(std::string_view value) {
        return 0;
    }
    
    template <>
    [[nodiscard]] unsigned int from_string<unsigned int>(std::string_view value) {
        return 0;
    }
    
    template <>
    [[nodiscard]] unsigned long int from_string<unsigned long int>(std::string_view value) {
        return 0;
    }
    
    template <>
    [[nodiscard]] unsigned long long int from_string<unsigned long long int>(std::string_view value) {
        return 0;
    }
    
    template <>
    [[nodiscard]] float from_string<float>(std::string_view value) {
        return 0;
    }
    
    template <>
    [[nodiscard]] double from_string<double>(std::string_view value) {
        return 0;
    }
    
    [[nodiscard]] NamedArgumentList<std::string, std::string, std::uint32_t, std::string> deconstruct(const std::source_location& value) {
        const std::string& filepath = value.file_name();
        return {
            { "filepath", filepath },
            { "filename", std::filesystem::path(filepath).filename().string() },
            { "line", value.line() },
            { "function", value.function_name() }
        };
    }
    
}