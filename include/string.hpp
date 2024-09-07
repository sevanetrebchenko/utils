
#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

#include "utils/concepts.hpp"

#include <string> // std::string
#include <source_location> // std::source_location
#include <stdexcept> // std::runtime_error
#include <variant> // std::variant
#include <optional> // std::optional

namespace utils {
    
    struct Message {
        template <String T>
        Message(T format, std::source_location source = std::source_location::current());
        
        std::string format;
        std::string source;
    };
    
    class FormatSpec {
        public:
            enum class Type {
                FormattingGroupList = 0,
                SpecifierList
            };
            
            FormatSpec();
            ~FormatSpec();
            
            Type type() const;
            bool empty() const;
            
            // Methods for specifier lists
            
            void set_specifier(std::string_view name, std::string value);
            
            std::string& operator[](std::string_view name);
            std::string_view operator[](std::string_view name) const;

            std::string& get_specifier(std::string_view name);
            std::string_view get_specifier(std::string_view name) const;

            // Note: must be called with types convertible to std::string_view (asserted at compile time)
            template <typename ...Ts>
            bool has_specifier(std::string_view first, Ts... rest) const;

            // Note: must be called with types convertible to std::string_view (asserted at compile time)
            template <typename ...Ts>
            std::string_view one_of(std::string_view first, Ts... rest) const;
            
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
            
            [[nodiscard]] bool has_specifier(std::string_view specifier) const;
            
            // A specification can either be a mapping of key - value pairs (specifier name / value) or a nested specification group
            // Specifiers are stored in a std::vector instead of std::unordered map as the number of formatting specifiers is expected to be relatively small
            std::variant<SpecifierList, FormattingGroupList> m_spec;
            Type m_type;
    };
    
    template <typename T>
    struct NamedArgument {
        NamedArgument(std::string_view name, const T& value);
        ~NamedArgument();
    
        std::string_view name;
        const T& value; // Store as a reference to avoid copying non-trivially copyable types
    };
    
    template <typename ...Ts>
    std::string format(Message message, const Ts&... args);
    
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
        struct IntegerFormatter {
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
            std::uint8_t digits;
        };
        
    }

    // Integer types
    
    // char
    template <>
    struct Formatter<std::int8_t> : detail::IntegerFormatter<std::int8_t> {
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
    struct Formatter<const char*> {
        Formatter();
        ~Formatter();
        
        void parse(const FormatSpec& spec);
        std::string format(const char* value) const;
        
        Justification justification;
        std::size_t width;
        char fill_character;
    };
    
    template <>
    struct Formatter<char*> : Formatter<const char*> {
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
    
}

// Template definitions
#include "string.tpp"


#endif // UTILS_STRING_HPP
