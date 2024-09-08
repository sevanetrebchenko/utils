//
//#pragma once
//
//#ifndef STRING_HPP
//#define STRING_HPP
//
//#include "utils/concepts.hpp"
//#include <vector> // std::vector
//#include <string> // std::string
//#include <string_view> // std::string_view
//#include <source_location> // std::source_location
//#include <variant> // std::variant
//#include <stdexcept> // std::runtime_error
//#include <ostream> // std::ostream
//#include <optional> // std::optional
//#include <functional> // std::function
//
//namespace utils {
//
//    // Forward declarations
//    template <typename T, typename E>
//    class ParseResult;
//
//    template <typename E>
//    class ParseResponse;
//
//
//    // Returns a vector containing the result of splitting 'in' by 'delimiter'.
//    [[nodiscard]] std::vector<std::string> split(std::string_view in, std::string_view delimiter);
//
//    // Trim off all whitespace characters on either side of 'in'.
//    [[nodiscard]] std::string_view trim(std::string_view in);
//
//    // std::strcasecmp requires null-terminated strings
//    template <String T, String U>
//    [[nodiscard]] bool icasecmp(const T& first, const U& second);
//
//    template <String T, String U>
//    [[nodiscard]] bool operator==(const T& first, const U& second);
//
//    // from_string for fundamental types (wrapper around std::from_chars)
//    // Returns the number of characters processed, throws on invalid argument
//
//    std::size_t from_string(std::string_view in, unsigned char& out);
//    std::size_t from_string(std::string_view in, short& out);
//    std::size_t from_string(std::string_view in, unsigned short& out);
//    std::size_t from_string(std::string_view in, int& out);
//    std::size_t from_string(std::string_view in, unsigned& out);
//    std::size_t from_string(std::string_view in, long& out);
//    std::size_t from_string(std::string_view in, unsigned long& out);
//    std::size_t from_string(std::string_view in, long long& out);
//    std::size_t from_string(std::string_view in, unsigned long long& out);
//
//    // Supports both scientific and fixed notation
//    std::size_t from_string(std::string_view in, float& out);
//    std::size_t from_string(std::string_view in, double& out);
//    std::size_t from_string(std::string_view in, long double& out);
//
//    // Section: Formatting
//
//    template <typename T>
//    struct Formatter;
//
//    class FormatString {
//        public:
//            struct Identifier {
//                enum class Type : std::uint8_t {
//                    Auto = 0,
//                    Position,
//                    Name,
//                };
//
//                Identifier();
//                Identifier(std::size_t position);
//                Identifier(std::string name);
//                ~Identifier();
//
//                bool operator==(const Identifier& other) const;
//
//                Type type;
//                std::size_t position;
//                std::string name;
//            };
//
//            class Specification {
//                public:
//                    enum class Type : std::uint8_t {
//                        FormattingGroupList,
//                        SpecifierList
//                    };
//
//                    struct Specifier {
//                        Specifier(std::string name, std::string value = "");
//                        ~Specifier();
//
//                        Specifier& operator=(std::string other);
//
//                        [[nodiscard]] bool operator==(const Specifier& other) const;
//                        [[nodiscard]] bool operator!=(const Specifier& other) const;
//
//                        std::string name;
//                        std::string value;
//                    };
//
//                    Specification();
//                    ~Specification();
//
//                    bool operator==(const Specification& other) const;
//                    bool operator!=(const Specification& other) const;
//
//                    // Used to distinguish the type of data this Specification contains
//                    Type type() const;
//
//                    // Returns the number of groups or the number of specifiers
//                    std::size_t size() const;
//                    bool empty() const;
//
//                    // Methods for specifier lists
//
//                    void set_specifier(std::string_view specifier, std::string value);
//
//                    Specifier& operator[](std::string_view specifier);
//                    const Specifier& operator[](std::string_view specifier) const;
//
//                    Specifier& get_specifier(std::string_view specifier);
//                    const Specifier& get_specifier(std::string_view specifier) const;
//
//                    template <String T, String ...Ts>
//                    const Specifier& one_of(const T& first, const Ts&... rest) const;
//
//                    template <String ...Ts>
//                    bool has_specifier(const Ts&... specifiers) const;
//
//                    template <String ...Ts>
//                    void ignore(const Ts&... specifiers) const;
//
//                    template <String ...Ts>
//                    void enable(const Ts&... specifiers) const;
//
//                    // Methods for formatting group lists
//
//                    Specification& operator[](std::size_t index);
//                    const Specification& operator[](std::size_t index) const;
//
//                    Specification& get_formatting_group(std::size_t index);
//                    const Specification& get_formatting_group(std::size_t index) const;
//
//                    bool has_group(std::size_t index) const;
//
//                private:
//                    using SpecifierList = std::vector<Specifier>;
//                    using FormattingGroupList = std::vector<Specification*>;
//
//                    // Conversion from specifier list to formatting group list
//                    explicit Specification(SpecifierList&& specifiers);
//
//                    [[nodiscard]] bool has_specifier(std::string_view specifier) const;
//
//                    // A specification can either be a mapping of key - value pairs (specifier name / value) or a nested specification group
//                    // Specifiers are stored in a std::vector instead of std::unordered map as the number of formatting specifiers is expected to be relatively small
//                    std::variant<SpecifierList, FormattingGroupList> m_spec;
//                    Type m_type;
//            };
//
//            template <String T>
//            FormatString(T fmt, std::source_location source = std::source_location::current());
//            FormatString(const FormatString& fmt, std::source_location = std::source_location::current());
//            ~FormatString();
//
//            template <typename ...Ts>
//            [[nodiscard]] FormatString format(const Ts&... args);
//
//            [[nodiscard]] bool empty() const;
//
//            [[nodiscard]] operator std::string() const;
//            [[nodiscard]] std::string string() const;
//
//            [[nodiscard]] std::source_location source() const;
//
//            // Retrieves the total number of unique placeholders.
//            // Placeholder uniqueness is determined by the identifier and formatting.
//            [[nodiscard]] std::size_t get_placeholder_count() const;
//            [[nodiscard]] std::size_t get_positional_placeholder_count() const;
//            [[nodiscard]] std::size_t get_named_placeholder_count() const;
//
//        private:
//            struct Placeholder {
//                std::size_t identifier_index;
//                std::size_t specification_index;
//                std::size_t position;
//                bool formatted;
//            };
//
//            void parse();
//            [[nodiscard]] ParseResponse<FormatString> parse_specification(std::string_view in, Specification& spec, bool nested = false);
//            ParseResult<Specification::Specifier, FormatString> parse_specifier(std::string_view in);
//
//            void register_placeholder(const Identifier& identifier, const Specification& spec, std::size_t position);
//
//            std::string m_format;
//            std::source_location m_source;
//
//            std::vector<Identifier> m_identifiers;
//            std::vector<Specification> m_specifications;
//
//            std::vector<Placeholder> m_placeholders;
//    };
//
//    std::ostream& operator<<(std::ostream& os, const FormatString& fmt);
//
//    template <typename ...Ts>
//    FormatString format(FormatString fmt, const Ts&... args);
//
//    // Section: Formatters
//
//    // Format specification should be provided in the following format:
//    // {:GLOBAL:PER_ELEMENT}
//    // Global formatting groups are applied to the type globally using the top-level formatter
//    // Per-element formatting groups can vary per type and are applied individually
//
//    template <typename T>
//    struct Formatter {
//    };
//
//    template <typename T>
//    concept is_formattable = requires(Formatter<T> formatter, const FormatString::Specification& spec, const T& value) {
//        { formatter.parse(spec) };
//        { formatter.format(value) } -> std::same_as<std::string>;
//    };
//
//    // Shared formatters
//
//    template <typename T>
//    class IntegerFormatter {
//        public:
//            enum class Representation : std::uint8_t {
//                Decimal = 0,
//                Binary,
//                Hexadecimal
//            };
//
//            enum class Sign : std::uint8_t {
//                NegativeOnly = 0,
//                Aligned,
//                Both
//            };
//
//            enum class Justification : std::uint8_t {
//                Left = 0,
//                Right,
//                Center
//            };
//
//            IntegerFormatter();
//            ~IntegerFormatter();
//
//            void parse(const FormatString::Specification& spec, const FormatString::Identifier& identifier = {});
//            [[nodiscard]] std::string format(T value) const;
//
//            Representation representation = Representation::Decimal;
//            Sign sign = Sign::NegativeOnly;
//            Justification justification = Justification::Left;
//
//            std::size_t width = 0u;
//            char fill_character = ' ';
//
//            // For decimal representations, separates every 3 characters with a comma
//            // For binary / hexadecimal representations, separates every 'group_size' bits with a single quote (default: 4)
//            std::optional<bool> use_separator_character; // = false
//
//            // Specifies how many characters are in a single group
//            // Works in conjunction with 'use_separator_character'
//            // Note: only applicable for binary / hexadecimal representations
//            std::optional<std::uint8_t> group_size; // = 4u;
//
//            // Specifies whether to use a base prefix (0b for binary, 0x for hexadecimal)
//            // Note: only applicable for binary / hexadecimal representations
//            bool use_base_prefix = true;
//
//            // Specifies the total number of digits to use when formatting
//            // Appends leading zeroes if the number of digits required is less than the requested value
//            // Rounds up to the nearest multiple of 'group_size', if specified and/or > 0
//            // Note: only applicable for binary / hexadecimal representations
//            std::uint8_t digits = 0u;
//
//        private:
//            inline std::string to_decimal(T value) const;
//            inline std::string to_binary(T value) const;
//            inline std::string to_hexadecimal(T value) const;
//    };
//
//    template <typename T>
//    class FloatingPointFormatter {
//        public:
//            enum class Representation : std::uint8_t {
//                Fixed,
//                Scientific,
//            };
//
//            enum class Sign : std::uint8_t {
//                NegativeOnly,
//                Aligned,
//                Both
//            };
//
//            enum class Justification : std::uint8_t {
//                Left,
//                Right,
//                Center
//            };
//
//            FloatingPointFormatter();
//            ~FloatingPointFormatter();
//
//            void parse(const FormatString::Specification& spec);
//            std::string format(T value) const;
//
//            Representation representation = Representation::Fixed;
//            Sign sign = Sign::NegativeOnly;
//            Justification justification = Justification::Left;
//
//            unsigned width = 0u;
//            char fill_character = ' ';
//
//            std::uint8_t precision = std::numeric_limits<float>::digits10;
//            bool use_separator_character = false;
//
//        private:
//            inline std::size_t format_to(T value, FormattingContext* context) const;
//    };
//
//    template <typename T>
//    class StringFormatter {
//        public:
//            enum class Justification : std::uint8_t {
//                Left,
//                Right,
//                Center
//            };
//
//            StringFormatter();
//            ~StringFormatter();
//
//            void parse(const FormatString::Specification& spec);
//            std::string format(const T& value) const;
//
//            Justification justification = Justification::Left;
//
//            unsigned width = 0u;
//            char fill_character = ' ';
//
//        private:
//            inline std::size_t format_to(const T& value, FormattingContext* context) const;
//    };
//
//    // Character types
//
//    template <>
//    struct Formatter<char> : StringFormatter<char> {
//    };
//
//    // Integer types
//
//    template <>
//    struct Formatter<unsigned char> : IntegerFormatter<unsigned char> {
//    };
//
//    template <>
//    struct Formatter<short> : IntegerFormatter<short> {
//    };
//
//    template <>
//    struct Formatter<unsigned short> : IntegerFormatter<unsigned short> {
//    };
//
//    template <>
//    struct Formatter<int> : IntegerFormatter<int> {
//    };
//
//    template <>
//    struct Formatter<unsigned> : IntegerFormatter<unsigned> {
//    };
//
//    template <>
//    struct Formatter<long> : IntegerFormatter<long> {
//    };
//
//    template <>
//    struct Formatter<unsigned long> : IntegerFormatter<unsigned long> {
//    };
//
//    template <>
//    struct Formatter<long long> : IntegerFormatter<long long> {
//    };
//
//    template <>
//    struct Formatter<unsigned long long> : IntegerFormatter<unsigned long long> {
//    };
//
//    // Floating-point types
//
//    template <>
//    struct Formatter<float> : FloatingPointFormatter<float> {
//    };
//
//    template <>
//    struct Formatter<double> : FloatingPointFormatter<double> {
//    };
//
//    template <>
//    struct Formatter<long double> : FloatingPointFormatter<long double> {
//    };
//
//    // String types
//
//    template <>
//    struct Formatter<char*> : StringFormatter<char*> {
//    };
//
//    template <>
//    struct Formatter<char[]> : StringFormatter<char[]> {
//    };
//
//    template <>
//    struct Formatter<const char*> : StringFormatter<const char*> {
//    };
//
//    template <>
//    struct Formatter<const char[]> : StringFormatter<const char[]> {
//    };
//
//    template <>
//    struct Formatter<std::string_view> : StringFormatter<std::string_view> {
//    };
//
//    template <>
//    struct Formatter<std::string> : public StringFormatter<std::string> {
//    };
//
//    // Container types
//
//    template <Container T>
//    class Formatter<T> {
//        public:
//            void parse(const FormatString::Specification& spec) {}
//            std::string format(const T& value) const {
//                return "vector";
//            }
//
//        private:
//            // Formatter<T> m_formatter;
//    };
//
//    template <typename Key, typename Value, typename Hash, typename Predicate, typename Allocator>
//    class Formatter<std::unordered_map<Key, Value, Hash, Predicate, Allocator>> {
//        public:
//            using T = std::unordered_map<Key, Value, Hash, Predicate, Allocator>;
//
//            enum class Justification : std::uint8_t {
//                Left = 0,
//                Right,
//                Center
//            };
//
//            Formatter();
//            ~Formatter();
//
//            void parse(const FormatString::Specification& spec);
//            std::string format(const T& value) const;
//
//            Justification justification;
//
//            std::size_t width;
//            char fill_character;
//
//        private:
//            Formatter<std::pair<Key, Value>> m_formatter;
//    };
//
//    // Standard types
//
//    // Format: filename:line
//    template <>
//    class Formatter<std::source_location> {
//        public:
//            Formatter();
//            ~Formatter();
//
//            void parse(const FormatString::Specification& spec);
//            std::string format(const std::source_location& value) const;
//
//        private:
//            Formatter<std::pair<unsigned, const char*>> m_formatters;
//    };
//
//    template <typename T, typename U>
//    class Formatter<std::pair<T, U>> {
//        public:
//            Formatter();
//            ~Formatter();
//
//            void parse(const FormatString::Specification& spec);
//            std::string format(const std::pair<T, U>& value) const;
//
//        private:
//            std::pair<Formatter<T>, Formatter<U>> m_formatters;
//    };
//
//    template <typename ...Ts>
//    class Formatter<std::tuple<Ts...>> {
//        public:
//
//        private:
//            std::tuple<Ts...> m_formatters;
//    };
//
//    // User-defined / custom types
//
//    template <typename T>
//    struct Formatter<NamedArgument<T>> : Formatter<T> {
//        Formatter();
//        ~Formatter();
//
//        void parse(const FormatString::Specification& spec);
//        std::string format(const NamedArgument<T>& value) const requires is_formattable<T> ;
//    };
//
//}
//
//// Template definitions
//#include "utils/detail/string.tpp"
//
//#endif // STRING_HPP

#pragma once

#ifndef STRING_HPP
#define STRING_HPP

#include "utils/concepts.hpp"

#include <string> // std::string
#include <source_location> // std::source_location
#include <stdexcept> // std::runtime_error
#include <variant> // std::variant
#include <optional> // std::optional

namespace utils {
    
    // Returns a vector containing the result of splitting 'in' by 'delimiter'.
    [[nodiscard]] std::vector<std::string> split(std::string_view in, std::string_view delimiter);

    // Trim off all whitespace characters on either side of 'in'.
    [[nodiscard]] std::string_view trim(std::string_view in);

    // std::strcasecmp requires null-terminated strings
    template <String T, String U>
    [[nodiscard]] bool icasecmp(const T& first, const U& second);

    template <String T, String U>
    [[nodiscard]] bool operator==(const T& first, const U& second);

    // from_string for fundamental types (wrapper around std::from_chars)
    // Returns the number of characters processed, throws on invalid argument

    std::size_t from_string(std::string_view in, unsigned char& out);
    std::size_t from_string(std::string_view in, short& out);
    std::size_t from_string(std::string_view in, unsigned short& out);
    std::size_t from_string(std::string_view in, int& out);
    std::size_t from_string(std::string_view in, unsigned& out);
    std::size_t from_string(std::string_view in, long& out);
    std::size_t from_string(std::string_view in, unsigned long& out);
    std::size_t from_string(std::string_view in, long long& out);
    std::size_t from_string(std::string_view in, unsigned long long& out);

    // Supports both scientific and fixed notation
    std::size_t from_string(std::string_view in, float& out);
    std::size_t from_string(std::string_view in, double& out);
    std::size_t from_string(std::string_view in, long double& out);
    
    class FormatSpec {
        public:
            enum class Type {
                FormattingGroupList = 0,
                SpecifierList
            };
            
            FormatSpec();
            ~FormatSpec();
            
            FormatSpec(const FormatSpec& other);
            FormatSpec(FormatSpec&& other) noexcept;
            FormatSpec& operator=(const FormatSpec& other);
            
            Type type() const;
            bool empty() const;
            std::size_t size() const;
            
            bool operator==(const FormatSpec& other) const;
            bool operator!=(const FormatSpec& other) const;
            
            // Methods for specifier lists
            
            void set_specifier(std::string_view name, std::string value);
            
            std::string& operator[](std::string_view name);
            std::string_view operator[](std::string_view name) const;

            std::string& get_specifier(std::string_view name);
            
            template <typename ...Ts>
            std::string_view get_specifier(std::string_view first, std::string_view second, Ts... rest) const;
            std::string_view get_specifier(std::string_view name) const;

            template <typename ...Ts>
            bool has_specifier(std::string_view first, std::string_view second, Ts... rest) const;
            bool has_specifier(std::string_view specifier) const;
            
            // Methods for formatting groups
            
            FormatSpec& operator[](std::size_t index);
            const FormatSpec& operator[](std::size_t index) const;
            
            FormatSpec& get_group(std::size_t index);
            const FormatSpec& get_group(std::size_t index) const;
            
            bool has_group(std::size_t index) const;
            
        private:
            struct Specifier {
                Specifier(std::string name, std::string value = "");
                ~Specifier();

                Specifier& operator=(std::string other);
                
                [[nodiscard]] bool operator==(const Specifier& other) const;
                [[nodiscard]] bool operator!=(const Specifier& other) const;
                
                std::string name;
                std::string value;
            };
            
            using SpecifierList = std::vector<Specifier>;
            using FormattingGroupList = std::vector<FormatSpec*>;
            
            // Conversion from specifier list to formatting group list
            explicit FormatSpec(SpecifierList&& specifiers);
            
            // A specification can either be a mapping of key - value pairs (specifier name / value) or a nested specification group
            // Specifiers are stored in a std::vector instead of std::unordered map as the number of formatting specifiers is expected to be relatively small
            std::variant<SpecifierList, FormattingGroupList> m_spec;
            Type m_type;
    };
    
    template <typename T>
    struct NamedArgument {
        using type = T;
        
        NamedArgument(std::string_view name, const T& value);
        ~NamedArgument();
    
        std::string_view name;
        const T& value; // Store as a reference to avoid copying non-trivially copyable types
    };
    
    // Function throws exception if a placeholder does not have an argument specified
    template <typename ...Ts>
    std::string format(std::string_view fmt, const Ts&... args);
    
    // Section: Formatters
    
    enum class Justification {
        Left = 0,
        Right,
        Center
    };
    
    enum class Sign {
        NegativeOnly = 0,
        Aligned,
        Both
    };
    
    template <typename T>
    struct Formatter {
        Formatter();
        ~Formatter();
        
        void parse(const FormatSpec& spec);
        std::string format(const T& value) const;
        
        std::size_t width;
        char fill_character;
    };
    
    namespace detail {
        
        template <typename T>
        class IntegerFormatter {
            public:
                IntegerFormatter();
                ~IntegerFormatter();
                
                void parse(const FormatSpec& spec);
                std::string format(T value) const;
                
                enum class Representation {
                    Decimal = 0,
                    Binary,
                    Hexadecimal
                } representation;
                
                Sign sign;
                Justification justification;
                std::size_t width;
                char fill_character;
                
                // For decimal representations, separates every 3 characters with a comma
                // For binary / hexadecimal representations, separates every 'group_size' bits with a single quote
                std::optional<bool> use_separator_character;
                
                // Specifies how many characters are in a single group (works in conjunction with 'use_separator_character' specifier)
                // Only applicable to binary / hexadecimal representations
                std::optional<std::uint8_t> group_size;
                
                // Specifies whether to use a base prefix (0b for binary, 0x for hexadecimal)
                // Only applicable to binary / hexadecimal representations
                bool use_base_prefix;
                
                // Specifies the total number of digits to use when formatting
                // The number of digits is rounded up to the nearest multiple of 'group_size', if specified
                // Only applicable to binary / hexadecimal representations
                std::optional<std::uint8_t> digits;
                
            private:
                inline std::string to_binary(T value) const;
                inline std::string to_decimal(T value) const;
                inline std::string to_hexadecimal(T value) const;
        };
        
    }

    // Integer types
    
    // char
    template <>
    struct Formatter<char> {
        Formatter();
        ~Formatter();
        
        void parse(const FormatSpec& spec);
        std::string format(char c) const;
        
        Justification justification;
        std::size_t width;
        char fill_character;
    };
    
    // signed char
    template <>
    struct Formatter<std::int8_t> : public detail::IntegerFormatter<std::int8_t> {
    };
    
    // unsigned char
    template <>
    struct Formatter<std::uint8_t> : detail::IntegerFormatter<std::uint8_t> {
    };
    
    // short
    template <>
    struct Formatter<std::int16_t> : detail::IntegerFormatter<std::int16_t> {
    };
    
    // unsigned short
    template <>
    struct Formatter<std::uint16_t> : detail::IntegerFormatter<std::uint16_t> {
    };
    
    // int
    template <>
    struct Formatter<std::int32_t> : detail::IntegerFormatter<std::int32_t> {
    };
    
    // unsigned int
    template <>
    struct Formatter<std::uint32_t> : detail::IntegerFormatter<std::uint32_t> {
    };
    
    // long
    template <>
    struct Formatter<long> : detail::IntegerFormatter<long> {
    };
    
    // unsigned long
    template <>
    struct Formatter<unsigned long> : detail::IntegerFormatter<unsigned long> {
    };
    
    // long long
    template <>
    struct Formatter<std::int64_t> : detail::IntegerFormatter<std::int64_t> {
    };
    
    // unsigned long long
    template <>
    struct Formatter<std::uint64_t> : detail::IntegerFormatter<std::uint64_t> {
    };
    
    // Floating point types
    
    namespace detail {
        
        template <typename T>
        struct FloatingPointFormatter {
            FloatingPointFormatter();
            ~FloatingPointFormatter();
            
            void parse(const FormatSpec& spec);
            std::string format(T value) const;
            
            enum class Representation {
                Decimal = 0,
                Binary,
                Hexadecimal
            } representation;
            
            Sign sign;
            Justification justification;
            std::size_t width;
            char fill_character;
            std::uint8_t precision;
            bool use_separator_character;
        };
        
    }
    
    template <>
    struct Formatter<float> : detail::FloatingPointFormatter<float> {
    };

    template <>
    struct Formatter<double> : detail::FloatingPointFormatter<double> {
    };
    
    template <>
    struct Formatter<long double> : detail::FloatingPointFormatter<long double> {
    };
    
    // String types
    
    template <>
    class Formatter<const char*> {
        public:
            Formatter();
            ~Formatter();
            
            void parse(const FormatSpec& spec);
            std::string format(const char* value) const;
            
            Justification justification;
            std::size_t width;
            char fill_character;
            
        protected:
            inline std::string format(const char* value, std::size_t length) const;
    };
    
    template <>
    struct Formatter<std::string_view> : Formatter<const char*> {
        std::string format(std::string_view value) const;
    };
    
    template <>
    struct Formatter<std::string> : Formatter<const char*> {
        std::string format(const std::string& value) const;
    };
    
    // Pointer types
    
    template <typename T>
    struct Formatter<T*> : detail::IntegerFormatter<std::uintptr_t> {
        void parse(const FormatSpec& spec);
    };

    // Standard types
    
    template <typename T, typename U>
    class Formatter<std::pair<T, U>> {
        public:
            Formatter();
            ~Formatter();
            
            void parse(const FormatSpec& spec);
            
            // Output format: { first, second }
            std::string format(const std::pair<T, U>& value) const;
        
            Justification justification;
            std::size_t width;
            char fill_character;
            
        private:
            std::pair<Formatter<T>, Formatter<U>> m_formatters;
    };
    
    template <typename ...Ts>
    class Formatter<std::tuple<Ts...>> {
        public:
            Formatter();
            ~Formatter();
            
            void parse(const FormatSpec& spec);
            
            // Output format: { first, second, ... }
            std::string format(const std::tuple<Ts...>& value) const;
        
            Justification justification;
            std::size_t width;
            char fill_character;
            
        private:
            std::tuple<Formatter<Ts...>> m_formatters;
    };
    
    template <>
    class Formatter<std::source_location> {
        public:
            Formatter();
            ~Formatter();
            
            void parse(const FormatSpec& spec);
            
            // Output format: filepath:line
            std::string format(const std::source_location& value) const;
            
            Justification justification;
            std::size_t width;
            char fill_character;
            
        private:
            Formatter<std::size_t> m_line_formatter;
            Formatter<const char*> m_file_formatter;
    };
    
    // Standard containers
    
    template <typename T>
    class Formatter<std::vector<T>> {
        public:
            Formatter();
            ~Formatter();
            
            void parse(const FormatSpec& spec);
            
            // Output format: [ value, ... ]
            std::string format(const std::vector<T>& value) const;
            
            Justification justification;
            std::size_t width;
            char fill_character;
            
        private:
            Formatter<T> m_formatter;
    };
    
    template <typename K, typename V, typename H, typename P, typename A>
    class Formatter<std::unordered_map<K, V, H, P, A>> {
        public:
            using T = std::unordered_map<K, V, H, P, A>;
            
            Formatter();
            ~Formatter();
            
            void parse(const FormatSpec& spec);
            
            // Output format: { { key: value }, ... }
            std::string format(const T& value) const;
            
            Justification justification;
            std::size_t width;
            char fill_character;
            
        private:
            Formatter<K> m_key_formatter;
            Formatter<V> m_value_formatter;
    };
    
    // Custom / user-defined types
    
    template <typename T>
    struct Formatter<NamedArgument<T>> : Formatter<T> {
        std::string format(const NamedArgument<T>& value);
    };
    
    template <typename T>
    std::string Formatter<NamedArgument<T>>::format(const NamedArgument<T>& value) {
        return Formatter<T>::format(value.value);
    }
    
}

// Template definitions
#include "utils/detail/string.tpp"


#endif // STRING_HPP



