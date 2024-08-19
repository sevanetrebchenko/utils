
#pragma once

#ifndef STRING_HPP
#define STRING_HPP

#include "utils/concepts.hpp"
#include "utils/format.hpp"

#include <vector> // std::vector
#include <string> // std::string
#include <string_view> // std::string_view

namespace utils {

    // Returns a vector containing the result of splitting 'in' by 'delimiter'.
    [[nodiscard]] std::vector<std::string> split(std::string_view in, std::string_view delimiter);
    
    // Trim off all whitespace characters on either side of 'in'.
    [[nodiscard]] std::string_view trim(std::string_view in);
    
    // std::strcasecmp requires null-terminated strings
    template <String T, String U>
    [[nodiscard]] bool casecmp(const T& first, const U& second) {}
    
    template <String T, String U>
    [[nodiscard]] bool operator==(const T& first, const U& second) {}
    
    // from_string for fundamental types (wrapper around std::from_chars)
    // Returns the number of characters processed, throws on invalid argument
    
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

#endif // STRING_HPP
