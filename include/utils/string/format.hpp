
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
            void validate_formatting(const Formatting& formatting) const;
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
        T value;
    };
    
    template <typename ...Ts>
    class NamedArgumentList {
        public:
            NamedArgumentList(NamedArgument<Ts>&&... args);
            ~NamedArgumentList();
            
            [[nodiscard]] const std::tuple<NamedArgument<Ts>...>& to_tuple() const;
            
            // Access NamedArguments by name
            template <typename T>
            [[nodiscard]] const T& get(std::string_view name) const;
            
            template <typename T>
            [[nodiscard]] T& get(std::string_view name);
            
        private:
            template <typename Fn, std::size_t Index = 0u>
            void get(std::string_view name, const Fn& fn);
            
            std::tuple<NamedArgument<Ts>...> m_tuple;
    };
    
    template <typename ...Ts>
    [[nodiscard]] std::string format(const FormatString& fmt, const Ts&... args);
    
    // Well-defined custom types that use this library should expose a set of named arguments that correspond to data of the class
    // These named arguments can be used in custom type format strings that override the default to_string implementation.
    template <typename T>
    void push_format_override(std::string fmt);
    
    template <typename T>
    void pop_format_override();
    
}

// Template definitions.
#include "utils/detail/string/format.tpp"

#endif // UTILS_FORMAT_HPP
