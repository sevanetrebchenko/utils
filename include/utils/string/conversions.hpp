
#pragma once

#ifndef UTILS_CONVERSIONS_HPP
#define UTILS_CONVERSIONS_HPP

#include "utils/string/formatting.hpp"
#include "utils/string/format.hpp"
#include "utils/concepts.hpp"

#include <string> // std::string
#include <source_location> // std::source_location
#include <tuple> // std::tuple
#include <filesystem>

namespace utils {
    
    // Character types
    [[nodiscard]] std::string to_string(char value, const Formatting& formatting = { });
    
    // Integer types
    [[nodiscard]] std::string to_string(short value, const Formatting& formatting = { });
    [[nodiscard]] std::string to_string(int value, const Formatting& formatting = { });
    [[nodiscard]] std::string to_string(long int value, const Formatting& formatting = { });
    [[nodiscard]] std::string to_string(long long int value, const Formatting& formatting = { });
    
    [[nodiscard]] std::string to_string(unsigned char value, const Formatting& formatting = { });
    [[nodiscard]] std::string to_string(unsigned short value, const Formatting& formatting = { });
    [[nodiscard]] std::string to_string(unsigned int value, const Formatting& formatting = { });
    [[nodiscard]] std::string to_string(unsigned long int value, const Formatting& formatting = { });
    [[nodiscard]] std::string to_string(unsigned long long int value, const Formatting& formatting = { });
    
    // Floating-point types
    // Supported format specifiers: precision (int), representation (string, fixed/scientific/percent),
    [[nodiscard]] std::string to_string(float value, const Formatting& formatting = { });
    [[nodiscard]] std::string to_string(double value, const Formatting& formatting = { });
    [[nodiscard]] std::string to_string(long double value, const Formatting& formatting = { });
    
    // String types
    // Supported format specifiers: width (int), fill (char), align (string, left/right/center), separator (char)
    [[nodiscard]] std::string to_string(const char* value, const Formatting& formatting = { });
    [[nodiscard]] std::string to_string(std::string_view value, const Formatting& formatting = { });
    [[nodiscard]] std::string to_string(const std::string& value, const Formatting& formatting = { });
    
    // Tuple types
    template <typename T, typename U>
    [[nodiscard]] std::string to_string(const std::pair<T, U>& value, const Formatting& formatting = { });
    
    template <typename ...Ts>
    [[nodiscard]] std::string to_string(const std::tuple<Ts...>& value, const Formatting& formatting = { });
    
    // Container types
    template <typename T>
    [[nodiscard]] std::string to_string(const T& value, const Formatting& formatting = { }) requires is_const_iterable<T>;
    
    // Pointer types
    template <typename T>
    [[nodiscard]] std::string to_string(const T* value, const Formatting& formatting = { });
    [[nodiscard]] std::string to_string(std::nullptr_t value, const Formatting& formatting = { });
    
    // Standard types
    [[nodiscard]] std::string to_string(const std::source_location& value, const Formatting& formatting = { });
    
    // User-defined types.
    template <typename T>
    [[nodiscard]] std::string to_string(const NamedArgument<T>& value, const Formatting& formatting = {});
    
    
    template <typename T>
    [[nodiscard]] T from_string(std::string_view str);
    
    // Character types
    template <>
    [[nodiscard]] char from_string(std::string_view str);
    
    // Integer types
    template <>
    [[nodiscard]] short from_string(std::string_view str);
    template <>
    [[nodiscard]] int from_string(std::string_view str);
    template <>
    [[nodiscard]] long int from_string(std::string_view str);
    template <>
    [[nodiscard]] long long int from_string(std::string_view str);
    
    template <>
    [[nodiscard]] unsigned char from_string(std::string_view str);
    template <>
    [[nodiscard]] unsigned short from_string(std::string_view str);
    template <>
    [[nodiscard]] unsigned int from_string(std::string_view str);
    template <>
    [[nodiscard]] unsigned long int from_string(std::string_view str);
    template <>
    [[nodiscard]] unsigned long long int from_string(std::string_view str);
    
    // Floating-point types
    template <>
    [[nodiscard]] float from_string(std::string_view str);
    template <>
    [[nodiscard]] double from_string(std::string_view str);
    
    // String types
    template <>
    [[nodiscard]] std::string from_string(std::string_view str);
    
    [[nodiscard]] NamedArgumentList<std::string, std::string, std::uint32_t, std::string> deconstruct(const std::source_location& value);
    
}

// Template definitions.
#include "utils/detail/string/conversions.tpp"

#endif // UTILS_CONVERSIONS_HPP
