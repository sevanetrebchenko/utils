
#pragma once

#ifndef UTILS_FORMATTING_TPP
#define UTILS_FORMATTING_TPP

namespace utils {
    
    template <typename T>
    bool Formatting::Specifier::operator==(T other) const {
        using Type = std::decay<T>::type;
        
        if constexpr (std::is_same<Type, const char*>::value) {
            return strcmp(m_raw.c_str(), other) == 0;
        }
        else {
            return to<Type>() == other;
        }
    }
    
    template <typename T>
    [[nodiscard]] T Formatting::Specifier::to() const {
        using Type = std::decay<T>::type;
        return static_cast<Type>(*this);
    }
    
    template <typename T>
    Formatting::Specifier::operator T() const {
        using Type = std::decay<T>::type;
        
        // Conversion from raw type std::string to a string type (std::string, const char*, std::string_view) can be done directly and does not require a from_string function call.
        if constexpr (std::is_same<Type, std::string>::value || std::is_same<Type, std::string_view>::value) {
            return m_raw;
        }
        else if constexpr (std::is_same<Type, const char*>::value) {
            return m_raw.c_str();
        }
        else {
            return from_string<T>(m_raw);
        }
    }
    
}

#endif // UTILS_FORMATTING_TPP