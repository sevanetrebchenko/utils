
#pragma once

#ifndef UTILS_CONVERSIONS_HPP
#define UTILS_CONVERSIONS_HPP

#include "utils/string/formatting.hpp"
#include "utils/string/format.hpp"
#include "utils/concepts.hpp"

#include <string> // std::string
#include <string_view> // std::string_view
#include <charconv> // std::chars_format

namespace utils {
    
    // Character types
    std::string to_string(char value, Formatting formatting = {});
    
    // Integer types
    std::string to_string(unsigned char value, Formatting formatting = {});
    std::string to_string(short value, Formatting formatting = {});
    std::string to_string(unsigned short value, Formatting formatting = {});
    std::string to_string(int value, Formatting formatting = {});
    std::string to_string(unsigned value, Formatting formatting = {});
    std::string to_string(long value, Formatting formatting = {});
    std::string to_string(unsigned long value, Formatting formatting = {});
    std::string to_string(long long value, Formatting formatting = {});
    std::string to_string(unsigned long long value, Formatting formatting = {});
    
    // Floating-point types
    std::string to_string(float value, Formatting formatting = {});
    std::string to_string(double value, Formatting formatting = {});
    std::string to_string(long double value, Formatting formatting = {});
    
    // String types
    std::string to_string(const char* value, Formatting formatting = {});
    std::string to_string(std::string_view value, Formatting formatting = {});
    std::string to_string(const std::string& value, Formatting formatting = {});
    
    // Standard types
    template <typename T>
    std::string to_string(const T* value, Formatting formatting = {});
    std::string to_string(std::nullptr_t value, Formatting formatting = {});
    
    template <typename T, typename U>
    std::string to_string(const std::pair<T, U>& value, Formatting formatting = {});
    template <typename ...Ts>
    std::string to_string(const std::tuple<Ts...>& value, Formatting formatting = {});
    
    std::string to_string(const std::source_location& value, Formatting formatting = {});
    
    // Standard containers
    template <typename T>
    std::string to_string(const T& value, Formatting formatting = {}) requires is_const_iterable<T>;
    
    template <typename T>
    std::string to_string(const NamedArgument<T>& value, Formatting formatting = {});
    
    
    // from_string for fundamental types (wrapper around std::from_chars)
    // Returns the number of characters processed
    
    std::size_t from_string(std::string_view in, unsigned char& out);
    std::size_t from_string(std::string_view in, short& out);
    std::size_t from_string(std::string_view in, unsigned short& out);
    std::size_t from_string(std::string_view in, int& out);
    std::size_t from_string(std::string_view in, unsigned& out);
    std::size_t from_string(std::string_view in, long& out);
    std::size_t from_string(std::string_view in, unsigned long& out);
    std::size_t from_string(std::string_view in, long long& out);
    std::size_t from_string(std::string_view in, unsigned long long& out);
    
    // Supports both scientific and fixed notation
    std::size_t from_string(std::string_view in, float& out);
    std::size_t from_string(std::string_view in, double& out);
    std::size_t from_string(std::string_view in, long double& out);
    
}

// Template definitions.
#include "utils/detail/string/conversions.tpp"

#endif // UTILS_CONVERSIONS_HPP
