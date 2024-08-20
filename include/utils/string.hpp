
#pragma once

#ifndef STRING_HPP
#define STRING_HPP

#include "utils/concepts.hpp"
#include <vector> // std::vector
#include <string> // std::string
#include <string_view> // std::string_view
#include <source_location> // std::source_location
#include <variant> // std::variant
#include <stdexcept> // std::runtime_error
#include <ostream> // std::ostream

namespace utils {

    // Forward declarations
    template <typename T, typename E>
    class ParseResult;
    
    template <typename E>
    class ParseResponse;
    
    
    
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
    
    // Section: Formatting
    
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
                    
                    struct Specifier {
                        Specifier(std::string name, std::string value = "");
                        ~Specifier();

                        Specifier& operator=(std::string other);
                        
                        [[nodiscard]] bool operator==(const Specifier& other) const;
                        [[nodiscard]] bool operator!=(const Specifier& other) const;
                        
                        std::string name;
                        std::string value;
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
                    
                    void set_specifier(std::string_view specifier, std::string value);
                    
                    Specifier& operator[](std::string_view specifier);
                    const Specifier& operator[](std::string_view specifier) const;

                    Specifier& get_specifier(std::string_view specifier);
                    const Specifier& get_specifier(std::string_view specifier) const;
                    
                    template <String T, String ...Ts>
                    const Specifier& one_of(const T& first, const Ts&... rest) const;
                    
                    template <String ...Ts>
                    bool has_specifier(const Ts&... specifiers) const;
                    
                    // Methods for formatting group lists
                    
                    Specification& operator[](std::size_t index);
                    const Specification& operator[](std::size_t index) const;
                    
                    Specification& get_formatting_group(std::size_t index);
                    const Specification& get_formatting_group(std::size_t index) const;
                    
                    bool has_group(std::size_t index) const;
                    
                private:
                    using SpecifierList = std::vector<Specifier>;
                    using FormattingGroupList = std::vector<Specification*>;
                    
                    // Conversion from specifier list to formatting group list
                    explicit Specification(SpecifierList&& specifiers);
                    
                    [[nodiscard]] bool has_specifier(std::string_view specifier) const;
                    
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
            [[nodiscard]] ParseResponse<FormatString> parse_specification(std::string_view in, Specification& spec, bool nested = false);
            ParseResult<Specification::Specifier, FormatString> parse_specifier(std::string_view in);
            
            void register_placeholder(const Identifier& identifier, const Specification& spec, std::size_t position);

            std::string m_format;
            std::source_location m_source;
            
            std::vector<Identifier> m_identifiers;
            std::vector<Specification> m_specifications;
            
            std::vector<Placeholder> m_placeholders;
    };
    
    std::ostream& operator<<(std::ostream& os, const FormatString& fmt);
    
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
    
    

    // Section: Formatters
    
    template <typename T>
    struct Formatter {
    };
    
    class FormattingContext {
        public:
            FormattingContext(std::size_t size, char* buffer = nullptr) : m_size(size),
                                                                          m_buffer(buffer) {}
            ~FormattingContext() {}

            void clear() {}
            
            char& operator[](std::size_t index) {
                return m_buffer[index];
            }
            
            void insert(std::size_t offset, char* src, std::size_t length = 0u) {}
            void insert(std::size_t offset, char c, std::size_t count = 1u) {}
        
            FormattingContext& substring(std::size_t offset, std::size_t size) {}
            
            const char* data() const;

            std::size_t length() const {
                return m_size;
            }
            
        private:
            std::size_t m_size;

            bool m_owner;
            char* m_buffer;
    };
    
    
    // Shared formatters
    
    template <typename T>
    class IntegerFormatter {
        public:
            IntegerFormatter();
            ~IntegerFormatter();
            
            void parse(const FormatString::Specification& spec);
            std::string format(T value) const;
            
            std::size_t reserve(T value) const;
            void format_to(T value, FormattingContext& context) const;
            
            enum class Representation : std::uint8_t {
                Decimal,
                Binary,
                Hexadecimal
            } representation;
            
            enum class Sign : std::uint8_t {
                NegativeOnly,
                Aligned,
                Both
            } sign;
            
            enum class Justification : std::uint8_t {
                Left,
                Right,
                Center
            } justification;
            
            unsigned width;
            char fill_character;
            
            // Optional specifiers for alternate representations
            char separator_character;
            char padding_character;
            bool use_base_prefix;
            std::uint8_t group_size;
            std::size_t precision;
            
        private:
            inline int get_base() const;
            inline std::size_t format_to(T value, int base, FormattingContext* context) const;
    };
    
    template <typename T>
    class FloatingPointFormatter {
        public:
            FloatingPointFormatter();
            ~FloatingPointFormatter();
            
            void parse(const FormatString::Specification& spec);
            std::string format(T value);
            
            std::size_t reserve(T value) const;
            void format_to(T value, FormattingContext& context) const;

            enum class Representation : std::uint8_t {
                Fixed,
                Scientific,
            } representation;
            
            enum class Sign : std::uint8_t {
                NegativeOnly,
                Aligned,
                Both
            } sign;
            
            enum class Justification : std::uint8_t {
                Left,
                Right,
                Center
            } justification;
            
            unsigned width;
            char fill_character;
            
            std::uint8_t precision;
            char separator_character;
            
        private:
            inline std::size_t format_to(T value, FormattingContext* context) const;
    };
    
    template <typename T>
    class StringFormatter {
        public:
            StringFormatter();
            ~StringFormatter();
            
            void parse(const FormatString::Specification& spec);
            std::string format(const T& value) const;
            
            std::size_t reserve(const T& value) const;
            void format_to(const T& value, FormattingContext& context) const;
        
            enum class Justification : std::uint8_t {
                Left,
                Right,
                Center
            } justification;
            
            unsigned width;
            char fill_character;
            
        private:
            inline std::size_t format_to(const T& value, FormattingContext* context) const;
    };
    

    
    // Character types
    
    template <>
    struct Formatter<char> : StringFormatter<char> {
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
    struct Formatter<char*> : StringFormatter<char*> {
    };
    
    template <>
    struct Formatter<char[]> : StringFormatter<char[]> {
    };
    
    template <>
    struct Formatter<const char*> : StringFormatter<const char*> {
    };
    
    template <>
    struct Formatter<const char[]> : StringFormatter<const char[]> {
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
            void parse(const FormatString::Specification& spec) {}
            std::string format(const T& value) const {
                return "vector";
            }
        
        private:
            // Formatter<T> m_formatter;
    };
    
    // Standard types
    
    template <>
    class Formatter<std::source_location> {
        public:
            void parse(const FormatString::Specification& spec) {}
            std::string format(const std::source_location& value) const {
//                return "";
            }
            
            std::size_t reserve(const std::source_location& value) const {
                // return std::strlen(value.file_name());
                return 0;
            }
            
            void format_to(const std::source_location& value, FormattingContext& context) const {
//                const char* filename = value.file_name();
//                for (std::size_t i = 0u; i < strlen(filename); ++i) {
//                    context[i] = filename[i];
//                }
            }
            
        private:
            Formatter<std::uint64_t> m_line_formatter;
            Formatter<const char*> m_filename_formatter;
    };

    // User-defined / custom types
    
    template <typename T>
    struct Formatter<NamedArgument<T>> : Formatter<T> {
    };
    
}

// Template definitions
#include "utils/detail/string.tpp"

#endif // STRING_HPP
