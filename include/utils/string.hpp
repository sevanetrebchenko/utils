
#pragma once

#ifndef STRING_HPP
#define STRING_HPP

#include "utils/concepts.hpp"
#include "utils/colors.hpp"

#include <string> // std::string
#include <source_location> // std::source_location
#include <stdexcept> // std::runtime_error
#include <variant> // std::variant
#include <optional> // std::optional
#include <filesystem> // std::filesystem::path

namespace utils {
    
    // Returns a vector containing the result of splitting 'in' by 'delimiter'.
    [[nodiscard]] std::vector<std::string_view> split(std::string_view in, std::string_view delimiter);

    // Trim off all whitespace characters on either side of 'in'.
    [[nodiscard]] std::string_view trim(std::string_view in);

    // std::strcasecmp requires null-terminated strings (does not work for std::string_view)
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
            
            // Provides read-only access to a specifier
            struct SpecifierView {
                std::string_view name;
                std::string_view value;
            };
            
            FormatSpec();
            FormatSpec(const FormatSpec& other);
            FormatSpec(FormatSpec&& other) noexcept;
            FormatSpec& operator=(const FormatSpec& other);
            ~FormatSpec();
            
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
            SpecifierView get_specifier(std::string_view first, std::string_view second, Ts... rest) const;
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
            
            // A spec can either be a mapping of key - value pairs (specifier name / value) or a nested specification group
            // Specifiers are stored in a std::vector instead of std::unordered map as the number of formatting specifiers is expected to be relatively small
            // The spec starts out as std::monostate to avoid unnecessary allocations
            std::variant<std::monostate, SpecifierList, FormattingGroupList> m_spec;
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
    
    template <typename T>
    struct is_named_argument : std::false_type {
    };

    template <typename T>
    struct is_named_argument<NamedArgument<T>> : std::true_type {
    };

    struct FormatString {
        // Purposefully not marked as explicit
        FormatString(std::string_view format, std::source_location source = std::source_location::current());
        FormatString(const char* format, std::source_location source = std::source_location::current());
        ~FormatString();
        
        std::string_view format;
        std::source_location source;
    };
    
    // Function throws exception if a placeholder does not have an argument specified
    template <typename ...Ts>
    std::string format(const FormatString& str, const Ts&... args);
    
    // Section: Formatters
    
    struct FormatterBase {
        FormatterBase();
        ~FormatterBase();
        
        void parse(const FormatSpec& spec);
        
        // Applies justification and color
        std::string format(char value) const;
        std::string format(std::string value) const;
        
        enum class Justification {
            Left = 0,
            Right,
            Center
        } justification;
        
        std::size_t width;
        char fill_character;
        
        enum class Style {
            None = 0,
            Bold = 1,
            Italicized = 3
        } style;
        std::optional<Color> color;
    };
    
    template <typename T>
    struct Formatter : public FormatterBase {
    };
    
    // Integer types
    
    template <typename T>
    class IntegerFormatter : public FormatterBase {
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
            
            enum class Sign {
                NegativeOnly = 0,
                Aligned,
                Both
            } sign;
            
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
    
    // char
    template <>
    struct Formatter<char> : public FormatterBase {
        Formatter();
        ~Formatter();
        
        void parse(const FormatSpec& spec);
        std::string format(char c) const;
    };
    
    // signed char
    template <>
    struct Formatter<std::int8_t> : public IntegerFormatter<std::int8_t> {
    };
    
    // unsigned char
    template <>
    struct Formatter<std::uint8_t> : public IntegerFormatter<std::uint8_t> {
    };
    
    // short
    template <>
    struct Formatter<std::int16_t> : public IntegerFormatter<std::int16_t> {
    };
    
    // unsigned short
    template <>
    struct Formatter<std::uint16_t> : public IntegerFormatter<std::uint16_t> {
    };
    
    // int
    template <>
    struct Formatter<std::int32_t> : public IntegerFormatter<std::int32_t> {
    };
    
    // unsigned int
    template <>
    struct Formatter<std::uint32_t> : public IntegerFormatter<std::uint32_t> {
    };
    
    // long
    template <>
    struct Formatter<long> : public IntegerFormatter<long> {
    };
    
    // unsigned long
    template <>
    struct Formatter<unsigned long> : public IntegerFormatter<unsigned long> {
    };
    
    // long long
    template <>
    struct Formatter<std::int64_t> : public IntegerFormatter<std::int64_t> {
    };
    
    // unsigned long long
    template <>
    struct Formatter<std::uint64_t> : public IntegerFormatter<std::uint64_t> {
    };
    
    // Floating point types
    
    template <typename T>
    struct FloatingPointFormatter : public FormatterBase {
        FloatingPointFormatter();
        ~FloatingPointFormatter();
        
        void parse(const FormatSpec& spec);
        std::string format(T value) const;
        
        enum class Representation {
            Fixed = 0,
            Scientific
        } representation;
        
        enum class Sign {
            NegativeOnly = 0,
            Aligned,
            Both
        } sign;
        std::uint8_t precision;
        bool use_separator_character;
    };
    
    // float
    template <>
    struct Formatter<float> : public FloatingPointFormatter<float> {
    };

    // double
    template <>
    struct Formatter<double> : public FloatingPointFormatter<double> {
    };
    
    // long double
    template <>
    struct Formatter<long double> : public FloatingPointFormatter<long double> {
    };
    
    // String types
    
    // const char*
    template <>
    class Formatter<const char*> : public FormatterBase {
        public:
            Formatter();
            ~Formatter();
            
            void parse(const FormatSpec& spec);
            std::string format(const char* value) const;
            
        protected:
            inline std::string format(const char* value, std::size_t length) const;
    };
    
    // std::string_view
    template <>
    struct Formatter<std::string_view> : public Formatter<const char*> {
        std::string format(std::string_view value) const;
    };
    
    // std::string
    template <>
    struct Formatter<std::string> : public Formatter<const char*> {
        std::string format(const std::string& value) const;
    };
    
    // Pointer types
    
    // void*
    template <>
    struct Formatter<void*> : public IntegerFormatter<std::uintptr_t> {
        Formatter();
        ~Formatter();
        
        void parse(const FormatSpec& spec);
        std::string format(void* value) const;
    };
    
    // T*
    template <typename T>
    struct Formatter<T*> : public Formatter<void*> {
    };
    
    // std::nullptr_t
    template <>
    struct Formatter<std::nullptr_t> : public Formatter<void*> {
    };

    // Standard types
    
    // std::pair
    template <typename T, typename U>
    class Formatter<std::pair<T, U>> : public FormatterBase {
        public:
            Formatter();
            ~Formatter();
            
            // Format: { first, second }
            void parse(const FormatSpec& spec);
            std::string format(const std::pair<T, U>& value) const;
            
        private:
            std::pair<Formatter<T>, Formatter<U>> m_formatters;
    };
    
    // std::tuple
    template <typename ...Ts>
    class Formatter<std::tuple<Ts...>> : public FormatterBase {
        public:
            Formatter();
            ~Formatter();
            
            // Format: { first, second, ... }
            void parse(const FormatSpec& spec);
            std::string format(const std::tuple<Ts...>& value) const;
            
        private:
            std::tuple<Formatter<Ts>...> m_formatters;
    };
    
    // std::source_location
    template <>
    class Formatter<std::source_location> : public FormatterBase {
        public:
            Formatter();
            ~Formatter();
            
            // Format: file:line
            void parse(const FormatSpec& spec);
            std::string format(const std::source_location& value) const;
            
        private:
            Formatter<std::size_t> m_line_formatter;
            Formatter<const char*> m_file_formatter;
    };
    
    // std::filesystem::path
    template <>
    struct Formatter<std::filesystem::path> : public Formatter<std::string> {
    };
    
    // Standard containers
    
    // std::vector
    template <typename T>
    class Formatter<std::vector<T>> : public FormatterBase {
        public:
            Formatter();
            ~Formatter();
            
            // Format: [ 1, 2, 3, ... ]
            void parse(const FormatSpec& spec);
            std::string format(const std::vector<T>& value) const;
            
        private:
            Formatter<T> m_formatter;
    };
    
    // std::unordered_map
    template <typename K, typename V, typename H, typename P, typename A>
    class Formatter<std::unordered_map<K, V, H, P, A>> : public FormatterBase {
        public:
            using T = std::unordered_map<K, V, H, P, A>;
            
            Formatter();
            ~Formatter();
            
            // Format: { { key: value }, ... }
            void parse(const FormatSpec& spec);
            std::string format(const T& value) const;
            
        private:
            Formatter<K> m_key_formatter;
            Formatter<V> m_value_formatter;
    };
    
    // Custom / user-defined types
    
    template <typename T>
    struct Formatter<NamedArgument<T>> : public Formatter<T> {
        std::string format(const NamedArgument<T>& value);
    };
    
}

// Template definitions
#include "utils/detail/string.tpp"


#endif // STRING_HPP



