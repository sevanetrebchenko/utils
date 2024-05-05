
#pragma once

#ifndef FORMAT_HPP
#define FORMAT_HPP

#include "utils/concepts.hpp"

#include <cstdint> // std::uint8_t
#include <string_view> // std::string_view
#include <string> // std::string
#include <unordered_map> // std::unordered_map
#include <variant> // std::variant
#include <source_location> // std::source_location
#include <stdexcept> // std::runtime_error

namespace utils {

    // Forward declarations
    template <typename T, typename E>
    class ParseResult;
    
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
                Identifier(std::string_view name);
                ~Identifier();
                
                bool operator==(const Identifier& other) const;

                Type type;
                std::string_view name;
                std::size_t position;
            };
            
            class Specification {
                public:
                    Specification();
                    ~Specification();
                    
                    bool operator==(const Specification& other) const;
                    
                    void add_group(const Specification& spec);
                    const Specification& get_group(std::size_t index) const;
                    bool has_group(std::size_t index) const;
                    
                    void add_specifier(std::size_t group, std::string_view key, std::string_view value);
                    std::string_view get_specifier(std::size_t group, std::string_view key) const;
                    bool has_specifier(std::size_t group, std::string_view key) const;
                    
                    bool empty() const;
                    
                private:
                    using SpecifierList = std::unordered_map<std::string_view, std::string_view>;
                    
                    SpecifierList& get_specifier_mapping(std::size_t group);
                    const SpecifierList& get_specifier_mapping(std::size_t group) const;
                    
                    // A specification group can either be:
                    //   - A mapping of key - value pairs, each representing a format specifier and value
                    //   - A nested specification group
                    std::vector<std::variant<SpecifierList, Specification*>> m_groups;
            };
            
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
            // A placeholder combines an Identifier and a formatting Specification
            struct Placeholder {
                Placeholder(std::size_t identifier_index, const Specification& spec);
                ~Placeholder();
                
                void add_insertion_point(std::size_t position);
                
                std::size_t identifier_index;
                Specification spec;
                
                std::vector<std::size_t> insertion_points;
            };
            
            ParseResult<Specification, std::string_view> parse_specification(std::string_view in, bool nested = false);
            
            void parse();
            void register_placeholder(const Identifier& identifier, const Specification& spec, std::size_t position);

            std::string m_format;
            std::string m_result;
            std::source_location m_source;
            std::vector<Identifier> m_identifiers;
            std::vector<Placeholder> m_placeholders;
    };
    
    template <typename T>
    struct NamedArgument {
        NamedArgument(std::string name, const T& value);
        ~NamedArgument();
    
        std::string name;
        T value;
    };
    
    template <typename ...Ts>
    std::string format(const FormatString& fmt, const Ts&... args);
    
    struct FormattedError : public std::runtime_error {
        template <typename ...Ts>
        FormattedError(FormatString fmt, const Ts&... args);
    };
    
    template <typename T>
    struct Formatter {
    };
    
    // Shared formatters
    
    template <typename T>
    class IntegerFormatter {
        public:
            IntegerFormatter();
            ~IntegerFormatter();
            
            void parse(const FormatString::Specification& spec);
            std::string format(T value);
        
        private:
            inline std::string to_base(T value, int base) const;
            
            enum class Representation : std::uint8_t {
                Decimal,
                Binary,
                Hexadecimal
            } m_representation;
            
            enum class Sign : std::uint8_t {
                NegativeOnly,
                Aligned,
                Both,
                None
            } m_sign;
            
            enum class Justification : std::uint8_t {
                Left,
                Right,
                Center
            } m_justification;
            
            std::uint32_t m_width;
            char m_fill;
            
            char m_padding;
            char m_separator;
            
            // Optional specifiers for alternate representations
            bool m_use_base_prefix;
            std::uint8_t m_group_size;
    };
    
    template <typename T>
    class FloatingPointFormatter {
        public:
            FloatingPointFormatter();
            ~FloatingPointFormatter();
            
            void parse(const FormatString::Specification& spec);
            std::string format(T value);

        private:
            enum class Representation : std::uint8_t {
                Fixed,
                Scientific,
            } m_representation;
            
            enum class Sign : std::uint8_t {
                NegativeOnly,
                Aligned,
                Both,
                None
            } m_sign;
            
            enum class Justification : std::uint8_t {
                Left,
                Right,
                Center
            } m_justification;
            
            std::uint32_t m_width;
            char m_fill;
            
            std::uint8_t m_precision;
            char m_separator;
    };
    
    template <typename T>
    class StringFormatter {
        public:
            StringFormatter();
            ~StringFormatter();
            
            void parse(const FormatString::Specification& spec);
            std::string format(const T& value) const;
        
        private:
            enum class Justification : std::uint8_t {
                Left,
                Right,
                Center
            } m_justification;
            
            std::uint32_t m_width;
            char m_fill;
    };
    
    // Character types
    
    template <>
    class Formatter<char> {
        public:
            Formatter();
            ~Formatter();
            
            void parse(const FormatString::Specification& spec);
            std::string format(char value);
            
        private:
            enum class Justification : std::uint8_t {
                Left,
                Right,
                Center
            } m_justification;
            
            std::uint32_t m_width;
            char m_fill;
    };
    
    // Integer types
    
    template <>
    struct Formatter<unsigned char> : IntegerFormatter<unsigned char> {
    };
    
    template <>
    struct Formatter<short> : IntegerFormatter<short> {
    };
    
    template <>
    struct Formatter<unsigned short> : IntegerFormatter<unsigned short> {
    };
    
    template <>
    struct Formatter<int> : IntegerFormatter<int> {
    };
    
    template <>
    struct Formatter<unsigned> : IntegerFormatter<unsigned> {
    };
    
    template <>
    struct Formatter<long> : IntegerFormatter<long> {
    };
    
    template <>
    struct Formatter<unsigned long> : IntegerFormatter<unsigned long> {
    };
    
    template <>
    struct Formatter<long long> : IntegerFormatter<long long> {
    };
    
    template <>
    struct Formatter<unsigned long long> : IntegerFormatter<unsigned long long> {
    };
    
    // Floating-point types
    
    template <>
    struct Formatter<float> : FloatingPointFormatter<float> {
    };

    template <>
    struct Formatter<double> : FloatingPointFormatter<double> {
    };
    
    template <>
    struct Formatter<long double> : FloatingPointFormatter<long double> {
    };
    
    // String types
    
    template <>
    struct Formatter<const char*> : StringFormatter<const char*> {
    };
    
    template <>
    struct Formatter<std::string_view> : StringFormatter<std::string_view> {
    };
    
    template <>
    struct Formatter<std::string> : StringFormatter<std::string> {
    };
    
    // Container types
    
    template <Container T>
    class Formatter<T> {
        public:
            void parse(const FormatString::Specification& spec);
            std::string format(const std::vector<T>& value) const;
        
        private:
            Formatter<T> m_formatter;
    };
    
    // Standard types
    
    template <>
    class Formatter<std::source_location> {
        public:
            void parse(const FormatString::Specification& spec);
            std::string format(const std::source_location& value) const;
            
        private:
            Formatter<std::uint64_t> m_line_formatter;
            Formatter<const char*> m_filename_formatter;
    };

    // User-defined / custom types
    
    template <typename T>
    class Formatter<NamedArgument<T>> {
        public:
            void parse(const FormatString::Specification& spec);
            std::string format(const NamedArgument<T>& value) const;
            
        private:
            Formatter<T> m_formatter;
    };

}

// Template definitions
#include "utils/detail/format.tpp"

#endif // FORMAT_HPP
