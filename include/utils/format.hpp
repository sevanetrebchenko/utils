
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
#include <cstring> // strlen

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
                    
                    struct Specifier {
                        Specifier(std::string name, std::string value = "");
                        ~Specifier();
                        
                        bool operator==(const Specifier& other) const;
                        bool operator!=(const Specifier& other) const;
                        
                        std::string name;
                        std::string value;
                    };
                    
                    Specification();
                    ~Specification();
                    
                    bool operator==(const Specification& other) const;
                    bool operator!=(const Specification& other) const;
                    
                    const Specification& operator[](std::size_t index) const;
                    Specification& operator[](std::size_t index);

                    std::string_view operator[](std::string_view key) const;
                    std::string& operator[](std::string_view key);
                    
                    // Used to distinguish the type of data this Specification contains
                    Type type() const;
                    
                    bool has_group(std::size_t index) const;
                    bool has_specifier(std::string_view key) const;

                    // Returns the number of groups or the number of specifiers
                    std::size_t size() const;
                    bool empty() const;
                    
                private:
                    using SpecifierList = std::vector<Specifier>;
                    using FormattingGroupList = std::vector<Specification*>;

                    // Conversion from specifier list to formatting group list
                    Specification(SpecifierList&& specifiers);
                    
                    // A specification can either be a mapping of key - value pairs (specifier name / value) or a nested specification group
                    // std::vector used instead of std::unordered_map as the number of formatting specifiers is expected to be relatively small and a vector
                    // is more cache friendly for the types of query operations the specification is intended to be used for
                    std::variant<SpecifierList, FormattingGroupList> m_spec;
                    Type m_type;
            };
            
            template <String T>
            FormatString(T fmt, std::source_location source = std::source_location::current());
            FormatString(const FormatString& fmt);
            ~FormatString();

            template <typename ...Ts>
            [[nodiscard]] FormatString format(const Ts&... args);
            
            operator std::string() const;
            
            [[nodiscard]] std::source_location source() const {
                return m_source;
            }
            
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
        NamedArgument(std::string name, const T& value);
        ~NamedArgument();
    
        std::string name;
        T value;
    };
    
    template <typename ...Ts>
    FormatString format(FormatString fmt, const Ts&... args);
    
    
    
    struct FormattedError : public std::runtime_error {
        template <typename ...Ts>
        FormattedError(FormatString fmt, const Ts&... args);
    };
    
    
    
    template <typename T>
    struct Formatter { };
    
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
            
        private:
            std::size_t m_size;

            bool m_owner;
            char* m_buffer;
    };
    
    
    // Shared formatters
    
    template <typename T>
    class IntegerFormatter {
        public:
            enum class Representation {
                Decimal,
                Binary,
                Hexadecimal,
                Bitset
            };
            
            enum class Sign : std::uint8_t {
                NegativeOnly,
                Aligned,
                Both,
                None
            };
            
            enum class Justification : std::uint8_t {
                Left,
                Right,
                Center
            };
            
            IntegerFormatter();
            ~IntegerFormatter();
            
            void parse(const FormatString::Specification& spec);
            
            void set_representation(Representation representation);
            void set_sign(Sign sign);
            void set_justification(Justification justification);
            
            std::size_t reserve(T value) const;
            
            // Returns the number of characters written
            std::size_t format_to(T value, FormattingContext& context) const {
                return 0;
            }
            
            std::string format(T value) const;
        
        private:
            inline std::string to_base(T value, int base) const;
            
            Representation m_representation;
            Sign m_sign;
            Justification m_justification;
            
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
            enum class Justification : std::uint8_t {
                Left,
                Right,
                Center
            };
            
            Formatter() {}
            ~Formatter() {}
            
            void parse(const FormatString::Specification& spec) {}
            std::string format(char value) {
                return "";
            }
            
        private:
            Justification m_justification;
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
    struct Formatter<char*> : StringFormatter<char*> {
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
                return "";
            }
            
            std::size_t reserve(const std::source_location& value) const {
                return std::strlen(value.file_name());
            }
            
            void format_to(const std::source_location& value, FormattingContext& context) const {
                const char* filename = value.file_name();
                for (std::size_t i = 0u; i < strlen(filename); ++i) {
                    context[i] = filename[i];
                }
            }
            
        private:
            Formatter<std::uint64_t> m_line_formatter;
            Formatter<const char*> m_filename_formatter;
    };

    // User-defined / custom types
    
    template <typename T>
    class Formatter<NamedArgument<T>> {
        public:
            void parse(const FormatString::Specification& spec) {}
            std::string format(const NamedArgument<T>& value) const {
                return m_formatter.format(value.value);
            }
            
        private:
            Formatter<T> m_formatter;
    };

}

// Template definitions
#include "utils/detail/format.tpp"

#endif // FORMAT_HPP
