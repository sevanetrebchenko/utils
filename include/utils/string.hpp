
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
            
            void set_value(T value);
            [[nodiscard]] bool has_custom_value() const;
            
            [[nodiscard]] T operator*() const;
            [[nodiscard]] T get() const;
            
        private:
            std::pair<T, bool> m_value;
    };
    
    // Pattern for specifying custom formatting: ([fill character][justification]) [sign][#][minimum width][,][.precision][type representation]
    
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
    // binary, '0o' for octal, and '0x' for hexadecimal. Providing a base prefix format specifier for a non-integer value throws a FormatError
    // exception.

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
    
    struct Formatting {
        enum class Justification : std::uint8_t {
            Right,
            Left,
            Center
        };
        
        enum class Representation : std::uint8_t {
            Decimal,
            Scientific,
            Percentage,
            Fixed,
            Binary,
            Octal,
            Hexadecimal
        };
        
        enum class Sign : std::uint8_t {
            NegativeOnly,
            Aligned,
            Both,
        };
        
        Formatting();
        ~Formatting();
        [[nodiscard]] bool operator==(const Formatting& other) const;
        
        FormattingSpecifier<Justification> justification;
        FormattingSpecifier<Representation> representation;
        FormattingSpecifier<Sign> sign;
        FormattingSpecifier<char> fill;
        FormattingSpecifier<bool> use_separator;
        FormattingSpecifier<bool> use_base_prefix;
        FormattingSpecifier<std::uint8_t> precision;
        FormattingSpecifier<std::uint32_t> width;
        
        std::shared_ptr<Formatting> nested;
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
    [[nodiscard]] std::string format(const std::string& fmt, const Ts&... args);
}

// Template definitions.
#include "utils/internal/string.tpp"

#endif // UTILS_STRING_HPP
