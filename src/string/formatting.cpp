
#include "utils/string/formatting.hpp"
#include "utils/exceptions.hpp"

namespace utils {
    
    // Formatting implementation

    Formatting::Formatting() = default;
    
    Formatting::~Formatting() = default;
    
    std::string& Formatting::operator[](const std::string& key) {
        return m_specifiers[key];
    }
    
    std::string_view Formatting::operator[](const std::string& key) const {
        auto iter = m_specifiers.find(key);
        if (iter != m_specifiers.end()) {
            return { iter->second };
        }
        
        return "";
    }
    
    bool Formatting::has_specifier(const std::string& key) const {
        auto iter = m_specifiers.find(key);
        return iter != m_specifiers.end();
    }
    
    bool Formatting::operator==(const Formatting& other) const {
        return m_specifiers == other.m_specifiers;
    }
    
    std::string_view Formatting::get_specifier(const std::string& key) const {
        auto iter = m_specifiers.find(key);
        if (iter != m_specifiers.end()) {
            return { iter->second };
        }
        
        return "";
    }
    
}