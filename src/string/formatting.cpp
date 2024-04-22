
#include "utils/string/formatting.hpp"
#include "utils/exceptions.hpp"

namespace utils {

    Formatting::Specifier::Specifier(std::string value) : m_raw(std::move(value)) {
    }
    
    Formatting::Specifier::~Specifier() = default;
    
    bool Formatting::Specifier::operator==(const Formatting::Specifier& other) const {
        return m_raw == other.m_raw;
    }
    
    Formatting::Specifier& Formatting::Specifier::operator=(const std::string& value) {
        m_raw = value;
        return *this;
    }
    
    // Formatting implementation

    Formatting::Formatting() = default;
    
    Formatting::~Formatting() = default;
    
    Formatting::Specifier& Formatting::operator[](const std::string& key) {
        return m_specifiers[key];
    }
    
    const Formatting::Specifier& Formatting::operator[](const std::string& key) const {
        auto iter = m_specifiers.find(key);
        if (iter != m_specifiers.end()) {
            return { iter->second };
        }
        
        // TODO:
        throw FormattedError("");
    }
    
    bool Formatting::has_specifier(const std::string& key) const {
        auto iter = m_specifiers.find(key);
        return iter == m_specifiers.end();
    }
    
    bool Formatting::operator==(const Formatting& other) const {
        return m_specifiers == other.m_specifiers;
    }

}