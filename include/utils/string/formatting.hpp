
#ifndef UTILS_FORMATTING_HPP
#define UTILS_FORMATTING_HPP

#include "utils/result.hpp"

#include <string> // std::string
#include <unordered_map> // std::unordered_map
#include <functional> // std::function

namespace utils {
    
    class Formatting {
        public:
            class Specifier {
                public:
                    Specifier(std::string value);
                    ~Specifier();
                    
                    template <typename T>
                    [[nodiscard]] bool operator==(T other) const;
                    [[nodiscard]] bool operator==(const Specifier& other) const;
                    
                    Specifier& operator=(const std::string& value);
                    
                    // Conversion operators.
                    template <typename T>
                    [[nodiscard]] T convert_to() const;
                    
                private:
                    std::string m_raw;
            };

            Formatting();
            ~Formatting();
            
            [[nodiscard]] bool operator==(const Formatting& other) const;
            
            Specifier& add_specifier(const std::string& key, std::string value);
            
            Specifier& get_specifier(const std::string& key);
            Specifier& operator[](const std::string& key);
            const Specifier& get_specifier(const std::string& key) const;
            const Specifier& operator[](const std::string& key) const;
            
            bool has_specifier(const std::string& key) const;
            
        private:
            void parse();
            
            std::unordered_map<std::string, Specifier> m_specifiers;
            std::string m_raw;
    };
    
}

// Template definitions.
#include "utils/detail/string/formatting.tpp"

#endif // UTILS_FORMATTING_HPP
