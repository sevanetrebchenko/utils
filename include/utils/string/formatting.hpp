
#pragma once

#ifndef UTILS_FORMATTING_HPP
#define UTILS_FORMATTING_HPP

#include "utils/result.hpp"

#include <string> // std::string
#include <unordered_map> // std::unordered_map
#include <functional> // std::function

namespace utils {
    
    struct Formatting {
        Formatting();
        
        enum class Justification : std::uint8_t {
            Left,
            Right,
            Center
        } justification;
        
        enum class Sign : std::uint8_t {
            NegativeOnly,
            None,
            Aligned,
            Both
        } sign;
        
        enum class Representation : std::uint8_t {
            Decimal,
            
            // Integer type representations
            Binary,
            Hexadecimal,
            
            // Floating point type representations
            Fixed,
            Scientific
        } representation;
        
        std::uint8_t precision;
        std::uint32_t width;
        
        char fill;
        char padding;
        
        char separator;
        
        // Optional specifiers for alternate representations
        bool use_base_prefix;
        std::uint8_t group_size;
        
        // Nested formatting
        Formatting* next;
    };
    
}

#endif // UTILS_FORMATTING_HPP
