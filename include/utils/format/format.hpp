
#ifndef FORMAT_HPP
#define FORMAT_HPP

#include "utils/concepts.hpp"

#include <cstdint> // std::uint8_t, std::size_t
#include <string> // std::string
#include <string_view> // std::string_view
#include <variant> // std::variant
#include <source_location> // std::source_location
#include <stdexcept> // std::runtime_error
#include <vector> // std::vector

namespace utils {
    
    class FormatString {
        public:
            struct Identifier {
                enum class Type : std::uint8_t {
                    Auto = 0,
                    Position,
                    Name,
                };
                
                Identifier();
                Identifier(std::size_t position);
                Identifier(std::string name);
                ~Identifier();
                
                bool operator==(const Identifier& other) const;

                Type type;
                std::size_t position;
                std::string name;
            };
            
            class Specification {
                public:
                    enum class Type : std::uint8_t {
                        FormattingGroupList,
                        SpecifierList
                    };
                    
                    Specification();
                    ~Specification();
                    
                    bool operator==(const Specification& other) const;
                    bool operator!=(const Specification& other) const;

                    // Used to distinguish the type of data this Specification contains
                    Type type() const;

                    // Returns the number of groups or the number of specifiers
                    std::size_t size() const;
                    bool empty() const;
                    
                    // Methods for specifier lists
                    
                    void set_specifier(std::string_view key, std::string value);
                    
                    std::string& operator[](std::string_view key);
                    std::string& get_specifier(std::string_view key);
                    
                    std::string_view get_specifier(std::string_view key) const;
                    std::string_view operator[](std::string_view key) const;
                    
                    bool has_specifier(std::string_view specifier) const;
                    
                    // Methods for formatting group lists
                    
                    const Specification& get_formatting_group(std::size_t index) const;
                    const Specification& operator[](std::size_t index) const;
                    
                    Specification& get_formatting_group(std::size_t index);
                    Specification& operator[](std::size_t index);
                    
                    bool has_group(std::size_t index) const;
                    
                private:
                    struct Specifier {
                        Specifier(std::string name, std::string value = "");
                        ~Specifier();
                        
                        bool operator==(const Specifier& other) const;
                        bool operator!=(const Specifier& other) const;
                        
                        std::string name;
                        std::string value;
                    };
                    
                    using SpecifierList = std::vector<Specifier>;
                    using FormattingGroupList = std::vector<Specification*>;
                    
                    // Conversion from specifier list to formatting group list
                    Specification(SpecifierList&& specifiers);
                    
                    // A specification can either be a mapping of key - value pairs (specifier name / value) or a nested specification group
                    // Specifiers are stored in a std::vector instead of std::unordered map as the number of formatting specifiers is expected to be relatively small
                    std::variant<SpecifierList, FormattingGroupList> m_spec;
                    Type m_type;
            };
            
            template <String T>
            FormatString(T fmt, std::source_location source = std::source_location::current());
            FormatString(const FormatString& fmt, std::source_location = std::source_location::current());
            ~FormatString();

            template <typename ...Ts>
            [[nodiscard]] FormatString format(const Ts&... args);
            
            [[nodiscard]] operator std::string() const;
            [[nodiscard]] std::string string() const;
            
            [[nodiscard]] std::source_location source() const;
            
            // Retrieves the total number of unique placeholders.
            // Placeholder uniqueness is determined by the identifier and formatting.
            [[nodiscard]] std::size_t get_placeholder_count() const;
            [[nodiscard]] std::size_t get_positional_placeholder_count() const;
            [[nodiscard]] std::size_t get_named_placeholder_count() const;

        private:
            struct Placeholder {
                std::size_t identifier_index;
                std::size_t specification_index;
                std::size_t position;
                bool formatted;
            };
            
            void parse();
            void register_placeholder(const Identifier& identifier, const Specification& spec, std::size_t position);

            std::string m_format;
            std::source_location m_source;
            
            std::vector<Identifier> m_identifiers;
            std::vector<Specification> m_specifications;
            
            std::vector<Placeholder> m_placeholders;
    };
    
    template <typename T>
    struct NamedArgument {
        NamedArgument(std::string_view name, const T& value);
        ~NamedArgument();
    
        std::string_view name;
        const T& value; // Store reference to avoid copying non-trivially copyable types
    };
    
    struct FormattedError : public std::runtime_error {
        template <typename ...Ts>
        FormattedError(FormatString fmt, const Ts&... args);
    };
    
    template <typename ...Ts>
    FormatString format(FormatString fmt, const Ts&... args);
    
}

// Template definitions
#include "utils/detail/format/format.tpp"

#endif // FORMAT_HPP
