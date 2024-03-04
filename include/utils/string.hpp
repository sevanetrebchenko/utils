
#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

#pragma once

#include "utils/result.hpp"
#include <vector> // std::vector
#include <string> // std::string
#include <memory> // std::shared_ptr
#include <stdexcept> // std::runtime_error

namespace utils {

    // Returns a vector containing the result of splitting 'in' by 'delimiter'.
    [[nodiscard]] std::vector<std::string> split(const std::string& in, const std::string& delimiter);
    
    
    // Returns a string of 'components' joined by 'glue'.
    template <typename Container>
    [[nodiscard]] std::string join(const Container& components, const std::string& glue);
    
    // Template specializations for type deductions.
    [[nodiscard]] std::string join(const std::initializer_list<std::string>& components, const std::string& glue);
    
    
    // Trim off all whitespace characters on either side of 'in'.
    [[nodiscard]] std::string trim(const std::string& in);
    
    
    // For formatting named format specifiers.
    // Note: custom types must define either a std::string conversion operator (T::operator std::string(), preferred) or a standalone to_string(const T&) function.

    template <typename T>
    class FormattingSpecifier {
        public:
            FormattingSpecifier(T value); // Value by default.
            ~FormattingSpecifier();
            
            FormattingSpecifier(const FormattingSpecifier<T>& other);
            FormattingSpecifier<T>& operator=(const FormattingSpecifier<T>& other);
            
            [[nodiscard]] bool has_custom_value() const;
            
            // Set format specifier.
            void set(T value);
            FormattingSpecifier<T>& operator=(T value);
            
            // Get format specifier.
            [[nodiscard]] T operator*() const;
            [[nodiscard]] T get() const;
            
        private:
            std::pair<T, bool> m_value;
    };
    
    // Pattern for specifying custom formatting: ([fill character][justification]) [sign][wildcard][minimum width][,][.precision][type representation]
    
    // Section: justification
    // The justification value controls how formatted placeholder values are aligned within the available space. A valid justification format
    // specifier can be accompanied by a fill character. This can be any character (defaults to a whitespace if not specified) and fills the
    // remaining space around the formatted placeholder value. Providing just a fill character without a justification format specifier is not
    // supported.
    // Valid format specifiers for justification:
    //   > : right-justify content to available space
    //   < : left-justify content to available space
    //   ^ : center content to available space
    
    // Section: sign
    // The sign value controls how the positive / negative signs are formatted for numeric values. As such, this format specifier is only
    // applicable to numeric types. Providing a sign format specifier for a type that is not a numeric argument throws a FormatError exception.
    //   - : display minus sign for negative values only
    //     : display minus sign for negative values, insert space before positive values (aligned)
    //   + : display minus sign for negative values, plus sign for positive values
    
    // Section: base prefix
    // The '#' format specifier is applicable only for integer types and forces the inclusion of a base prefix. This works in combination with
    // the type representation to use the respective prefix that corresponds to the type of the placeholder. These prefixes include: '0b' for
    // binary, '0o' for octal, and '0x' for hexadecimal.

    // Section: minimum width
    
    // Section: separator
    
    // Section: precision
    
    // Section: type representation
    //   d : decimal
    //   e : scientific notation
    //   % : percentage
    //   f : fixed
    //   b : binary
    //   o : octal
    //   x : hexadecimal
    
                    // For binary / hexadecimal representations, precision and separator values are optional. The user can specify the group size and
                // separator character for making binary representations more readable. Group size is specified via the precision format specifier.
                // For example, 0001 0111 is the binary representation of 23 with a group size of 4 and a whitespace (' ') as the separator character.
                // Note that the representation was padded with additional leading 0s so that each group contains the same number of characters. The
                // separator character can also be used to separate the representation from the base prefix: 0b 0001 0111. Specifying the group size
                // or separator character without the other is not valid. This feature must be explicitly opted into, and hence any format specifier
                // default values are not applicable.
    
    
    class Formatting {
        public:
            enum class Justification : std::uint8_t {
                Right,
                Left,
                Center
            };
            
            enum class Representation : std::uint8_t {
                Decimal,
                Binary,
                Hexadecimal,
                Scientific,
                Fixed, // Fixed number of decimal places
            };
            
            enum class Sign : std::uint8_t {
                NegativeOnly,
                Aligned,
                Both,
            };
            
            Formatting();
            Formatting(const Formatting& other);
            ~Formatting();
            [[nodiscard]] bool operator==(const Formatting& other) const;
            Formatting& operator=(const Formatting& other);
            
            void set_nested_formatting(Formatting* nested);
            
            // The nested() function returns a default-initialized Formatting object if there is no nested formatting available.
            // This makes formatting custom types easier as it doesn't require the user to provide format specifier strings for each
            // nested type. An example of when this would be useful is when formatting a container of objects of a custom type - by hiding the
            // nested formatting (which may or may not be null, if not provided) behind a proxy, the user can provide a format specifier string
            // for the container and use default formatting for the container elements.
            [[nodiscard]] Formatting nested() const;
            
            FormattingSpecifier<Justification> justification;
            FormattingSpecifier<char> fill;
            FormattingSpecifier<std::uint32_t> width;
            
            FormattingSpecifier<Representation> representation;
            FormattingSpecifier<Sign> sign;
            FormattingSpecifier<char> separator;
            FormattingSpecifier<bool> wildcard;
            FormattingSpecifier<std::uint8_t> precision;
            
        private:
            Formatting* m_nested;
    };
    
    struct FormatError final : public std::runtime_error {
        template <typename ...Ts>
        explicit FormatError(const std::string& fmt, const Ts&... args);
        ~FormatError() override;
    };
    
    template <typename T>
    struct arg {
        arg(std::string name, const T& value);
        ~arg();
    
        std::string name;
        const T& value; // Maintain reference for making dealing with non-trivially copyable types easier.
    };
    
    // Python f-string format
    template <typename ...Ts>
    [[nodiscard]] std::string format(std::string_view fmt, const Ts&... args);
    
    template <typename T>
    [[nodiscard]] std::string to_string(const T& value, const Formatting& formatting = { });
    
}

// Template definitions.
#include "utils/internal/string.tpp"

#endif // UTILS_STRING_HPP
