
#ifndef UTILS_FORMATTING_HPP
#define UTILS_FORMATTING_HPP

#include <string> // std::string
#include <unordered_map> // std::unordered_map

namespace utils {
    
    class Formatting {
        public:
            class Specifier {
                public:
                    Specifier(std::string value = "");
                    ~Specifier();
                    
                    [[nodiscard]] bool operator==(const Specifier& other) const;
                    
                    template <typename T>
                    [[nodiscard]] bool operator==(T other) const;
                    
                    Specifier& operator=(const std::string& value);
                    
                    // Conversion operators.
                    template <typename T>
                    [[nodiscard]] T to() const;
                    
                    template <typename T>
                    [[nodiscard]] operator T() const;
    
                private:
                    std::string m_raw;
            };

            Formatting();
            ~Formatting();
            
            [[nodiscard]] Specifier& operator[](const std::string& key);
            [[nodiscard]] Specifier operator[](const std::string& key) const;
            
            [[nodiscard]] bool operator==(const Formatting& other) const;
            
            [[nodiscard]] Formatting nested() const;
            
        private:
            std::unordered_map<std::string, Specifier> m_specifiers;
            std::string m_raw;
    };
    
}

// Template definitions.
#include "utils/detail/string/formatting.tpp"

#endif // UTILS_FORMATTING_HPP
