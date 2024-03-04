
#ifndef UTILS_STRING_TPP
#define UTILS_STRING_TPP

#include "utils/constexpr.hpp"
#include "utils/concepts.hpp"
#include "utils/result.hpp"
#include "utils/tuple.hpp"
#include "utils/assert.hpp"
#include "utils/enum.hpp"

#include <functional>
#include <cstdio> // std::snprintf
#include <memory> // std::unique_ptr
#include <stdexcept> // std::runtime_error
#include <tuple> // std::tuple_cat, std::make_tuple
#include <regex> // std::regex_match
#include <iostream>
#include <cassert>
#include <ostream>
#include <sstream>
#include <iomanip>
#include <charconv>

namespace utils {
    namespace internal {

        DEFINE_ENUM_BITFIELD_OPERATIONS(Formatting::Representation)
        DEFINE_ENUM_BITFIELD_OPERATIONS(Formatting::Justification)
        DEFINE_ENUM_BITFIELD_OPERATIONS(Formatting::Sign)
        
        class FormatString {
            public:
                FormatString(std::string_view in);
                ~FormatString();

                template <typename ...Ts>
                [[nodiscard]] std::string format(const Ts&... args) const;
                
                [[nodiscard]] std::size_t get_total_placeholder_count() const; // Includes duplicates
                [[nodiscard]] std::size_t get_unique_placeholder_count() const;
                [[nodiscard]] std::size_t get_positional_placeholder_count() const;
                [[nodiscard]] std::size_t get_named_placeholder_count() const;
                
            private:
                struct FormattedPlaceholder {
                    FormattedPlaceholder(std::size_t placeholder_index, const Formatting& formatting);
                    ~FormattedPlaceholder();
                    
                    void add_insertion_point(std::size_t position);
                    
                    std::size_t placeholder_index;
                    Formatting formatting;

                    // Positional and named placeholders can appear multiple times in the same format string.
                    // An optimization we can make when formatting placeholders is to format all unique placeholders once and cache them for later. A placeholders
                    // uniqueness is determined by its formatting specifiers - that is, if a placeholder has the same identifier and format specifiers as another,
                    // both placeholders will be formatted in the same way.
                    // We can save on processing power by simply keeping track of the positions in which a given placeholder and formatting specifiers are
                    // used to take advantage of work that was already done and avoid unnecessary duplicate formatting operations.
                    std::vector<std::size_t> insertion_points;
                };
                
                struct Identifier {
                    enum class Type {
                        None = 0,
                        Position,
                        Name
                    };
        
                    Identifier();
                    explicit Identifier(int position);
                    explicit Identifier(std::string name);
                    ~Identifier();
                    
                    [[nodiscard]] bool operator==(const Identifier& other) const;
        
                    Type type;
                    int position; // Allows for ~2 billion unique positions in a single format string.
                    std::string name;
                };

                // Allow external functions to convert
                friend std::string to_string(Identifier::Type);
                
                // Errors returned while parsing a format string.
                enum class ErrorCode {
                    // Error codes for use when parsing placeholder identifiers
                    Whitespace = 0,
                    DomainError,
                    InvalidIdentifier,
                    
                    // Error codes for use when parsing placeholder format specifier strings
                    InvalidFormatSpecifier,
                    EmptyFormatSpecifierString
                };
                
                struct Error {
                    explicit Error(ErrorCode code, int position = -1);
                    ~Error();
                    
                    ErrorCode code;
                    int position; // optional
                };
                
                [[nodiscard]] Result<Identifier, Error> parse_identifier(std::string_view in) const;
                [[nodiscard]] Result<Formatting, Error> parse_formatting(std::string_view in) const;
                
                void register_placeholder(const Identifier& identifier, const Formatting& formatting, std::size_t position);
                
                std::string m_format;
                std::vector<Identifier> m_placeholder_identifiers;
                std::vector<FormattedPlaceholder> m_formatted_placeholders;
        };
        
        template <typename T>
        struct is_format_arg : std::false_type { };
        
        template <typename T>
        struct is_format_arg<arg<T>> : std::true_type { };
        
        [[nodiscard]] char to_specifier(Formatting::Justification justification);
        [[nodiscard]] std::string to_string(Formatting::Justification justification);
        [[nodiscard]] char to_specifier(Formatting::Representation representation);
        [[nodiscard]] std::string to_string(Formatting::Representation representation);
        
        [[nodiscard]] inline char to_specifier(Formatting::Sign sign) {
            switch (sign) {
                case Formatting::Sign::NegativeOnly:
                    return '-';
                case Formatting::Sign::Aligned:
                    return ' ';
                case Formatting::Sign::Both:
                    return '+';
                default:
                    ASSERT(false, "unknown sign value ({})", static_cast<std::underlying_type<Formatting::Sign>::type>(sign));
                    return '\0';
            }
        }
        
        template <typename T>
        [[nodiscard]] std::string pointer_to_string(T pointer, const Formatting& formatting) {
            using Type = typename std::decay<T>::type;
            static_assert(std::is_pointer<Type>::value || std::is_null_pointer<Type>::value, "non-pointer type provided to pointer_to_string");

            std::stringstream builder { };
            
            // Formatting: sign
            if (formatting.sign.has_custom_value()) {
                // Signs on pointer values are not supported.
                throw FormatError("error formatting format string - invalid specifier '{}' for pointer type", to_specifier(*formatting.sign));
            }
            
            // Formatting: separator
            if (formatting.separator.has_custom_value()) {
                throw FormatError("error formatting format string - invalid specifier '{}' for pointer type", *formatting.separator);
            }
            
            // Formatting: data representation
            if (!formatting.representation.has_custom_value() || *formatting.representation == Formatting::Representation::Hexadecimal) {
                // By default, or explicitly, hexadecimal is used for representing pointer addresses.
                builder << std::hex;
            }
            else {
                char specifier = to_specifier(*formatting.representation);
                throw FormatError("error formatting format string - invalid specifier {} for pointer type", specifier);
            }
            
            if (*formatting.wildcard) {
                builder << std::showbase;
            }
            
            builder << (pointer ? (void*) pointer : "nullptr");
            std::string value = std::move(builder.str());
            builder.str(std::string()); // Clear builder internals.
            
            std::int64_t current_width = static_cast<std::int64_t>(value.length());
            unsigned desired_width = *formatting.width;
            std::int64_t left_padding = 0u;
            std::int64_t right_padding = 0u;
            
            // Formatting: justification
            Formatting::Justification justification = *formatting.justification;
            if (justification == Formatting::Justification::Right) {
                if (desired_width > current_width) {
                    left_padding = current_width - desired_width;
                }
            }
            else if (justification == Formatting::Justification::Left) {
                if (desired_width > current_width) {
                    right_padding = current_width - desired_width;
                }
            }
            else {
                if (desired_width > current_width) {
                    std::int64_t extra = current_width - desired_width;
                    std::int64_t half = extra / 2;
                    if (extra % 2 == 0) {
                        left_padding = half;
                        right_padding = half;
                    }
                    else {
                        left_padding = half + 1;
                        right_padding = half;
                    }
                }
            }
            
            char fill_character = *formatting.fill;
            builder << std::setw(left_padding) << std::setfill(fill_character) << "" << value << std::setw(right_padding) << std::setfill(fill_character) << "";
            
            return std::move(builder.str());
        }
        
        template <typename T>
        [[nodiscard]] std::string to_binary(T value, unsigned group_size = 0, char separator = ' ') {
            static_assert(is_integer_type<T>::value, "value must be an integer type");
            
            std::string result;
            
            if (group_size) {
                unsigned count = 0u;
                
                while (value > 0) {
                    ++count;
                    result += static_cast<char>((value % 2) + '0');
                    value /= 2;
    
                    if (count % group_size == 0u) {
                        result += separator;
                    }
                }
                
                // Make sure all groups contain the same number of characters.
                while (count % group_size != 0u) {
                    result += '0';
                    ++count;
                }
            }
            else {
                while (value > 0) {
                    result += static_cast<char>((value % 2) + '0');
                    value /= 2;
                }
            }
            
            std::reverse(result.begin(), result.end());
            return std::move(result);
        }
        
        template <typename T>
        [[nodiscard]] std::string to_hexadecimal(T value, unsigned group_size = 0u, char separator = ' ') {
            static_assert(is_integer_type<T>::value, "value must be an integer type");
            
            std::string result;
            
            if (group_size) {
                unsigned count = 0u;
                
                while (value > 0) {
                    ++count;
                    
                    unsigned remainder = value % 16;
                    if (remainder < 10) {
                        result += static_cast<char>(value + '0');
                    }
                    else {
                        remainder -= 10;
                        result += static_cast<char>(value + 'A');
                    }
                    value /= 16;
    
                    if (count % group_size == 0u) {
                        result += separator;
                    }
                }
                
                // Make sure all groups contain the same number of characters.
                while (count % group_size != 0u) {
                    result += '0';
                    ++count;
                }
            }
            else {
                while (value > 0) {
                    unsigned remainder = value % 16;
                    if (remainder < 10) {
                        result += static_cast<char>(value + '0');
                    }
                    else {
                        remainder -= 10;
                        result += static_cast<char>(value + 'A');
                    }
                    value /= 16;
                }
            }
            
            std::reverse(result.begin(), result.end());
            return std::move(result);
        }
        
        inline std::string apply_justification(const std::string& value, Formatting::Justification justification, unsigned minimum_width, char fill_character) {
            std::size_t length = value.length();
            if (length >= minimum_width) {
                // Minimum width is less than the current width, justification is a noop.
                return value;
            }
            
            std::string result;
            result.resize(minimum_width, fill_character);
            
            std::size_t offset;
            
            if (justification == Formatting::Justification::Left) {
                offset = 0u;
            }
            else if (justification == Formatting::Justification::Right) {
                offset = minimum_width - length;
            }
            else {
                // Center justification.
                offset = (minimum_width - length) / 2;
            }
            
            for (std::size_t i = 0u; i < length; ++i) {
                result[i + offset] = value[i];
            }
            
            return std::move(result);
        }
        
        [[nodiscard]] inline std::string separate(std::string_view in, char separator) {
            std::ostringstream builder { };
            std::size_t current;
            
            // Append decimal portion.
            std::size_t start = in.find('.');
            if (start != std::string::npos) {
                if (start == in.length() - 1u) {
                    builder << 0;
                }
                else {
                    for (std::size_t i = in.length() - 1u; i != start; --i) {
                        builder << in[i];
                    }
                }
                
                // Append decimal.
                builder << '.';
                current = start - 1u;
            }
            else {
                current = in.length() - 1u;
            }
            
            if (start != 0u) {
                bool negative = true;
                std::size_t end = in.find('-');
                if (end == std::string::npos) {
                    end = 0u;
                    negative = false;
                }
        
                std::size_t count = 0u;
                while (current != end) {
                    builder << in[current];
                    ++count;
                    
                    if (count % 3u == 0u) {
                        // Append separator.
                        
                        if (negative) {
                            if (current - 1u != end) {
                                // Avoid extra leading separator: -,123,456.789
                                builder << separator;
                            }
                        }
                        else {
                            builder << separator;
                        }
                    }
                    
                    --current;
                }
                
                if (negative) {
                    builder << '-';
                }
                else {
                    builder << in[end];
                }
            }
            else {
                // Decimal is at the first index of the input string.
                builder << 0;
            }
            
            std::string result = builder.str();
            std::reverse(result.begin(), result.end());
            return std::move(result);
        }
        
        template <typename T>
        [[nodiscard]] std::string fundamental_to_string(T value, const Formatting& formatting) {
            using Type = typename std::decay<T>::type;
            static_assert(std::is_fundamental<Type>::value, "type provided to fundamental_to_string must be built-in");
            
            // Format specifiers can be split into two categories: internal and external.
            // Internal specifiers affect how the value is displayed:
            //   - type representation
            //   - sign
            //   - separator character
            //   - wildcard
            //   - precision
            // External specifiers affect how the space around the value is used:
            //   - justification
            //   - fill character
            //   - width
            
            bool is_character_type = std::is_same<Type, char>::value || std::is_same<Type, signed char>::value;
            std::ostringstream builder { };
            
            if (is_character_type) {
                if (formatting.representation.has_custom_value()) {
                    Formatting::Representation representation = *formatting.representation;
                    throw FormatError("error formatting format string - invalid format specifier '{}' ({} representation is not supported for character types)", to_specifier(representation), to_string(representation));
                }
                
                if (formatting.sign.has_custom_value()) {
                    throw FormatError("error formatting format string - invalid format specifier '{}' (sign format specifiers are not supported for character types)", to_specifier(*formatting.sign));
                }
                
                if (formatting.separator.has_custom_value()) {
                    throw FormatError("error formatting format string - invalid format specifier '{}' (separator characters are not supported for character types)", *formatting.separator);
                }
                
                if (formatting.wildcard.has_custom_value()) {
                    throw FormatError("error formatting format string - invalid format specifier '{}' (wildcard format specifier is not valid for character types");
                }
                
                if (formatting.precision.has_custom_value()) {
                    throw FormatError("error formatting format string - precision specifier '.{}' is not valid for character types", std::to_string(*formatting.precision));
                }
                
                builder << value;
            }
            else {
                // Formatting: sign.
                switch (*formatting.sign) {
                    case Formatting::Sign::NegativeOnly:
                        if (value < T(0)) {
                            builder << '-';
                            value = -value;
                        }
                        break;
                    case Formatting::Sign::Aligned:
                        if (value < T(0)) {
                            builder << '-';
                            value = -value;
                        }
                        else {
                            builder << ' ';
                        }
                        break;
                    case Formatting::Sign::Both:
                        if (value < T(0)) {
                            builder << '-';
                            value = -value;
                        }
                        else if (value == T(0)) {
                            builder << ' ';
                        }
                        else {
                            builder << '+';
                        }
                        break;
                }
            
                Formatting::Representation representation = *formatting.representation;
                
                if constexpr (is_integer_type<Type>::value) {
                    switch (representation) {
                        case Formatting::Representation::Decimal: {
                            if (formatting.wildcard.has_custom_value()) {
                            }
                            if (formatting.precision.has_custom_value()) {
                            }
                            
                            builder << value;
                            
                            if (formatting.separator.has_custom_value()) {
                                std::string internal = builder.str();
                                builder.str(std::string()); // Clear stream internals.
                                
                                builder << separate(internal, *formatting.separator);
                            }
                            
                            break;
                        }
                        case Formatting::Representation::Binary: {
                            // For binary representations of integer values, the wildcard specifier controls whether to add a base prefix to the result.
                            bool use_base_prefix = *formatting.wildcard;
                            if (use_base_prefix) {
                                builder << "0b";
                            }
        
                            bool has_custom_precision = formatting.precision.has_custom_value();
                            bool has_custom_separator = formatting.separator.has_custom_value();
                            
                            if (has_custom_precision && has_custom_separator) {
                                unsigned group_size = *formatting.precision;
                                char separator = *formatting.separator;
                                
                                // Group size must be a power of 2.
                                bool is_power_of_two = (group_size != 0u) && ((group_size & (group_size - 1u)) == 0u);
                                if (!is_power_of_two) {
                                    throw FormatError("error formatting format string - group size must be a power of 2 (received: {})", group_size);
                                }
        
                                // If applicable, binary representation and base prefix are separated by the separator character.
                                if (use_base_prefix) {
                                    builder << separator;
                                }
                                builder << to_binary(value, group_size, separator);
                            }
                            else if (!has_custom_precision && !has_custom_separator) {
                                builder << to_binary(value);
                            }
                            else {
                                throw FormatError("error formatting format string - missing value for {} (group size and separator character must both be explicitly specified for structured binary representations)", has_custom_precision ? "separator character" : "group size");
                            }
                            break;
                        }
                        case Formatting::Representation::Hexadecimal: {
                            // For hexadecimal representations of integer values, the wildcard specifier controls whether to add a base prefix to the result.
                            bool use_base_prefix = *formatting.wildcard;
                            if (use_base_prefix) {
                                builder << "0x";
                            }
        
                            bool has_custom_precision = formatting.precision.has_custom_value();
                            bool has_custom_separator = formatting.separator.has_custom_value();
                            
                            if (has_custom_precision && has_custom_separator) {
                                unsigned group_size = *formatting.precision;
                                char separator = *formatting.separator;
                                
                                // Group size must be a power of 2.
                                bool is_power_of_two = (group_size != 0u) && ((group_size & (group_size - 1u)) == 0u);
                                if (!is_power_of_two) {
                                    throw FormatError("error formatting format string - group size must be a power of 2 (received: {})", group_size);
                                }
                                
                                // If the base prefix is applied, it is separated from the rest of the hexadecimal representation by the separator character.
                                if (use_base_prefix) {
                                    builder << separator;
                                }
                                
                                builder << to_hexadecimal(value, group_size, separator);
                            }
                            else if (!has_custom_precision && !has_custom_separator) {
                                builder << to_hexadecimal(value);
                            }
                            else {
                                throw FormatError("error formatting format string - missing value for {} (group size and separator character must both be explicitly specified for structured hexadecimal representations)", has_custom_precision ? "separator character" : "group size");
                            }
                            break;
                        }
                        case Formatting::Representation::Scientific: {
                            if (formatting.separator.has_custom_value()) {
                            }
                            
                            if (formatting.wildcard.has_custom_value()) {
                            }
            
                            // Integer values must be cast to a floating point type to be displayed properly using scientific notation.
                            builder << std::scientific << std::setprecision(*formatting.precision) << static_cast<long double>(value);
                            break;
                        }
                        case Formatting::Representation::Fixed: {
                            if (formatting.wildcard.has_custom_value()) {
                            }
                            
                            if (formatting.separator.has_custom_value()) {
                                std::string internal = builder.str();
                                builder.str(std::string()); // Clear stream internals.
                                
                                builder << separate(internal, *formatting.separator);
                            }
                            else {
                                builder << value;
                            }
        
                            // Integer values have no decimal places, but this can be faked manually if a fixed precision is requested.
                            if (formatting.precision.has_custom_value()) {
                                unsigned precision = *formatting.precision;
                                if (precision > 0) {
                                    builder << '.';
                                    for (unsigned i = 0u; i < precision; ++i) {
                                        builder << 0;
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
                else {
                    // Floating point type.
                    switch (representation) {
                        case Formatting::Representation::Decimal: {
                            if (formatting.wildcard.has_custom_value()) {
                            }

                            // Precision is applied by default for floating point numbers.
                            builder << std::setprecision(*formatting.precision) << value;
                            
                            if (formatting.separator.has_custom_value()) {
                                std::string internal = builder.str();
                                builder.str(std::string()); // Clear stream internals.
                                
                                builder << separate(internal, *formatting.separator);
                            }
                            break;
                        }
                        case Formatting::Representation::Scientific: {
                            if (formatting.wildcard.has_custom_value()) {
                            }
                            
                            if (formatting.separator.has_custom_value()) {
                            }
                            
                            // Precision is applied by default for floating point numbers.
                            builder << std::scientific << std::setprecision(*formatting.precision) << value;
                            break;
                        }
                        case Formatting::Representation::Fixed: {
                            if (formatting.wildcard.has_custom_value()) {
                            }
                            
                            // Precision is applied by default for floating point numbers.
                            builder << std::fixed << std::setprecision(*formatting.precision) << value;

                            if (formatting.separator.has_custom_value()) {
                                std::string internal = builder.str();
                                builder.str(std::string()); // Clear stream internals.
                                
                                builder << separate(internal, *formatting.separator);
                            }
                            break;
                        }
                        
                        // Unsupported representations.
                        case Formatting::Representation::Binary:
                        case Formatting::Representation::Hexadecimal:
                            throw FormatError("error formatting format string - invalid format specifier '{}' ({} representation of floating point numbers is not supported)", to_specifier(representation), to_string(representation));
                    }
                }
            }
            
            // Apply external format specifiers.
            return apply_justification(builder.str(), *formatting.justification, *formatting.width, *formatting.fill);
        }
        
        template <typename T>
        [[nodiscard]] std::string container_to_string(const T& container, const Formatting& formatting) {
            static_assert(is_const_iterable<T>, "container type provided to container_to_string must support const iteration (begin/end).");
            auto current = std::begin(container);
            auto end = std::end(container);
            
            if ((end - current) == 0u) {
                // Special formatting for when a container has no elements.
                return "[]";
            }
            
            std::string result = "[ ";
            
            // If custom formatting is specified, it is applied to container elements.
            result.append(to_string(*current, formatting));
            for (++current; current != end; ++current) {
                result.append(", " + to_string(*current, formatting));
            }
            
            result.append(" ]");

            return std::move(result);
        }
        
        template <typename T>
        [[nodiscard]] std::string tuple_to_string(const T& value, const Formatting& formatting) {
            using Type = typename std::decay<T>::type;
            static_assert(is_pair<Type>::value || is_tuple<Type>::value, "type provided to tuple_to_string must be a tuple type");
            
            std::string result = "{ ";
            
            // Use std::apply + fold expression to iterate over and format the elements of the tuple.
            std::apply([&result, &formatting](const auto&&... args) {
                ((result.append(stringify(args, formatting) + ", ")), ...);
            }, value);
            
            // Overwrite the trailing ", " with " }".
            std::size_t length = result.length();
            result[length - 2u] = ' ';
            result[length - 1u] = '}';
            return result;
        }
        
        template <typename Tuple, std::size_t N = 0>
        bool is_structured_argument_type(const Tuple& tuple, std::size_t index) {
            if (N == index) {
                using Type = typename std::decay<decltype(std::get<N>(tuple))>::type;
                return is_format_arg<Type>::value;
            }
            
            if constexpr (N + 1 < std::tuple_size<Tuple>::value) {
                return is_structured_argument_type<Tuple, N + 1>(tuple, index);
            }
            
            ASSERT(false, "tuple index {} is out of bounds", index);
            return false;
        }
        
        [[nodiscard]] inline std::string to_string(const Formatting& formatting) {
            std::stringstream builder;
            
            builder << '{';
            
            if (formatting.justification.has_custom_value()) {
                if (formatting.fill.has_custom_value()) {
                    builder << *formatting.fill;
                }
                
                builder << to_specifier(*formatting.justification);
            }

            if (formatting.sign.has_custom_value()) {
                builder << to_specifier(*formatting.sign);
            }
            
            if (*formatting.wildcard) {
                builder << '#';
            }
            
            if (formatting.width.has_custom_value()) {
                builder << std::to_string(*formatting.width);
            }
            
            if (formatting.separator.has_custom_value()) {
                builder << *formatting.separator;
            }
            
            if (formatting.precision.has_custom_value()) {
                builder << '.' << std::to_string(*formatting.precision);
            }
            
            if (formatting.representation.has_custom_value()) {
                builder << to_specifier(*formatting.representation);
            }
            
            builder << '}';
            
            return std::move(builder.str());
        }
        
        template <typename ...Ts>
        [[noreturn]] void reformat(const FormatError& error, const Ts&... args) {
            throw FormatError(error.what(), args...);
        }
        
        template <typename T>
        [[nodiscard]] std::string to_string(const T& value, const Formatting& formatting) {
            using Type = typename std::decay<T>::type;
        
            if constexpr (std::is_fundamental<Type>::value && !std::is_null_pointer<Type>::value) {
                // C++ built-ins
                return fundamental_to_string(value, formatting);
            }
            else if constexpr (is_standard_container<Type>::value) {
                // standard C++ container
                return container_to_string(value, formatting);
            }
            else if constexpr (is_pair<Type>::value || is_tuple<Type>::value) {
                // std::pair, std::tuple
                return tuple_to_string(value, formatting);
            }
            else if constexpr (std::is_same<Type, std::string>::value) {
                // std::string
                return value;
            }
            else if constexpr (std::is_pointer<Type>::value || std::is_null_pointer<Type>::value) {
                // pointer types
                return pointer_to_string(value, formatting);
            }
            else if constexpr (is_format_arg<Type>::value) {
                return ""; //to_string(value.value, formatting);
            }
            // Utilize user-defined std::string conversion operators for all other custom types.
            // This implementation allows for either a std::string conversion class operator (T::operator std::string(), preferred) or a standalone to_string(const T&) function.
            else if constexpr (is_convertible_to_string<Type>) {
                // Note: can also use std::is_convertible<Type, std::string>::value if the conversion operator is not marked explicit.
                return std::string(value);
            }
            else {
                return to_string(value);
            }
        }
        
        template <typename ...Ts>
        std::string FormatString::format(const Ts& ...args) const {
            std::string result = m_format;

            if constexpr (sizeof...(args) == 0u) {
                return m_format;
            }
            
            if (m_placeholder_identifiers.empty()) {
                if (sizeof...(args) > 0u) {
                    // TODO: log warning message
                }
            }
            else {
                auto tuple = std::make_tuple(args...);
                
                if (m_placeholder_identifiers[0].type == Identifier::Type::None) {
                    // For unstructured format strings, there is a 1:1 correlation between a placeholder value and its insertion point.
                    // Hence, the number of arguments provided to format(...) should be at least as many as the number of placeholders.
                    // Note: while it is valid to provide more arguments than necessary, these arguments will be ignored.
                    std::size_t placeholder_count = get_unique_placeholder_count();
                    if (placeholder_count > sizeof...(args)) {
                        // Not enough arguments provided to format(...);
                        throw FormatError("error formatting format string - expecting {} arguments, but received {}", placeholder_count, sizeof...(args));
                    }
                    
                    // Unstructured format strings should only contain auto-numbered placeholders - verify that there are no positional / named argument values present in the arguments provided to format(...).
                    for (std::size_t i = 0u; i < placeholder_count; ++i) {
                        if (is_structured_argument_type(tuple, i)) {
                            const std::string& name = runtime_get(tuple, i, [i]<typename T>(const T& value) -> const std::string& {
                                if constexpr (is_format_arg<T>::value) {
                                    return value.name;
                                }
                                
                                // This should never happen.
                                
                                // TODO: assert instead
                                throw std::runtime_error(utils::format("internal runtime_get error - invalid type at tuple index {}", i));
                            });
                            
                            throw FormatError("error formatting format string - encountered value for named placeholder {} at index {} (structured placeholder values are not allowed in unstructured format strings)", name, i);
                        }
                    }
                }
                else {
                    // Format string should only contain positional / named placeholders.
                    std::size_t positional_placeholder_count = get_positional_placeholder_count();
                    std::size_t named_placeholder_count = get_named_placeholder_count();
                    
                    // TODO: check positional placeholder indices, warn on gaps or non-consecutive values resulting in arguments not being used
                    
                    if (positional_placeholder_count + named_placeholder_count > sizeof...(args)) {
                        // Not enough arguments provided to format(...);
                        throw FormatError("error formatting format string - expecting {} arguments, but received {}", positional_placeholder_count + named_placeholder_count, sizeof...(args));
                    }

                    for (std::size_t i = 0u; i < positional_placeholder_count; ++i) {
                        if (is_structured_argument_type(tuple, i)) {
                            const std::string& name = runtime_get(tuple, i, [i]<typename T>(const T& value) -> const std::string& {
                                if constexpr (is_format_arg<T>::value) {
                                    return value.name;
                                }
                                // This should never happen.
                                // TODO: assert
                                throw std::runtime_error(utils::format("internal runtime_get error - invalid type at tuple index {}", i));
                            });

                            // Retrieve the first position at which this placeholder was referenced.
                            std::size_t position = std::string::npos;
                            for (const FormattedPlaceholder& placeholder : m_formatted_placeholders) {
                                if (placeholder.placeholder_index == i && !placeholder.insertion_points.empty()) {
                                    position = placeholder.insertion_points[0];
                                }
                            }
                            
                            if (position != std::string::npos) {
                                throw std::runtime_error(utils::format("expecting value for positional placeholder {} (not referenced), but received value for named placeholder {} - values for all positional placeholders must come before any values for named placeholders", i, name));
                            }
                            else {
                                throw std::runtime_error(utils::format("expecting value for positional placeholder {} (first referenced at index {}), but received value for named placeholder {} - values for all positional placeholders must come before any values for named placeholders", i, position, name));
                            }
                        }
                    }
                    
                    // Verify that all named placeholders have
//                    for (std::size_t i = 0u; i < get_unique_placeholder_count(); ++i) {
//                        const PlaceholderIdentifier& identifier = m_placeholder_identifiers[i];
//                        if (identifier.type == PlaceholderIdentifier::Type::Name) {
//                            bool found = for_each(tuple, [&identifier]<typename T>(const T& a) -> bool {
//                                if constexpr (is_format_arg<typename std::decay<T>::type>::value) {
//                                    return a.name == identifier.name;
//                                }
//                                else {
//                                    return false;
//                                }
//                            });
//
//                            if (!found) {
//                                return "named placeholder " + identifier.name + " missing argument";
//                            }
//                        }
//                    }
                }
                
                // Format all unique placeholders once to save on computation power.
                // This is only really applicable for positional / named placeholder values, since these can be referenced multiple times
                // in the format string, but this logic is near identical for auto-numbered placeholder values.
                std::size_t unique_placeholder_count = get_unique_placeholder_count();
                
                std::vector<std::string> formatted_placeholders;
                formatted_placeholders.reserve(unique_placeholder_count);

                for (std::size_t i = 0u; i < unique_placeholder_count; ++i) {
                    const FormattedPlaceholder& placeholder = m_formatted_placeholders[i];
                    const Formatting& formatting = placeholder.formatting;
                    
                    formatted_placeholders.emplace_back(runtime_get(tuple, i, [i, &formatting] <typename T>(const T& value) -> std::string {
                        try {
                            return internal::to_string<T>(value, formatting);
                        }
                        catch (const FormatError& e) {
                            reformat(e, arg("position", 5), arg("argind", i));
                        }
                    }));
                }
                
                // Values for placeholders are inserted into the resulting string in reverse order of appearance so that inserting a
                // placeholder value does not offset the insertion positions of any placeholders that come before it.

                struct InsertionPoint {
                    std::size_t placeholder_index; // Index of the placeholder to insert.
                    std::size_t position; // Position at which the placeholder should be inserted.
                };
                
                // Comparator to create a min heap based on the insertion point position.
                static auto comparator = [](const InsertionPoint& a, const InsertionPoint& b) -> bool {
                    return a.position > b.position;
                };
                
                std::priority_queue<InsertionPoint, std::vector<InsertionPoint>, decltype(comparator)> insertion_points(comparator);
                for (std::size_t i = 0u; i < unique_placeholder_count; ++i) {
                    const FormattedPlaceholder& placeholder = m_formatted_placeholders[i];
                    
                    for (std::size_t position : placeholder.insertion_points) {
                        insertion_points.emplace(placeholder.placeholder_index, position);
                    }
                }
                
                // Placeholder values are inserted into the string front to back. This allows an easier way of handling insertions for
                // placeholders that are directly adjacent due to peculiarities with the std::string::insert() function inserting starting at
                // the character right before the indicated position. By keeping track of an offset and adjusting the insertion point of
                // subsequent placeholders accordingly, we can insert values for adjacent placeholders without any extra whitespace.
                
                std::size_t offset = 0u;
                
                while (!insertion_points.empty()) {
                    const InsertionPoint& insertion_point = insertion_points.top();
                    const std::string& placeholder_value = formatted_placeholders[insertion_point.placeholder_index];
                    
                    result.insert(insertion_point.position + offset, placeholder_value);
                    offset += placeholder_value.length();
                    
                    insertion_points.pop();
                }
            }
            
            return result;
        }
        
    }
    
    
    template <typename Container>
    [[nodiscard]] std::string join(const Container& container, const std::string& glue) {
        // TODO: support custom containers with iterators
//        static_assert(is_const_iterable<Container>, "container must support std::begin/std::end");
        
        auto iter = std::begin(container);
        std::string result;

//        result.append(internal::stringify(*iter));
//        for (++iter; iter != std::end(container); ++iter) {
//            result.append(glue);
//            result.append(internal::stringify(*iter));
//        }

        return std::move(result);
    }

    template <typename T>
    FormattingSpecifier<T>::FormattingSpecifier(T value) : m_value(std::make_pair(value, false)) {
    }
    
    template <typename T>
    FormattingSpecifier<T>::~FormattingSpecifier() = default;
    
    template <typename T>
    FormattingSpecifier<T>::FormattingSpecifier(const FormattingSpecifier<T>& other) : m_value(other.m_value) {
        // Preserve whether this holds a custom value or not.
    }
    
    template <typename T>
    FormattingSpecifier<T>& FormattingSpecifier<T>::operator=(const FormattingSpecifier<T>& other) {
        // Preserve whether this holds a custom value or not.
        if (this != &other) {
            m_value = other.m_value;
        }
        
        return *this;
    }
    
    template <typename T>
    bool FormattingSpecifier<T>::has_custom_value() const {
        return m_value.second;
    }
    
    template <typename T>
    void FormattingSpecifier<T>::set(T value) {
        m_value.first = value;
        m_value.second = true;
    }
    
    template <typename T>
    FormattingSpecifier<T>& FormattingSpecifier<T>::operator=(T value) {
        m_value.first = value;
        m_value.second = true;
        return *this;
    }
    
    template <typename T>
    T FormattingSpecifier<T>::operator*() const {
        return m_value.first;
    }
    
    template <typename T>
    T FormattingSpecifier<T>::get() const {
        return m_value.first;
    }
    
    template <typename ...Ts>
    FormatError::FormatError(const std::string& fmt, const Ts& ...args) : std::runtime_error(utils::format(fmt, args...)) {
    }
    
    template <typename T>
    arg<T>::arg(std::string name, const T& value) : name(std::move(name)),
                                                    value(value) {
    }
    
    template <typename T>
    arg<T>::~arg() = default;
    
    template <typename ...Ts>
    std::string format(std::string_view fmt, const Ts&... args) {
        using namespace internal;
        FormatString str = FormatString(fmt);
        
        if constexpr (sizeof...(args) > 0) {
            return str.format(args...);
        }
        else {
            return "";
        }
    }
    
    template <typename T>
    std::string to_string(const T& value, const Formatting& formatting) {
        return internal::to_string<T>(value, formatting);
    }
    
}

#endif // UTILS_STRING_TPP
