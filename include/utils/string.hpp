
#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

#pragma once

#include <vector> // std::vector
#include <string> // std::string
#include <functional> // std::function

namespace utils {

    // Returns a vector containing the result of splitting 'in' by 'delimiter'.
    [[nodiscard]] std::vector<std::string> split(const std::string& in, const std::string& delimiter);
    
    
    // Returns a string of 'components' joined by 'glue'.
    template <typename Container>
    [[nodiscard]] std::string join(const Container& components, const std::string& glue);
    
    // Template specializations for type deductions.
    [[nodiscard]] std::string join(const std::initializer_list<std::string>& components, const std::string& glue);
    
    
    // Trim off all whitespace characters on either side of 'in'.
    [[nodiscard]] std::string trim(const std::string& in);
    
    
    // For formatting named format specifiers.
    // Note: custom types must define either a std::string conversion operator (T::operator std::string(), preferred) or a standalone to_string(const T&) function.

    template <typename T>
    struct arg {
        arg(std::string name, const T& value);
        ~arg();
    
        std::string name;
        const T& value; // Maintain reference for making dealing with non-trivially copyable types easier.
    };
    
    // Python f-string format
    template <typename ...Ts>
    [[nodiscard]] std::string format(const std::string& fmt, const Ts&... args);
    
}

// Template definitions.
#include "utils/internal/string.tpp"

#endif // UTILS_STRING_HPP
