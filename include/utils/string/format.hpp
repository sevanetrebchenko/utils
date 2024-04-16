
#pragma once

#ifndef UTILS_FORMAT_HPP
#define UTILS_FORMAT_HPP

#include <string> // std::string
#include <unordered_map> // std::unordered_map
#include <source_location> // std::source_location

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
    
    class FormatString {
        public:
            FormatString(std::string_view fmt, std::source_location source = std::source_location::current());
            FormatString(const char* fmt, std::source_location source = std::source_location::current());
            FormatString(std::string fmt, std::source_location source = std::source_location::current());
            ~FormatString();

            template <typename ...Ts>
            [[nodiscard]] std::string format(const Ts&... args) const;
            
            [[nodiscard]] std::string_view format_string() const;
            [[nodiscard]] std::source_location source() const;
            
            // Retrieves the total number of unique placeholders.
            // Placeholder uniqueness is determined by the identifier and formatting.
            [[nodiscard]] std::size_t get_placeholder_count() const;
            [[nodiscard]] std::size_t get_positional_placeholder_count() const;
            [[nodiscard]] std::size_t get_named_placeholder_count() const;

        private:
            struct Identifier {
                enum class Type {
                    Auto = 0,
                    Position,
                    Name,
                };

                Identifier();
                explicit Identifier(std::size_t position);
                explicit Identifier(std::string name);
                ~Identifier();

                [[nodiscard]] bool operator==(const Identifier& other) const;

                Type type;
                std::size_t position;
                std::string name;
            };

            struct FormattedPlaceholder {
                FormattedPlaceholder(std::size_t identifier_index, const Formatting& formatting);
                ~FormattedPlaceholder();
                
                void add_insertion_point(std::size_t position);
                
                std::size_t identifier_index;
                Formatting formatting;
                
                std::vector<std::size_t> insertion_points;
            };
            
            void parse();
            void register_placeholder(const Identifier& identifier, const Formatting& formatting, std::size_t position);

            std::string m_format;
            std::string m_result;
            std::source_location m_source;
            std::vector<Identifier> m_placeholder_identifiers;
            std::vector<FormattedPlaceholder> m_formatted_placeholders;
    };
    
    template <typename T>
    struct NamedArgument {
        NamedArgument(std::string name, const T& value);
        ~NamedArgument();
    
        std::string name;
        const T& value; // Maintain reference for making dealing with non-trivially copyable types easier
    };
    
    template <typename ...Ts>
    [[nodiscard]] std::string format(const FormatString& fmt, const Ts&... args);
    
}

// Template definitions.
#include "utils/detail/string/format.tpp"

#endif // UTILS_FORMAT_HPP
