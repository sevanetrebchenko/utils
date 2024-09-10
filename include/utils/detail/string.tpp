
//
//    // Default element formatting: { key: value }
//    template <typename K, typename V, typename H, typename P, typename A>
//    FormatString Formatter<std::unordered_map<K, V, H, P, A>>::m_default_format = "{{ {key}: {value} }}";
//
//    template <typename K, typename V, typename H, typename P, typename A>
//    Formatter<std::unordered_map<K, V, H, P, A>>::Formatter() : m_formatter() {
//    }
//
//    template <typename K, typename V, typename H, typename P, typename A>
//    Formatter<std::unordered_map<K, V, H, P, A>>::~Formatter() = default;
//
//    template <typename K, typename V, typename H, typename P, typename A>
//    void Formatter<std::unordered_map<K, V, H, P, A>>::push_format(FormatString fmt) {
//        // Remap placeholder names from std::unordered_map naming convention to std::pair naming convention
//        fmt.rename_placeholder("key", "first");
//        fmt.rename_placeholder("value", "second");
//        m_formatter.set_format(fmt);
//    }
//
//    template <typename K, typename V, typename H, typename P, typename A>
//    void Formatter<std::unordered_map<K, V, H, P, A>>::pop_format() {
//    }
//
//    template <typename K, typename V, typename H, typename P, typename A>
//    void Formatter<std::unordered_map<K, V, H, P, A>>::parse(const FormatString::Specification& spec) {
//        if (spec.empty()) {
//            return;
//        }
//
//        if (spec.type() == FormatString::Specification::Type::SpecifierList) {
//        }
//        else {
//            // Global formatting group is applied to the container
//            if (spec.has_group(0)) {
//                const FormatString::Specification& global = spec.get_formatting_group(0);
//            }
//
//            if (spec.has_group(1)) {
//                m_formatter.parse(spec.get_formatting_group(1));
//            }
//        }
//    }
//
//    template <typename K, typename V, typename H, typename P, typename A>
//    std::string Formatter<std::unordered_map<K, V, H, P, A>>::format(const T& value) const {
//        if (value.empty()) {
//            return "{ }";
//        }
//
//        // Format: { { key: value }, { key: value }, { key: value }, { key: value } }
//
//        std::size_t num_elements = value.size();
//
//        // Format std::unordered_map elements
//        std::vector<std::string> elements;
//        elements.reserve(num_elements);
//        for (auto iter = std::begin(value); iter != std::end(value); ++iter) {
//            elements.emplace_back(m_formatter.format(*iter));
//        }
//
//        // 2 characters for container opening / closing braces { }
//        // 2 characters for leading space before the first element and trailing space after the last element
//        // 2 characters for comma + space (between two elements)
//        std::size_t length = 4u + (num_elements - 1u) * 2u;
//
//        for (const std::string& element : elements) {
//            length += element.length();
//        }
//
//        std::size_t capacity = std::max(length, width);
//        std::string result(capacity, fill_character);
//
//        // Apply justification
//        std::size_t write_position;
//        switch (justification) {
//            case Justification::Left:
//                write_position = 0u;
//                break;
//            case Justification::Right:
//                write_position = capacity - length;
//                break;
//            case Justification::Center:
//                write_position = (capacity - length) / 2;
//                break;
//        }
//
//        result[write_position++] = '{';
//        result[write_position++] = ' ';
//
//        for (const std::string& element : elements) {
//            length = element.length();
//            result.replace(write_position, length, element);
//            write_position += length;
//
//            result[write_position++] = ',';
//            result[write_position++] = ' ';
//        }
//
//        // Overwrite the last two characters to remove the trailing comma
//        result[capacity - 2u] = ' ';
//        result[capacity - 1u] = '}';
//
//        return std::move(result);
//    }
//
//    template <typename K, typename V, typename H, typename P, typename A>
//    void Formatter<std::unordered_map<K, V, H, P, A>>::format_to(const T& value, FormattingContext context) const {
//        if (value.empty()) {
//            context.insert(0, "{ }", 3);
//        }
//        else {
//            std::size_t offset = 0u;
//
//            // Opening brace
//            context[offset++] = '{';
//            context[offset++] = ' ';
//
//            for (auto iter = std::begin(value); iter != std::end(value); ++iter) {
//                std::size_t length = m_formatter.reserve(*iter);
//
//                FormattingContext slice { length, &context[offset] };
//                m_formatter.format_to(*iter, slice);
//
//                offset += length;
//
//                context[offset++] = ',';
//                context[offset++] = ' ';
//            }
//
//            // Overwrite the last two characters to avoid having a trailing comma
//            context[offset - 2u] = ' ';
//            context[offset - 1u] = '}';
//        }
//    }
//
//    template <typename T, typename U>
//    Formatter<std::pair<T, U>>::Formatter() : m_formatters(),
//                                              m_formats(1) {
//        static_assert(is_formattable<T> || is_formattable_to<T>, "std::pair<T, U>: T must (at least) provide implementations for parse() and format()");
//        static_assert(is_formattable<U> || is_formattable_to<U>, "std::pair<T, U>: U must (at least) provide implementations for parse() and format()");
//
//        if constexpr (!is_formattable_to<T>) {
//            // TODO: performance log message
//        }
//        if constexpr (!is_formattable_to<T>) {
//            // TODO: performance log message
//        }
//    }
//
//    template <typename T, typename U>
//    Formatter<std::pair<T, U>>::~Formatter() = default;
//
//    template <typename T, typename U>
//    void Formatter<std::pair<T, U>>::parse(const FormatString::Specification& spec) {
//
//        if (spec.empty()) {
//            return;
//        }
//
//        FormatString::Specification::Type type = spec.type();
//
//        if (type == FormatString::Specification::Type::SpecifierList) {
//            // Specifier list provided, apply specifiers globally
//        }
//        else {
//            m_format.update_placeholder("first", [](FormatString::Identifier& identifier, FormatString::Specification& spec, std::size_t position) {
//                spec +=
//            });
//
//            if (spec.has_group(0)) {
//                m_formatters.first.parse(spec.get_formatting_group(0));
//            }
//
//            if (spec.has_group(1)) {
//                m_formatters.first.parse(spec.get_formatting_group(1));
//            }
//
//            // TODO: additional groups are not used
//        }
//    }
//
//    template <typename T, typename U>
//    std::string Formatter<std::pair<T, U>>::format(const std::pair<T, U>& value) const {
//        FormatString fmt = m_format.empty() ? m_format : m_default;
//        std::string formatted = fmt.format(NamedArgument<T>("first", value.first), NamedArgument<T>("second", value.second));
//
//        // Apply justification, width, etc
//
//        return std::move(formatted);
//    }
//
//    template <typename T>
//    Formatter<NamedArgument<T>>::Formatter() : Formatter<T>() {
//    }
//
//    template <typename T>
//    Formatter<NamedArgument<T>>::~Formatter() = default;
//
//    template <typename T>
//    void Formatter<NamedArgument<T>>::parse(const FormatString::Specification& spec) {
//        Formatter<T>::parse(spec);
//    }
//
//    template <typename T>
//    std::string Formatter<NamedArgument<T>>::format(const NamedArgument<T>& value) const requires is_formattable<T> {
//        return Formatter<T>::format(value.value);
//    }
//
//    template <typename T>
//    std::size_t Formatter<NamedArgument<T>>::reserve(const NamedArgument<T>& value) const requires is_formattable_to<T> {
//        return Formatter<T>::reserve(value.value);
//    }
//
//    template <typename T>
//    void Formatter<NamedArgument<T>>::format_to(const NamedArgument<T>& value, FormattingContext context) const requires is_formattable_to<T> {
//        return Formatter<T>::format_to(value.value, context);
//    }
//
//}
//
//#endif

#pragma once

#ifndef STRING_TPP
#define STRING_TPP

#include "utils/result.hpp"
#include "utils/logging.hpp"
#include "utils/assert.hpp"
#include "utils/tuple.hpp"

#include <charconv> // std::to_chars

namespace utils {

    namespace detail {

        struct Identifier {
            Identifier();
            Identifier(std::size_t position);
            Identifier(std::string_view name);
            ~Identifier();
            
            bool operator==(const Identifier& other) const;
            
            enum class Type {
                Auto = 0,
                Position,
                Name,
            } type;
            
            // This layout has the same size as std::variant<std::size_t, std::string>
            // Prefer this way so that accessing the underlying identifier is easier
            std::size_t position;
            std::string_view name;
        };
        
        struct Specifier {
            std::string_view name;
            std::string_view value;
        };
        
        char nibble_to_hexadecimal(const char* nibble);
        
        // Returns the number of characters read
        std::size_t parse_identifier(std::string_view in, Identifier& out);
        
        // Returns the index of the first invalid character
        std::size_t parse_format_spec(std::string_view in, FormatSpec& out, bool nested = false);
        
        // Performs various validation checks on function arguments
        template <typename Tuple>
        void validate_arguments(const Tuple& tuple, bool is_auto_numbered) {
            if (is_auto_numbered) {
                // Check: argument list must not contain any NamedArgument<T> types
                utils::apply([]<typename T, std::size_t I>(const T& value) {
                    if constexpr (is_named_argument<T>::value) {
                        throw std::runtime_error(utils::format("invalid argument at position {} - named arguments are not allowed in format strings that only contain auto-numbered placeholders", I));
                    }
                }, tuple);
            }
            else {
                // Format string contains a mix of positional and named placeholders
                std::size_t num_positional_arguments = 0u;
                constexpr std::size_t num_arguments = std::tuple_size<Tuple>::value;
        
                // Check: arguments for positional placeholders must come before any arguments for named placeholders
                utils::apply([&num_positional_arguments, positional_arguments_parsed = false]<typename T>(const T&, std::size_t index) mutable {
                    if constexpr (is_named_argument<T>::value) {
                        if (!positional_arguments_parsed) {
                            positional_arguments_parsed = true;
                        }
                    }
                    else {
                        if (positional_arguments_parsed) {
                            // Encountered positional argument after named argument cutoff
                            throw std::runtime_error(utils::format("invalid argument at position {} - arguments for positional placeholders must come before arguments for named placeholders", index));
                        }
        
                        ++num_positional_arguments;
                    }
                }, tuple);
                
                // Check: two NamedArgument<T> arguments should not reference the same named placeholder
                utils::apply_for([&tuple, num_arguments]<typename T>(const T& outer, std::size_t i) {
                    ASSERT(is_named_argument<T>::value, "argument is not of type NamedArgument<T>");
        
                    if constexpr (is_named_argument<T>::value) {
                        utils::apply_for([&outer, i]<typename U>(const U& inner, std::size_t j) {
                            ASSERT(is_named_argument<U>::value, "argument is not of type NamedArgument<U>");
        
                            if constexpr (is_named_argument<U>::value) {
                                if (outer.name == inner.name) {
                                    throw std::runtime_error(utils::format("invalid argument at position {} - named arguments must be unique (argument for placeholder '{}' first encountered at argument position {})", j, inner.name, i));
                                }
                            }
                        }, tuple, i + 1u, num_arguments);
                    }
                }, tuple, num_positional_arguments, num_arguments);
            }
        }
        
        template <typename T>
        IntegerFormatter<T>::IntegerFormatter() : representation(Representation::Decimal),
                                                  sign(Sign::NegativeOnly),
                                                  justification(Justification::Left),
                                                  width(0u),
                                                  fill_character(' '),
                                                  use_separator_character(),
                                                  group_size(),
                                                  use_base_prefix(false),
                                                  digits() {
            static_assert(is_integer_type<T>::value, "value must be an integer type");
        }
    
        template <typename T>
        IntegerFormatter<T>::~IntegerFormatter() = default;
    
        template <typename T>
        void IntegerFormatter<T>::parse(const FormatSpec& spec) {
            ASSERT(spec.type() == FormatSpec::Type::SpecifierList, "format specification for integer values must be a list of specifiers");
    
            if (spec.has_specifier("representation")) {
                std::string_view value = trim(spec.get_specifier("representation"));
                if (icasecmp(value, "decimal")) {
                    representation = Representation::Decimal;
                }
                else if (icasecmp(value, "binary")) {
                    representation = Representation::Binary;
                }
                else if (icasecmp(value, "hexadecimal")) {
                    representation = Representation::Hexadecimal;
                }
                else {
                    logging::warning("ignoring unknown representation specifier value: '{}' - expecting one of: decimal, binary, or hexadecimal (case-insensitive)", value);
                }
            }
    
            if (spec.has_specifier("sign")) {
                std::string_view value = trim(spec.get_specifier("sign"));
                if (icasecmp(value, "negative only") || icasecmp(value, "negative_only") || icasecmp(value, "negativeonly")) {
                    sign = Sign::NegativeOnly;
                }
                else if (icasecmp(value, "aligned")) {
                    sign = Sign::Aligned;
                }
                else if (icasecmp(value, "both")) {
                    sign = Sign::Both;
                }
                else {
                    logging::warning("ignoring unknown sign specifier value: '{}' - expecting one of: negative only (variants: negative_only, negativeonly), aligned, or both (case-insensitive)", value);
                }
            }
            
            if (spec.has_specifier("justification", "justify", "alignment", "align")) {
                std::string_view value = trim(spec.get_specifier("justification", "justify", "alignment", "align"));
                if (icasecmp(value, "left")) {
                    justification = Justification::Left;
                }
                else if (icasecmp(value, "right")) {
                    justification = Justification::Right;
                }
                else if (icasecmp(value, "center")) {
                    justification = Justification::Center;
                }
                else {
                    logging::warning("ignoring unknown justification specifier value: '{}' - expecting one of: left, right, or center (case-insensitive)", value);
                }
            }
    
            if (spec.has_specifier("width")) {
                std::string_view value = trim(spec.get_specifier("width"));
    
                unsigned _width;
                std::size_t num_characters_read = from_string(value, _width);
    
                if (num_characters_read < value.length()) {
                    logging::warning("ignoring invalid width specifier value: '{}' - specifier value must be an integer", value);
                }
                else {
                    width = _width;
                }
            }
    
            if (spec.has_specifier("fill", "fill_character", "fillcharacter")) {
                std::string_view value = trim(spec.get_specifier("fill", "fill_character", "fillcharacter"));
                if (value.length() > 1u) {
                    logging::warning("ignoring invalid fill character specifier value: '{}' - specifier value must be a single character", value);
                }
                else {
                    fill_character = value[0];
                }
            }
    
            if (spec.has_specifier("use_separator", "useseparator", "use_separator_character", "useseparatorcharacter")) {
                std::string_view value = trim(spec.get_specifier("use_separator", "useseparator", "use_separator_character", "useseparatorcharacter"));
                if (icasecmp(value, "true") || icasecmp(value, "1")) {
                    use_separator_character = true;
                }
                else if (icasecmp(value, "false") || icasecmp(value, "0")) {
                    use_separator_character = false;
                }
                else {
                    logging::warning("ignoring unknown use_separator_character specifier value: '{}' - expecting one of: true / 1, false / 0 (case-insensitive)", value);
                }
            }
    
            if (spec.has_specifier("group_size", "groupsize")) {
                std::string_view value = trim(spec.get_specifier("group_size", "groupsize"));
    
                unsigned _group_size;
                std::size_t num_characters_read = from_string(value, _group_size);
    
                if (num_characters_read < value.length()) {
                    logging::warning("ignoring invalid group_size specifier value: '{}' - specifier value must be an integer", value);
                }
                else {
                    group_size = _group_size;
                }
            }
    
            if (spec.has_specifier("use_base_prefix", "usebaseprefix")) {
                std::string_view value = trim(spec.get_specifier("use_base_prefix", "usebaseprefix"));
                if (icasecmp(value, "true") || icasecmp(value, "1")) {
                    use_base_prefix = true;
                }
                else if (icasecmp(value, "false") || icasecmp(value, "0")) {
                    use_base_prefix = false;
                }
                else {
                    logging::warning("ignoring unknown use_base_prefix specifier value: '{}' - expecting one of: true / 1, false / 0 (case-insensitive)", value);
                }
            }
    
            if (spec.has_specifier("digits")) {
                std::string_view value = trim(spec.get_specifier("digits"));
    
                unsigned _digits;
                std::size_t num_characters_read = from_string(value, _digits);
    
                if (num_characters_read < value.length()) {
                    logging::warning("ignoring invalid digits specifier value: '{}' - specifier value must be an integer", value);
                }
                else {
                    digits = _digits;
                }
            }
        }
    
        template <typename T>
        std::string IntegerFormatter<T>::format(T value) const {
            if (representation == Representation::Decimal) {
                return to_decimal(value);
            }
            else if (representation == Representation::Binary) {
                return to_binary(value);
            }
            else {
                ASSERT(representation == Representation::Hexadecimal, "unknown representation");
                return to_hexadecimal(value);
            }
        }
    
        template <typename T>
        std::string IntegerFormatter<T>::to_decimal(T value) const {
            // Compute the minimum number of characters to hold the formatted value
            std::size_t num_characters;
    
            // +1 character for sign
            
            if constexpr (std::is_signed<T>::value) {
                if (value < 0) {
                    num_characters = (std::size_t) (std::log10(-value) + 1u) + 1u;
                }
                else {
                    num_characters = (std::size_t) (std::log10(value) + 1u) + (sign != Sign::NegativeOnly);
                }
            }
            else {
                num_characters = (std::size_t) (std::log10(value) + 1u) + (sign != Sign::NegativeOnly);
            }
    
            bool _use_separator_character = use_separator_character && *use_separator_character;
            std::uint8_t _group_size = 3u;
    
            // Reserve capacity for separator characters (inserted between two groups)
            std::size_t num_separator_characters = 0u;
            if (_use_separator_character) {
                num_separator_characters = num_characters / _group_size;
    
                // Do not include a leading separator character if the number of characters is an even multiple of the group size
                if (num_characters && num_characters % _group_size == 0) {
                    num_separator_characters -= 1u;
                }
            }
    
            std::size_t length = num_characters + num_separator_characters;
            std::size_t capacity = std::max(length, (std::size_t) width);
            std::string result(capacity, fill_character);
    
            std::size_t write_position;
            switch (justification) {
                case Justification::Left:
                    write_position = 0u;
                    break;
                case Justification::Right:
                    write_position = capacity - length;
                    break;
                case Justification::Center:
                    write_position = (capacity - length) / 2;
                    break;
            }
    
            char buffer[std::numeric_limits<T>::digits10 + (std::is_signed<T>::value ? 1 : 0)];
            char* start = buffer;
            char* end = buffer + sizeof(buffer) / sizeof(buffer[0]);
            const auto& [ptr, error_code] = std::to_chars(start, end, value, 10);
    
            if (value < 0) {
                result[write_position++] = '-';
                ++start; // Do not read negative sign from buffer
            }
            else {
                switch (sign) {
                    case Sign::Aligned:
                        result[write_position++] = ' ';
                        break;
                    case Sign::Both:
                        result[write_position++] = '+';
                        break;
                    case Sign::NegativeOnly:
                    default:
                        break;
                }
            }
    
            std::size_t num_characters_written = ptr - start;
    
            if (_use_separator_character) {
                std::size_t current = 3 - (num_characters_written % 3);
                for (std::size_t i = 0u; i < num_characters_written; ++i, ++current) {
                    if (i && (current % 3) == 0u) {
                        result[write_position++] = ',';
                    }
    
                    result[write_position++] = *(start + i);
                }
            }
            else {
                // Copy contents directly
                result.replace(write_position, num_characters_written, start, 0, num_characters_written);
            }
            
            return std::move(result);
        }
    
        template <typename T>
        std::string IntegerFormatter<T>::to_binary(T value) const {
            std::size_t num_characters;
    
            // Compute the minimum number of characters to hold the formatted value
            if (value < 0) {
                // Twos complement is used for formatting negative values, which by default uses as many digits as required by the system architecture
                num_characters = sizeof(T) * CHAR_BIT;
            }
            else {
                // The minimum number of digits required to format a binary number is log2(n) + 1
                num_characters = (std::size_t) std::floor(std::log2(value)) + 1u;
            }
    
            std::size_t num_padding_characters = 0u;
    
            // The number of characters can be overridden by a user-specified 'digits' value
            // If the desired number of digits is smaller than the required number of digits, remove digits starting from the front (most significant) bits
            // If the desired number of digits is larger than the required number of digits, append digits to the front (1 for negative integers, 0 for positive integers)
            if (digits) {
                std::uint8_t _digits = *digits;
                if (num_characters >= _digits) {
                    num_characters = _digits;
                }
                else {
                    // Append leading padding characters to reach the desired number of digits
                    num_padding_characters = _digits - num_characters;
                }
            }
    
            bool _use_separator_character = false;
            std::uint8_t _group_size = 0u;
    
            if (use_separator_character) {
                if (*use_separator_character) {
                    if (group_size) {
                        _group_size = *group_size;
    
                        if (*group_size) {
                            _use_separator_character = true;
                        }
                        else {
                            // Group size explicitly provided as 0, use of separator character is disabled
                            _use_separator_character = false;
                        }
                    }
                    else {
                        // Group size is 4 by default (if not specified)
                        _group_size = 4u;
                        _use_separator_character = true;
                    }
                }
                else {
                    // Use of separator character explicitly disabled
                    _use_separator_character = false;
                }
            }
            else {
                // Use of separator character is disabled by default
                _use_separator_character = false;
            }
    
            // Reserve capacity for separator characters (inserted between two groups)
            std::size_t num_separator_characters = 0u;
            if (_use_separator_character) {
                num_separator_characters = num_characters / _group_size;
    
                // Do not include a leading separator character if the number of characters is an even multiple of the group size
                // Example: 0b'0000 should be 0b0000
                if (num_characters && num_characters % _group_size == 0) {
                    num_separator_characters -= 1u;
                }
            }
    
            std::size_t length = num_characters + num_padding_characters + num_separator_characters;
            if (use_base_prefix) {
                // +2 characters for base prefix '0b'
                length += 2u;
            }
    
            std::size_t capacity = std::max(length, (std::size_t) width);
            std::string result(capacity, fill_character);
    
            std::size_t write_position;
            switch (justification) {
                case Justification::Left:
                    write_position = 0u;
                    break;
                case Justification::Right:
                    write_position = capacity - length;
                    break;
                case Justification::Center:
                    write_position = (capacity - length) / 2;
                    break;
            }
    
            // Convert value to binary
            char buffer[sizeof(T) * CHAR_BIT] { 0 };
            char* end = buffer;
            for (int i = 0; i < (int) num_characters; ++i, ++end) {
                int bit = (value >> (num_characters - 1 - i)) & 1;
                *end = (char) (48 + bit); // 48 is the ASCII code for '0'
            }
    
            if (use_base_prefix) {
                result[write_position++] = '0';
                result[write_position++] = 'b';
            }
    
            if (_use_separator_character) {
                std::size_t current = 0u;
    
                for (std::size_t i = 0u; i < num_padding_characters; ++i, ++current) {
                    if (current && (num_characters - current) % _group_size == 0u) {
                        result[write_position++] = '\'';
                    }
    
                    result[write_position++] = value < 0 ? '1' : '0';
                }
    
                for (char* start = buffer; start != end; ++start, ++current) {
                    if (current && (num_characters - current) % _group_size == 0u) {
                        result[write_position++] = '\'';
                    }
    
                    result[write_position++] = *start;
                }
            }
            else {
                for (std::size_t i = 0u; i < num_padding_characters; ++i) {
                    result[write_position++] = value < 0 ? '1' : '0';
                }
    
                // Copy contents directly
                std::size_t num_characters_written = end - buffer;
                result.replace(write_position, num_characters_written, buffer, 0, num_characters_written);
            }
    
            return std::move(result);
        }
    
        template <typename T>
        std::string IntegerFormatter<T>::to_hexadecimal(T value) const {
            std::size_t num_characters;
    
            // Compute the minimum number of characters to hold the formatted value
            if (value < 0) {
                // Twos complement is used for formatting negative values, which by default uses as many digits as required by the system architecture
                num_characters = sizeof(T) * CHAR_BIT / 4;
            }
            else {
                // The minimum number of digits required to format a binary number is log2(n) + 1
                // Each hexadecimal character represents 4 bits
                num_characters = (std::size_t) std::floor(std::log2(value) / 4) + 1u;
            }
    
            std::size_t num_padding_characters = 0u;
    
            // The number of characters can be overridden by a user-specified 'digits' value
            // If the desired number of digits is smaller than the required number of digits, remove digits starting from the front (most significant) bits
            // If the desired number of digits is larger than the required number of digits, append digits to the front (1 for negative integers, 0 for positive integers)
            if (digits) {
                std::uint8_t _digits = *digits;
                if (num_characters >= _digits) {
                    num_characters = _digits;
                }
                else {
                    // Append leading padding characters to reach the desired number of digits
                    num_padding_characters = _digits - num_characters;
                }
            }
    
            bool _use_separator_character = false;
            std::uint8_t _group_size = 0u;
    
            if (use_separator_character) {
                if (*use_separator_character) {
                    if (group_size) {
                        _group_size = *group_size;
    
                        if (*group_size) {
                            _use_separator_character = true;
                        }
                        else {
                            // Group size explicitly provided as 0, use of separator character is disabled
                            _use_separator_character = false;
                        }
                    }
                    else {
                        // Group size is 4 by default (if not specified)
                        _group_size = 4u;
                        _use_separator_character = true;
                    }
                }
                else {
                    // Use of separator character explicitly disabled
                    _use_separator_character = false;
                }
            }
            else {
                // Use of separator character is disabled by default
                _use_separator_character = false;
            }
    
            // Reserve capacity for separator characters (inserted between two groups)
            std::size_t num_separator_characters = 0u;
            if (_use_separator_character) {
                num_separator_characters = (num_characters + num_padding_characters) / _group_size;
    
                // Do not include a leading separator character if the number of characters is an even multiple of the group size
                // Example: 0b'0000 should be 0b0000
                if ((num_characters + num_padding_characters) % _group_size == 0) {
                    num_separator_characters -= 1u;
                }
            }
    
            std::size_t length = num_characters + num_padding_characters + num_separator_characters;
    
            if (use_base_prefix) {
                // +2 characters for base prefix '0x'
                length += 2u;
            }
    
            std::size_t capacity = std::max(length, (std::size_t) width);
            std::string result(capacity, fill_character);
    
            std::size_t write_position;
            switch (justification) {
                case Justification::Left:
                    write_position = 0u;
                    break;
                case Justification::Right:
                    write_position = capacity - length;
                    break;
                case Justification::Center:
                    write_position = (capacity - length) / 2;
                    break;
            }
    
            // Convert value to binary
            std::size_t num_characters_binary = num_characters * 4u;
            char buffer[sizeof(T) * CHAR_BIT] { '0' };
            char* end = buffer;
            for (int i = 0; i < (int) num_characters_binary; ++i, ++end) {
                int bit = (value >> (num_characters_binary - 1 - i)) & 1;
                *end = (char) (48 + bit); // 48 is the ASCII code for '0'
            }
    
            if (use_base_prefix) {
                result[write_position++] = '0';
                result[write_position++] = 'x';
            }
    
            char nibble[4] = { '0' };
    
            // Convert binary representation to hexadecimal
            // Pad first group binary representation if necessary, as this group might not be the same size as the rest
            // Example: 0b01001 has two groups (000)1 and 1001
            std::size_t num_groups = num_characters_binary / 4u;
            std::size_t remainder = num_characters_binary % 4u;
            std::size_t num_padding_characters_binary = 0u;
    
            if (remainder) {
                // Number of binary characters does not equally divide into a hexadecimal representation, first group will require padding characters
                num_groups += 1u;
                num_padding_characters_binary = 4u - remainder;
            }
    
            if (_use_separator_character) {
                std::size_t current = 0u;
    
                // Append any extra padding characters
                for (std::size_t i = 0u; i < num_padding_characters; ++i, ++current) {
                    if (current && (num_characters - current) % _group_size == 0u) {
                        result[write_position++] = '\'';
                    }
    
                    result[write_position++] = value < 0 ? 'F' : '0';
                }
    
                char* ptr = buffer;
                for (std::size_t group = 0; group < num_groups; ++group, ++current) {
                    // Insert separator
                    if (current && (num_characters - current) % _group_size == 0u) {
                        result[write_position++] = '\'';
                    }
    
                    // Convert binary to hexadecimal
                    int i = 0;
    
                    // Append any necessary padding characters to the front of the nibble
                    if (num_padding_characters_binary) {
                        for (; i < num_padding_characters_binary; ++i) {
                            nibble[i] = value < 0 ? '1' : '0';
                        }
                        // Padding is only applicable to the first group
                        num_padding_characters_binary = 0u;
                    }
    
                    while (i != 4) {
                        nibble[i++] = *ptr++;
                    }
                    result[write_position++] = detail::nibble_to_hexadecimal(nibble);
                }
            }
            else {
                for (std::size_t i = 0u; i < num_padding_characters; ++i) {
                    result[write_position++] = value < 0 ? 'F' : '0';
                }
    
                char* ptr = buffer;
                for (std::size_t group = 0; group < num_groups; ++group) {
                    // Convert binary to hexadecimal
                    int i = 0;
    
                    // Append any necessary padding characters to the front of the nibble
                    if (num_padding_characters_binary) {
                        for (; i < num_padding_characters_binary; ++i) {
                            nibble[i] = value < 0 ? '1' : '0';
                        }
                        // Padding is only applicable to the first group
                        num_padding_characters_binary = 0u;
                    }
    
                    while (i != 4) {
                        nibble[i++] = *ptr++;
                    }
                    result[write_position++] = detail::nibble_to_hexadecimal(nibble);
                }
            }
    
            return std::move(result);
        }
    
        template <typename T>
        FloatingPointFormatter<T>::FloatingPointFormatter() : representation(Representation::Fixed),
                                                              sign(Sign::NegativeOnly),
                                                              justification(Justification::Left),
                                                              width(0u),
                                                              fill_character(' '),
                                                              precision(std::numeric_limits<T>::digits10) {
            static_assert(is_floating_point_type<T>::value, "value must be a floating point type");
        }
    
        template <typename T>
        FloatingPointFormatter<T>::~FloatingPointFormatter() = default;
    
        template <typename T>
        void FloatingPointFormatter<T>::parse(const FormatSpec& spec) {
            if (spec.type() == FormatSpec::Type::FormattingGroupList) {
                throw std::runtime_error("format specification for floating point values must be a list of specifiers");
            }
    
            if (spec.has_specifier("representation")) {
                std::string_view value = trim(spec.get_specifier("representation"));
                if (icasecmp(value, "fixed")) {
                    representation = Representation::Fixed;
                }
                else if (icasecmp(value, "scientific")) {
                    representation = Representation::Scientific;
                }
                else {
                    logging::warning("ignoring unknown representation specifier value: '{}' - expecting one of: fixed, scientific (case-insensitive)", value);
                }
            }
    
    
            if (spec.has_specifier("sign")) {
                std::string_view value = trim(spec.get_specifier("sign"));
                if (icasecmp(value, "negative only") || icasecmp(value, "negative_only") || icasecmp(value, "negativeonly")) {
                    sign = Sign::NegativeOnly;
                }
                else if (icasecmp(value, "aligned")) {
                    sign = Sign::Aligned;
                }
                else if (icasecmp(value, "both")) {
                    sign = Sign::Both;
                }
                else {
                    logging::warning("ignoring unknown sign specifier value: '{}' - expecting one of: negative only (variants: negative_only, negativeonly), aligned, or both (case-insensitive)", value);
                }
            }
    
            if (spec.has_specifier("justification", "justify", "alignment", "align")) {
                std::string_view value = trim(spec.get_specifier("justification", "justify", "alignment", "align"));
                if (icasecmp(value, "left")) {
                    justification = Justification::Left;
                }
                else if (icasecmp(value, "right")) {
                    justification = Justification::Right;
                }
                else if (icasecmp(value, "center")) {
                    justification = Justification::Center;
                }
                else {
                    logging::warning("ignoring unknown justification specifier value: '{}' - expecting one of: left, right, or center (case-insensitive)", value);
                }
            }
    
            if (spec.has_specifier("width")) {
                std::string_view value = trim(spec.get_specifier("width"));
    
                unsigned _width;
                std::size_t num_characters_read = from_string(value, _width);
    
                if (num_characters_read < value.length()) {
                    logging::warning("ignoring invalid width specifier value: '{}' - specifier value must be an integer", value);
                }
                else {
                    width = _width;
                }
            }
    
            if (spec.has_specifier("fill", "fill_character", "fillcharacter")) {
                std::string_view value = trim(spec.get_specifier("fill", "fill_character", "fillcharacter"));
                if (value.length() > 1u) {
                    logging::warning("ignoring invalid fill character specifier value: '{}' - specifier value must be a single character", value);
                }
                else {
                    fill_character = value[0];
                }
            }
    
            if (spec.has_specifier("precision")) {
                std::string_view value = trim(spec.get_specifier("precision"));
    
                unsigned _precision;
                std::size_t num_characters_read = from_string(value, _precision);
    
                if (num_characters_read < value.length()) {
                    logging::warning("ignoring invalid precision specifier value: '{}' - specifier value must be an integer", value);
                }
                else {
                    precision = _precision;
                }
            }
    
            if (spec.has_specifier("use_separator", "useseparator", "use_separator_character", "useseparatorcharacter")) {
                std::string_view value = trim(spec.get_specifier("use_separator", "useseparator", "use_separator_character", "useseparatorcharacter"));
                if (icasecmp(value, "true") || icasecmp(value, "1")) {
                    use_separator_character = true;
                }
                else if (icasecmp(value, "false") || icasecmp(value, "0")) {
                    use_separator_character = false;
                }
                else {
                    logging::warning("ignoring unknown use_separator_character specifier value: '{}' - expecting one of: true / 1, false / 0 (case-insensitive)", value);
                }
            }
        }
    
        template <typename T>
        std::string FloatingPointFormatter<T>::format(T value) const {
            std::size_t length = 0u;
            std::size_t read_offset = 0u;
    
            // Sign character
            length += value < 0 ? 1u : unsigned(sign != Sign::NegativeOnly);
    
            int num_significant_figures = std::numeric_limits<T>::digits10 + 1;
            if (precision) {
                num_significant_figures = precision;
            }
    
            std::chars_format format_flags = std::chars_format::fixed;
            if (representation == Representation::Scientific) {
                format_flags = std::chars_format::scientific;
            }
            constexpr int num_significant_digits = std::numeric_limits<T>::max_digits10; // Max number of significant digits
            constexpr int sign_character = 1; // 1 character for sign
            constexpr int decimal_character = 1; // 1 character for the decimal point
            constexpr int exponent_character = 1; // 'e' character
            constexpr int exponent_sign = 1; // '+' or '-' sign for exponent
            constexpr int exponent_digits = std::numeric_limits<T>::max_exponent10; // Max number of digits in the exponent
    
            char buffer[sign_character + num_significant_digits + decimal_character + exponent_character + exponent_sign + exponent_digits];
            char* start = buffer;
            char* end = buffer + sizeof(buffer) / sizeof(buffer[0]);
    
            // std::numeric_limits<T>::digits10 represents the number of decimal places that are guaranteed to be preserved when converted to text
            // Note: last decimal place will be rounded
            int conversion_precision = std::clamp(num_significant_figures, 0, std::numeric_limits<T>::digits10 + 1);
            const auto& [ptr, error_code] = std::to_chars(start, end, value, format_flags, conversion_precision);
    
            std::size_t num_characters_written = ptr - (start + read_offset);
            length += num_characters_written;
    
            // Additional precision
            length += std::max(0, num_significant_figures - conversion_precision);
    
            std::size_t decimal_position = num_characters_written;
            if (use_separator_character) {
                char* decimal = std::find(start + read_offset, ptr, '.');
                decimal_position = decimal - (start + read_offset);
    
                // Separators get inserted every 3 characters up until the position of the decimal point
                length += (decimal_position - 1) / 3;
            }
    
            std::size_t capacity = std::max(length, (std::size_t) width);
            std::string result(capacity, fill_character);
    
            std::size_t write_position;
            switch (justification) {
                case Justification::Left:
                    write_position = 0u;
                    break;
                case Justification::Right:
                    write_position = capacity - length;
                    break;
                case Justification::Center:
                    write_position = (capacity - length) / 2;
                    break;
            }
    
            if (value < 0) {
                result[write_position++] = '-';
            }
            else {
                switch (sign) {
                    case Sign::Aligned:
                        result[write_position++] = ' ';
                        break;
                    case Sign::Both:
                        result[write_position++] = '+';
                        break;
                    case Sign::NegativeOnly:
                    default:
                        break;
                }
            }
    
            if (representation == Representation::Scientific) {
                char* e = std::find(buffer, ptr, 'e');
                std::size_t e_position = e - (start + read_offset);
    
                for (std::size_t i = 0u; i < e_position; ++i) {
                    result[write_position++] = *(buffer + read_offset + i);
                }
    
                // For scientific notation, fake precision must be appended before the 'e' denoting the exponent
                for (std::size_t i = conversion_precision; i < num_significant_figures; ++i) {
                    result[write_position++] = '0';
                }
    
                for (start = buffer + read_offset + e_position; start != ptr; ++start) {
                    result[write_position++] = *start;
                }
            }
            else {
                // Separator character only makes sense for fixed floating point values
                if (use_separator_character) {
                    // Separators get inserted every 3 characters up until the position of the decimal point
                    std::size_t group_size = 3;
                    std::size_t counter = group_size - (decimal_position % group_size);
    
                    // Write the number portion, up until the decimal point (with separators)
                    for (std::size_t i = 0; i < decimal_position; ++i, ++counter) {
                        if (i && counter % group_size == 0u) {
                            result[write_position++] = ',';
                        }
    
                        result[write_position++] = *(buffer + read_offset + i);
                    }
    
                    // Write decimal portion
                    for (start = buffer + read_offset + decimal_position; start != ptr; ++start) {
                        result[write_position++] = *start;
                    }
                }
                else {
                    for (start = buffer + read_offset; start != ptr; ++start) {
                        result[write_position++] = *start;
                    }
                }
    
                // For regular floating point values, fake higher precision by appending the remaining decimal places as 0
                for (std::size_t i = conversion_precision; i < num_significant_figures; ++i) {
                    result[write_position++] = '0';
                }
            }
    
            return std::move(result);
        }
        
        char nibble_to_hexadecimal(const char nibble[4]);
        
    }
    
    // namespace utils
    
    template <String T, String U>
    [[nodiscard]] bool icasecmp(const T& first, const U& second) {
        std::string_view a = first;
        std::string_view b = second;
        
        if (a.length() != b.length()) {
            return false;
        }
        
        for (std::size_t i = 0u; i < a.length(); ++i) {
            if (std::tolower(a[i]) != std::tolower(b[i])) {
                return false;
            }
        }
        
        return true;
    }

    template <String T, String U>
    [[nodiscard]] bool operator==(const T& first, const U& second) {
        std::string_view a = first;
        std::string_view b = second;
        
        if (a.length() != b.length()) {
            return false;
        }
        
        for (std::size_t i = 0u; i < a.length(); ++i) {
            if (a[i] != b[i]) {
                return false;
            }
        }
        
        return true;
    }
    
    template <typename ...Ts>
    std::string_view FormatSpec::get_specifier(std::string_view first, std::string_view second, Ts... rest) const {
        constexpr std::size_t argument_count = sizeof...(Ts) + 2u; // Include 'first' and 'second' specifiers
        std::pair<std::string_view, std::string_view> specifiers[argument_count];

        // Index of the first valid specifier
        std::size_t index = argument_count; // Initially set to invalid
        std::size_t valid_specifier_count = 0u;

        utils::apply([this, &specifiers, &index, &valid_specifier_count](std::string_view name, std::size_t i) {
            specifiers[i].first = name;
            if (has_specifier(name)) {
                index = i;
                specifiers[i].second = get_specifier(name);
                ++valid_specifier_count;
            }
        }, std::make_tuple(first, second, std::string_view(rest)...));

        if (valid_specifier_count == 0u) {
            // No valid specifiers found
            // Error message format: bad format specification access - no values found for any of the following specifiers: ... (length: 87 + comma-separated list of specifiers)
            std::size_t capacity = 87u;
            for (std::size_t i = 0u; i < argument_count; ++i) {
                capacity += specifiers[i].second.length();
                if (i != argument_count) {
                    // Avoid trailing comma ', '
                    capacity += 2u;
                }
            }
            
            std::string error;
            error.reserve(capacity);
            
            error += "bad format specification access - no values found for any of the following specifiers: ";

            for (std::size_t i = 0u; i < argument_count; ++i) {
                error += specifiers[i].first; // Name

                // Do not add a trailing comma
                if (i != argument_count) {
                    error += ", ";
                }
            }

            throw std::runtime_error(error);
        }
        else if (valid_specifier_count > 1u) {
            // Found multiple valid specifiers
            // Error message format: ambiguous format specification access - value found for more than one of the following specifiers: ... (length: 99 + comma-separated list of found specifiers)
            std::size_t capacity = 99u;
            for (std::size_t i = 0u, count = 0u; i < argument_count; ++i) {
                if (!specifiers[i].second.empty()) {
                    capacity += specifiers[i].first.length();
                    if (count != valid_specifier_count) {
                        // Do not add a trailing comma
                        capacity += 2u;
                    }
                    ++count;
                }
            }

            std::string error;
            error.reserve(capacity);
            
            error += "ambiguous format specification access - value found for more than one of the following specifiers: ";

            for (std::size_t i = 0u, count = 0u; i < argument_count; ++i) {
                if (!specifiers[i].second.empty()) {
                    error += specifiers[i].first;
                    if (count != valid_specifier_count) {
                        // Do not add a trailing comma
                        error += ", ";
                    }
                    ++count;
                }
            }
            
            throw std::runtime_error(error);
        }
        
        return specifiers[index].second;
    }
    
    template <typename ...Ts>
    bool FormatSpec::has_specifier(std::string_view first, std::string_view second, Ts... rest) const {
        return has_specifier(first) || has_specifier(second) || (has_specifier(rest) || ...);
    }
    
    template <typename T>
    NamedArgument<T>::NamedArgument(std::string_view name, const T& value) : name(name),
                                                                             value(value) {
    }

    template <typename T>
    NamedArgument<T>::~NamedArgument() {
    }
    
    template <typename ...Ts>
    std::string format(FormatString str, const Ts&... args) {
        std::string_view fmt = str.format;
        const std::source_location& source = str.source;
        
        if (fmt.empty()) {
            return "";
        }
        
        constexpr std::size_t num_arguments = sizeof...(Ts);
        
        std::size_t length = fmt.length();
        std::size_t last_read_position = 0u;
        std::size_t i = 0u;
        
        std::string out;
        
        if constexpr (num_arguments) {
            std::tuple<typename std::decay<const Ts>::type...> tuple = std::make_tuple(args...);
            
            std::optional<detail::Identifier::Type> type { };
            std::size_t argument_index = 0u; // Used only for auto-numbered format strings
            
            while (i < length) {
                if (fmt[i] == '{') {
                    if (i + 1 == length) {
                        throw std::runtime_error(utils::format("unterminated placeholder opening brace at position {} - opening brace literals must be escaped as '}}' ({})", i, source));
                    }
                    else if (fmt[i + 1] == '{') {
                        // Escaped opening brace '{{'
                        out.append(fmt, last_read_position, i - last_read_position + 1u); // Include the first opening brace
                        
                        ++i;
                        last_read_position = i + 1u; // Skip to the next character after the second brace
                    }
                    else {
                        out.append(fmt, last_read_position, i - last_read_position); // Do not include opening brace
                        
                        // Skip placeholder opening brace '{'
                        i++;
                        
                        detail::Identifier identifier { };
                        i += parse_identifier(fmt.substr(i), identifier);
                        if (fmt[i] != ':' && fmt[i] != '}') {
                            // Expecting format spec separator ':' or placeholder closing brace '}'
                            throw std::runtime_error(utils::format("invalid character '{}' at position {} - expecting format spec separator ':' or placeholder closing brace '}' ({})", fmt[i], i, source));
                        }
                        
                        if (!type) {
                            // The identifier of the first placeholder dictates the type of format string
                            type = identifier.type;
                            detail::validate_arguments(tuple, identifier.type == detail::Identifier::Type::Auto);
                        }
                        else {
                            // Verify format string homogeneity - all placeholders must be of the same type
                            bool homogeneous = (*type == detail::Identifier::Type::Auto && identifier.type == detail::Identifier::Type::Auto) || (*type != detail::Identifier::Type::Auto && identifier.type != detail::Identifier::Type::Auto);
                            if (!homogeneous) {
                                throw std::runtime_error(utils::format("invalid format string - placeholder types must be homogeneous ({})", source));
                            }
                        }
                        
                        FormatSpec spec { };
                        if (fmt[i] == ':') {
                            // Skip format spec separator ':'
                            ++i;
                            
                            i += detail::parse_format_spec(fmt.substr(i, length - i), spec);
                            if (fmt[i] != '}') {
                                throw std::runtime_error(utils::format("invalid character '{}' at position {} - expecting placeholder closing brace '}' ({})", fmt[i], i, source));
                            }
                        }
    
                        // Format placeholder
                        switch (*type) {
                            case detail::Identifier::Type::Auto: {
                                // utils::apply is a noop is the argument index exceeds the length of the tuple
                                // An exception for this is raised below (after parsing the whole format string)
                                utils::apply([&spec, &out]<typename T>(const T& value) {
                                    Formatter<T> formatter { };
                                    formatter.parse(spec);
                                    out.append(formatter.format(value));
                                }, tuple, argument_index++);
                                break;
                            }
                            case detail::Identifier::Type::Position: {
                                if (identifier.position >= num_arguments) {
                                    throw std::runtime_error(utils::format("invalid format string - missing argument for placeholder {} at position {} ()", identifier.position, i, source));
                                }
                                else {
                                    utils::apply([&spec, &out]<typename T>(const T& value) {
                                        Formatter<T> formatter { };
                                        formatter.parse(spec);
                                        out.append(formatter.format(value));
                                    }, tuple, identifier.position);
                                }
                                break;
                            }
                            case detail::Identifier::Type::Name: {
                                bool formatted = false;
                                utils::apply([name = identifier.name, &formatted, &spec, &out]<typename T>(const T& arg) {
                                    if constexpr (is_named_argument<T>::value) {
                                        if (arg.name == name) {
                                            Formatter<typename T::type> formatter { };
                                            formatter.parse(spec);
                                            out.append(formatter.format(arg.value));
                                            formatted = true;
                                        }
                                    }
                                }, tuple);
                                
                                if (!formatted) {
                                    throw std::runtime_error(utils::format("invalid format string - missing NamedArgument for placeholder '{}' at position {{ ({})", identifier.name, i, source));
                                }
                                break;
                            }
                        }
                        
                        // Skip placeholder closing brace '}'
                        last_read_position = ++i;
                    }
                }
                else if (fmt[i] == '}') {
                    if (i + 1 < length && fmt[i + 1] == '}') {
                        // Escaped closing brace '}}'
                        out.append(fmt, last_read_position, i - last_read_position + 1u); // Include the first opening brace
                        
                        ++i;
                        last_read_position = i + 1u; // Skip to the next character after the second brace
                    }
                    else {
                        throw std::runtime_error(utils::format("invalid placeholder closing brace at position {} - closing brace literals must be escaped as '}}' ({})", i, source));
                    }
                }
                
                ++i;
            }
            
            if (*type == detail::Identifier::Type::Auto && argument_index > num_arguments) {
                throw std::runtime_error(utils::format("not enough arguments provided to format(...) - expecting: {}, received: {} ({})", argument_index, num_arguments, source));
            }
        }
        else {
            // No arguments provided to format
            while (i < length) {
                if (fmt[i] == '{') {
                    if (i + 1 == length) {
                        throw std::runtime_error(utils::format("unterminated placeholder opening brace at position {} - opening brace literals must be escaped as '}}' ()", i, source));
                    }
                    else if (fmt[i + 1] == '{') {
                        // Escaped opening brace '{{'
                        out.append(fmt, last_read_position, i - last_read_position + 1u); // Include the first opening brace
                        
                        ++i;
                        last_read_position = i + 1u; // Skip to the next character after the second brace
                    }
                    else {
                        // format(...) was called with no arguments, so the existence of any placeholders is invalid
                        throw std::runtime_error(utils::format("invalid format string - missing argument for placeholder at position {} ()", i, source));
                    }
                }
                else if (fmt[i] == '}') {
                    if (i + 1 < length && fmt[i + 1] == '}') {
                        // Escaped closing brace '}}'
                        out.append(fmt, last_read_position, i - last_read_position + 1u); // Include the first opening brace
                        
                        ++i;
                        last_read_position = i + 1u; // Skip to the next character after the second brace
                    }
                    else {
                        throw std::runtime_error(utils::format("invalid placeholder closing brace at position {} - closing brace literals must be escaped as '}}' ()", i, source));
                    }
                }
                
                ++i;
            }
        }
        
        if (i != last_read_position) {
            // Append any remaining characters
            out.append(fmt, last_read_position, i - last_read_position);
        }
        
        return std::move(out);
    }
    
    template <typename K, typename V, typename H, typename P, typename A>
    Formatter<std::unordered_map<K, V, H, P, A>>::Formatter() : justification(Justification::Left),
                                                                width(0u),
                                                                fill_character(' '),
                                                                m_key_formatter(),
                                                                m_value_formatter() {
    }
    
    template <typename K, typename V, typename H, typename P, typename A>
    Formatter<std::unordered_map<K, V, H, P, A>>::~Formatter() = default;

    template <typename K, typename V, typename H, typename P, typename A>
    void Formatter<std::unordered_map<K, V, H, P, A>>::parse(const FormatSpec& spec) {
        if (spec.empty()) {
            return;
        }
        
        if (spec.type() == FormatSpec::Type::SpecifierList) {
            // A format spec consisting of a list of specifiers is applied globally to the unordered map
            parse_specifiers(spec);
        }
        else {
            if (spec.has_group(0)) {
                // The first formatting group is applied globally to the unordered map
                const FormatSpec& group = spec.get_group(0);
                if (group.type() != FormatSpec::Type::SpecifierList) {
                    throw std::runtime_error("invalid std::unordered_map format spec - formatting group 0 must be a specifier list");
                }
                parse_specifiers(group);
            }
            if (spec.has_group(1)) {
                // The second formatting group is applied to the map key type
                m_key_formatter.parse(spec.get_group(1));
            }
            if (spec.has_group(2)) {
                // The third formatting group is applied to the map value type
                m_value_formatter.parse(spec.get_group(2));
            }
        }
    }
    
    template <typename K, typename V, typename H, typename P, typename A>
    std::string Formatter<std::unordered_map<K, V, H, P, A>>::format(const T& value) const {
        if (value.empty()) {
            return "{ }";
        }
        
        std::size_t num_elements = value.size();

        // Format elements
        std::vector<std::pair<std::string, std::string>> elements;
        elements.reserve(num_elements);
        
        for (auto iter = std::begin(value); iter != std::end(value); ++iter) {
            elements.emplace_back(m_key_formatter.format(iter->first), m_value_formatter.format(iter->second));
        }

        // 2 characters for container opening / closing braces { }
        // 2 characters for leading space before the first element and trailing space after the last element
        
        // 2 characters for element opening / closing braces { } (per element)
        // 2 characters for leading space before the element key and trailing space after the element value (per element)
        // 2 characters for comma + space between element key and value (per element)
        // 2 characters for comma + space between two elements (per element - 1)
        std::size_t length = 4 + (num_elements * 6) + (num_elements - 1) * 2;
        for (const std::pair<std::string, std::string>& element : elements) {
            length += element.first.length();
            length += element.second.length();
        }

        std::size_t capacity = std::max(length, width);
        std::string result(capacity, fill_character);

        // Apply justification
        std::size_t write_position;
        switch (justification) {
            case Justification::Left:
                write_position = 0u;
                break;
            case Justification::Right:
                write_position = capacity - length;
                break;
            case Justification::Center:
                write_position = (capacity - length) / 2;
                break;
        }

        result[write_position++] = '{';
        result[write_position++] = ' ';

        for (std::size_t i = 0u; i < num_elements; ++i) {
            // Element format: { key: value }
            const std::pair<std::string, std::string>& element = elements[i];
            
            result[write_position++] = '{';
            result[write_position++] = ' ';
            
            // Key
            length = element.first.length();
            result.replace(write_position, length, element.first, 0, length);
            write_position += length;

            result[write_position++] = ',';
            result[write_position++] = ' ';
            
            // Value
            length = element.second.length();
            result.replace(write_position, length, element.second, 0, length);
            write_position += length;
            
            result[write_position++] = ' ';
            result[write_position++] = '}';
            
            // Elements are formatted into a comma-separated list
            if (i != num_elements - 1u) {
                // Do not insert a trailing comma
                result[write_position++] = ',';
                result[write_position++] = ' ';
            }
        }

        result[write_position++] = ' ';
        result[write_position++] = '}';

        return std::move(result);
    }
    
    template <typename K, typename V, typename H, typename P, typename A>
    void Formatter<std::unordered_map<K, V, H, P, A>>::parse_specifiers(const utils::FormatSpec& spec) {
        if (spec.has_specifier("justification", "justify", "alignment", "align")) {
            std::string_view value = trim(spec.get_specifier("justification", "justify", "alignment", "align"));
            if (icasecmp(value, "left")) {
                justification = Justification::Left;
            }
            else if (icasecmp(value, "right")) {
                justification = Justification::Right;
            }
            else if (icasecmp(value, "center")) {
                justification = Justification::Center;
            }
            else {
                logging::warning("ignoring unknown justification specifier value: '{}' - expecting one of: left, right, or center (case-insensitive)", value);
            }
        }

        if (spec.has_specifier("width")) {
            std::string_view value = trim(spec.get_specifier("width"));

            unsigned _width;
            std::size_t num_characters_read = from_string(value, _width);

            if (num_characters_read < value.length()) {
                logging::warning("ignoring invalid width specifier value: '{}' - specifier value must be an integer", value);
            }
            else {
                width = _width;
            }
        }

        if (spec.has_specifier("fill", "fill_character", "fillcharacter")) {
            std::string_view value = trim(spec.get_specifier("fill", "fill_character", "fillcharacter"));
            if (value.length() > 1u) {
                logging::warning("ignoring invalid fill character specifier value: '{}' - specifier value must be a single character", value);
            }
            else {
                fill_character = value[0];
            }
        }
    }
    
}

#endif // STRING_TPP

