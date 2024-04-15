
#pragma once

#ifndef UTILS_SPECIFIERS_HPP
#define UTILS_SPECIFIERS_HPP

#include "utils/string/conversions.hpp"

#include <string> // std::string
#include <string_view> // std::string_view

namespace utils {
    
    enum class Justification {
        Left,
        Right,
        Center
    };
    
    [[nodiscard]] std::string to_string(Justification justification);
    
    template <>
    [[nodiscard]] Justification from_string<Justification>(std::string_view str);
    
    
    enum class Sign {
        NegativeOnly,
        Aligned,
        Both,
    };
    
    [[nodiscard]] std::string to_string(Sign sign);
    
    template <>
    [[nodiscard]] Sign from_string<Sign>(std::string_view str);
    
}

// Template definitions.
#include "utils/detail/string/specifier.tpp"

#endif // UTILS_SPECIFIERS_HPP
