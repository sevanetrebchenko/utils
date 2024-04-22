
#pragma once

#ifndef UTILS_CONVERSIONS_HPP
#define UTILS_CONVERSIONS_HPP

#include "utils/string/formatting.hpp"
#include "utils/string/format.hpp"
#include "utils/concepts.hpp"

#include <string> // std::string
#include <string_view> // std::string_view

namespace utils {
    
    // Character types
    std::string to_string(char value, const Formatting& formatting = { });
    
    // Integer types
    std::string to_string(unsigned char value, const Formatting& formatting = { });
    std::string to_string(short value, const Formatting& formatting = { });
    std::string to_string(unsigned short value, const Formatting& formatting = { });
    std::string to_string(int value, const Formatting& formatting = { });
    std::string to_string(unsigned value, const Formatting& formatting = { });
    std::string to_string(long value, const Formatting& formatting = { });
    std::string to_string(unsigned long value, const Formatting& formatting = { });
    std::string to_string(long long value, const Formatting& formatting = { });
    std::string to_string(unsigned long long value, const Formatting& formatting = { });
    
    // Floating-point types
    std::string to_string(float value, const Formatting& formatting = { });
    std::string to_string(double value, const Formatting& formatting = { });
    std::string to_string(long double value, const Formatting& formatting = { });
    
    // String types
    std::string to_string(const char* value, const Formatting& formatting = { });
    std::string to_string(std::string_view value, const Formatting& formatting = { });
    std::string to_string(const std::string& value, const Formatting& formatting = { });
    
    // Standard types
    template <typename T>
    std::string to_string(const T* value, const Formatting& formatting = { });
    std::string to_string(std::nullptr_t value, const Formatting& formatting = { });
    
    template <typename T, typename U>
    std::string to_string(const std::pair<T, U>& value, const Formatting& formatting = { });
    template <typename ...Ts>
    std::string to_string(const std::tuple<Ts...>& value, const Formatting& formatting = { });
    
    std::string to_string(const std::source_location& value, const Formatting& formatting = { });
    
    // Standard containers
    template <typename T>
    std::string to_string(const T& value, const Formatting& formatting = { }) requires is_const_iterable<T>;
    
    // User-defined types
    template <typename T>
    std::string to_string(const NamedArgument<T>& value, const Formatting& formatting = {});
    
    
    // Types that support conversion from a string should define a specialization for this function.
    template <typename T>
    T from_string(std::string_view str);
    
    template <> char from_string(std::string_view str);
    
    template <> unsigned char from_string(std::string_view str);
    template <> short from_string(std::string_view str);
    template <> unsigned short from_string(std::string_view str);
    template <> int from_string(std::string_view str);
    template <> unsigned from_string(std::string_view str);
    template <> long from_string(std::string_view str);
    template <> unsigned long from_string(std::string_view str);
    template <> long long from_string(std::string_view str);
    template <> unsigned long long from_string(std::string_view str);
    
    template <> float from_string(std::string_view str);
    template <> double from_string(std::string_view str);
    template <> long double from_string(std::string_view str);
    
    template <typename T, typename U>
    std::pair<T, U> from_string(std::string_view str);
    
}

// Template definitions.
#include "utils/detail/string/conversions.tpp"

#endif // UTILS_CONVERSIONS_HPP
