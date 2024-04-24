
#pragma once

#ifndef UTILS_FORMAT_HPP
#define UTILS_FORMAT_HPP

#include "utils/string/formatting.hpp"

#include <string> // std::string
#include <unordered_map> // std::unordered_map
#include <source_location> // std::source_location
#include <functional> // std::function

namespace utils {
    
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
    
    template <typename ...Ts>
    [[nodiscard]] std::string format(const FormatString& fmt, const Ts&... args);
    
    template <typename T>
    struct NamedArgument {
        NamedArgument(std::string name, const T& value);
        ~NamedArgument();
    
        std::string name;
        T value;
    };
    
    // Character types
    std::string to_string(char value, const Formatting& formatting);
    
    // Integer types
    std::string to_string(unsigned char value, const Formatting& formatting);
    std::string to_string(short value, const Formatting& formatting);
    std::string to_string(unsigned short value, const Formatting& formatting);
    std::string to_string(int value, const Formatting& formatting);
    std::string to_string(unsigned value, const Formatting& formatting);
    std::string to_string(long value, const Formatting& formatting);
    std::string to_string(unsigned long value, const Formatting& formatting);
    std::string to_string(long long value, const Formatting& formatting);
    std::string to_string(unsigned long long value, const Formatting& formatting);
    
    // Floating-point types
    std::string to_string(float value, const Formatting& formatting);
    std::string to_string(double value, const Formatting& formatting);
    std::string to_string(long double value, const Formatting& formatting);
    
    // String types
    std::string to_string(const char* value, const Formatting& formatting);
    std::string to_string(std::string_view value, const Formatting& formatting);
    std::string to_string(const std::string& value, const Formatting& formatting);
    
    // Standard types
    template <typename T>
    std::string to_string(const T* value, const Formatting& formatting);
    std::string to_string(std::nullptr_t value, const Formatting& formatting);
    
    template <typename T, typename U>
    std::string to_string(const std::pair<T, U>& value, const Formatting& formatting);
    template <typename ...Ts>
    std::string to_string(const std::tuple<Ts...>& value, const Formatting& formatting);
    
    std::string to_string(const std::source_location& value, const Formatting& formatting);
    
    // Standard containers
    template <typename T>
    std::string to_string(const T& value, const Formatting& formatting) requires is_const_iterable<T>;
    
    template <typename T>
    std::string to_string(const NamedArgument<T>& value, const Formatting& formatting);
    
}

// Template definitions.
#include "utils/detail/string/format.tpp"

#endif // UTILS_FORMAT_HPP
