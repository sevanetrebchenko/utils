
#pragma once

#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

#include "utils/string/format.hpp"
#include "utils/concepts.hpp"

#include <vector> // std::vector
#include <string> // std::string
#include <string_view> // std::string_view
#include <initializer_list> // std::initializer_list

namespace utils {

    // Returns a vector containing the result of splitting 'in' by 'delimiter'.
    [[nodiscard]] std::vector<std::string> split(const std::string& in, const std::string& delimiter);
    
    // Returns a string of 'components' joined by 'glue'.
    template <typename C>
    [[nodiscard]] std::string join(const C& container, const std::string& glue) requires is_const_iterable<C>;
    
    // Template specializations for type deductions.
    [[nodiscard]] std::string join(const std::initializer_list<std::string_view>& components, const std::string& glue);
    
    // Trim off all whitespace characters on either side of 'in'.
    [[nodiscard]] std::string trim(std::string_view in);
    
}

// Template definitions.
#include "utils/detail/string.tpp"

#endif // UTILS_STRING_HPP
