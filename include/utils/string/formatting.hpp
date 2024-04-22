
#pragma once

#ifndef UTILS_FORMATTING_HPP
#define UTILS_FORMATTING_HPP

#include "utils/result.hpp"

#include <string> // std::string
#include <unordered_map> // std::unordered_map
#include <functional> // std::function

namespace utils {
    
    class Formatting {
        public:
            Formatting();
            ~Formatting();
            
            [[nodiscard]] bool operator==(const Formatting& other) const;
            
            std::string& add_specifier(const std::string& key, std::string value);
            
            std::string& get_specifier(const std::string& key);
            std::string& operator[](const std::string& key);
            std::string_view get_specifier(const std::string& key) const;
            std::string_view operator[](const std::string& key) const;
            
            bool has_specifier(const std::string& key) const;
            
        private:
            std::unordered_map<std::string, std::string> m_specifiers;
    };
    
}

// Template definitions.
#include "utils/detail/string/formatting.tpp"

#endif // UTILS_FORMATTING_HPP
