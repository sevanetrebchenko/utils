
#ifndef FORMATTERS_HPP
#define FORMATTERS_HPP

#include <cstdint> // std::size_t

namespace utils {
    
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
            enum class Representation : std::uint8_t {
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
            std::string format(T value) const;
            
            std::size_t reserve(T value) const;
            void format_to(T value, FormattingContext& context) const;
            
            
            void set_representation(Representation representation);
            Representation get_representation() const;
            
            void set_sign(Sign sign);
            Sign get_sign() const;
            
            void set_justification(Justification justification);
            Justification get_justification() const;
            
            void set_width() const;
            std::size_t get_width() const;
            
            void set_fill_character(char fill);
            char get_fill_character() const;
            
            void set_padding_character(char padding);
            char get_padding_character() const;
            
            void set_separator_character(char padding);
            char get_separator_character() const;
            
            void use_base_prefix(bool use);
            bool use_base_prefix() const;
            
            void set_group_size(std::size_t group_size);
            std::size_t get_group_size() const;

            void set_precision(std::size_t precision);
            std::size_t get_precision() const;
            
        private:
            inline int get_base() const;
            
            inline std::size_t to_base(T value, int base, FormattingContext* context) const;
            inline std::size_t apply_justification(std::size_t capacity, FormattingContext& context) const;
            
            Representation m_representation;
            Sign m_sign;
            Justification m_justification;
            
            std::size_t m_width;
            char m_fill;
            
            // Optional specifiers for alternate representations
            char m_separator;
            char m_padding;
            bool m_use_base_prefix;
            std::uint8_t m_group_size;
            std::size_t m_precision;
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

#endif // FORMATTERS_HPP
