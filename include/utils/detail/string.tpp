
#pragma once

#ifndef UTILS_STRING_TPP
#define UTILS_STRING_TPP

#include "utils/constexpr.hpp"
#include "utils/concepts.hpp"
#include "utils/result.hpp"
#include "utils/tuple.hpp"
#include "utils/assert.hpp"
#include "utils/enum.hpp"
#include "utils/exceptions.hpp"
#include "utils/logging/logging.hpp"

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
#include <bitset> // std::bitset

// TODO: move elsewhere

namespace utils {
    
    namespace internal {

//        template <typename T>
//        [[nodiscard]] std::string pointer_to_string(T pointer, const Formatting& formatting) {
//            using Type = typename std::decay<T>::type;
//            static_assert(std::is_pointer<Type>::value || std::is_null_pointer<Type>::value, "non-pointer type provided to pointer_to_string");
//
//            std::stringstream builder { };
//            // Formatting: sign
//            if (formatting.sign.has_custom_value()) {
//                // Signs on pointer values are not supported.
//                throw FormatError("error formatting format string - invalid specifier '{}' for pointer type", to_specifier(*formatting.sign));
//            }
//
//            // Formatting: separator
//            if (formatting.separator.has_custom_value()) {
//                throw FormatError("error formatting format string - invalid specifier '{}' for pointer type", *formatting.separator);
//            }
//
//            // Formatting: data representation
//            if (!formatting.representation.has_custom_value() || *formatting.representation == Formatting::Representation::Hexadecimal) {
//                // By default, or explicitly, hexadecimal is used for representing pointer addresses.
//                builder << std::hex;
//            }
//            else {
//                char specifier = to_specifier(*formatting.representation);
//                throw FormatError("error formatting format string - invalid specifier {} for pointer type", specifier);
//            }
//
//            if (*formatting.wildcard) {
//                builder << std::showbase;
//            }
//
//            builder << (pointer ? (void*) pointer : "nullptr");
//            std::string value = std::move(builder.str());
//            builder.str(std::string()); // Clear builder internals.
//
//            std::int64_t current_width = static_cast<std::int64_t>(value.length());
//            unsigned desired_width = *formatting.width;
//            std::int64_t left_padding = 0u;
//            std::int64_t right_padding = 0u;
//
//            // Formatting: justification
//            Formatting::Justification justification = *formatting.justification;
//            if (justification == Formatting::Justification::Right) {
//                if (desired_width > current_width) {
//                    left_padding = current_width - desired_width;
//                }
//            }
//            else if (justification == Formatting::Justification::Left) {
//                if (desired_width > current_width) {
//                    right_padding = current_width - desired_width;
//                }
//            }
//            else {
//                if (desired_width > current_width) {
//                    std::int64_t extra = current_width - desired_width;
//                    std::int64_t half = extra / 2;
//                    if (extra % 2 == 0) {
//                        left_padding = half;
//                        right_padding = half;
//                    }
//                    else {
//                        left_padding = half + 1;
//                        right_padding = half;
//                    }
//                }
//            }
//
//            char fill_character = *formatting.fill;
//            builder << std::setw(left_padding) << std::setfill(fill_character) << "" << value << std::setw(right_padding) << std::setfill(fill_character) << "";
//
//            return std::move(builder.str());
//        }
//
//        template <typename T>
//        [[nodiscard]] std::string to_binary(T value, unsigned group_size = 0, char separator = ' ') {
//            static_assert(is_integer_type<T>::value, "value must be an integer type");
//
//            std::string result;
//
//            if (group_size) {
//                unsigned count = 0u;
//
//                while (value > 0) {
//                    ++count;
//                    result += static_cast<char>((value % 2) + '0');
//                    value /= 2;
//
//                    if (count % group_size == 0u) {
//                        result += separator;
//                    }
//                }
//
//                // Make sure all groups contain the same number of characters.
//                while (count % group_size != 0u) {
//                    result += '0';
//                    ++count;
//                }
//            }
//            else {
//                while (value > 0) {
//                    result += static_cast<char>((value % 2) + '0');
//                    value /= 2;
//                }
//            }
//
//            std::reverse(result.begin(), result.end());
//            return std::move(result);
//        }
//
//        template <typename T>
//        [[nodiscard]] std::string to_hexadecimal(T value, unsigned group_size = 0u, char separator = ' ') {
//            static_assert(is_integer_type<T>::value, "value must be an integer type");
//
//            std::string result;
//
//            if (group_size) {
//                unsigned count = 0u;
//
//                while (value > 0) {
//                    ++count;
//
//                    unsigned remainder = value % 16;
//                    if (remainder < 10) {
//                        result += static_cast<char>(value + '0');
//                    }
//                    else {
//                        remainder -= 10;
//                        result += static_cast<char>(value + 'A');
//                    }
//                    value /= 16;
//
//                    if (count % group_size == 0u) {
//                        result += separator;
//                    }
//                }
//
//                // Make sure all groups contain the same number of characters.
//                while (count % group_size != 0u) {
//                    result += '0';
//                    ++count;
//                }
//            }
//            else {
//                while (value > 0) {
//                    unsigned remainder = value % 16;
//                    if (remainder < 10) {
//                        result += static_cast<char>(value + '0');
//                    }
//                    else {
//                        remainder -= 10;
//                        result += static_cast<char>(value + 'A');
//                    }
//                    value /= 16;
//                }
//            }
//
//            std::reverse(result.begin(), result.end());
//            return std::move(result);
//        }
//
//        inline std::string apply_justification(const std::string& value, Formatting::Justification justification, unsigned minimum_width, char fill_character) {
//            std::size_t length = value.length();
//            if (length >= minimum_width) {
//                // Minimum width is less than the current width, justification is a noop.
//                return value;
//            }
//
//            std::string result;
//            result.resize(minimum_width, fill_character);
//
//            std::size_t offset;
//
//            if (justification == Formatting::Justification::Left) {
//                offset = 0u;
//            }
//            else if (justification == Formatting::Justification::Right) {
//                offset = minimum_width - length;
//            }
//            else {
//                // Center justification.
//                offset = (minimum_width - length) / 2;
//            }
//
//            for (std::size_t i = 0u; i < length; ++i) {
//                result[i + offset] = value[i];
//            }
//
//            return std::move(result);
//        }
//
//        [[nodiscard]] inline std::string separate(std::string_view in, char separator) {
//            std::ostringstream builder { };
//            std::size_t current;
//
//            // Append decimal portion.
//            std::size_t start = in.find('.');
//            if (start != std::string::npos) {
//                if (start == in.length() - 1u) {
//                    builder << 0;
//                }
//                else {
//                    for (std::size_t i = in.length() - 1u; i != start; --i) {
//                        builder << in[i];
//                    }
//                }
//
//                // Append decimal.
//                builder << '.';
//                current = start - 1u;
//            }
//            else {
//                current = in.length() - 1u;
//            }
//
//            if (start != 0u) {
//                bool negative = true;
//                std::size_t end = in.find('-');
//                if (end == std::string::npos) {
//                    end = 0u;
//                    negative = false;
//                }
//
//                std::size_t count = 0u;
//                while (current != end) {
//                    builder << in[current];
//                    ++count;
//
//                    if (count % 3u == 0u) {
//                        // Append separator.
//
//                        if (negative) {
//                            if (current - 1u != end) {
//                                // Avoid extra leading separator: -,123,456.789
//                                builder << separator;
//                            }
//                        }
//                        else {
//                            builder << separator;
//                        }
//                    }
//
//                    --current;
//                }
//
//                if (negative) {
//                    builder << '-';
//                }
//                else {
//                    builder << in[end];
//                }
//            }
//            else {
//                // Decimal is at the first index of the input string.
//                builder << 0;
//            }
//
//            std::string result = builder.str();
//            std::reverse(result.begin(), result.end());
//            return std::move(result);
//        }
//
//        template <typename T>
//        [[nodiscard]] std::string fundamental_to_string(T value, const Formatting& formatting) {
//            using Type = typename std::decay<T>::type;
//            static_assert(std::is_fundamental<Type>::value, "type provided to fundamental_to_string must be built-in");
//
//            // Format specifiers can be split into two categories: internal and external.
//            // Internal specifiers affect how the value is displayed:
//            //   - type representation
//            //   - sign
//            //   - separator character
//            //   - wildcard
//            //   - precision
//            // External specifiers affect how the space around the value is used:
//            //   - justification
//            //   - fill character
//            //   - width
//
//            bool is_character_type = std::is_same<Type, char>::value || std::is_same<Type, signed char>::value;
//            std::ostringstream builder { };
//
//            if (is_character_type) {
//                if (formatting.representation.has_custom_value()) {
//                    Formatting::Representation representation = *formatting.representation;
//                    throw FormatError("error formatting format string - invalid format specifier '{}' ({} representation is not supported for character types)", to_specifier(representation), to_string(representation));
//                }
//
//                if (formatting.sign.has_custom_value()) {
//                    throw FormatError("error formatting format string - invalid format specifier '{}' (sign format specifiers are not supported for character types)", to_specifier(*formatting.sign));
//                }
//
//                if (formatting.separator.has_custom_value()) {
//                    throw FormatError("error formatting format string - invalid format specifier '{}' (separator characters are not supported for character types)", *formatting.separator);
//                }
//
//                if (formatting.wildcard.has_custom_value()) {
//                    throw FormatError("error formatting format string - invalid format specifier '{}' (wildcard format specifier is not valid for character types");
//                }
//
//                if (formatting.precision.has_custom_value()) {
//                    throw FormatError("error formatting format string - precision specifier '.{}' is not valid for character types", std::to_string(*formatting.precision));
//                }
//
//                builder << value;
//            }
//            else {
//                // Formatting: sign.
//                switch (*formatting.sign) {
//                    case Formatting::Sign::NegativeOnly:
//                        if (value < T(0)) {
//                            builder << '-';
//                            value = -value;
//                        }
//                        break;
//                    case Formatting::Sign::Aligned:
//                        if (value < T(0)) {
//                            builder << '-';
//                            value = -value;
//                        }
//                        else {
//                            builder << ' ';
//                        }
//                        break;
//                    case Formatting::Sign::Both:
//                        if (value < T(0)) {
//                            builder << '-';
//                            value = -value;
//                        }
//                        else if (value == T(0)) {
//                            builder << ' ';
//                        }
//                        else {
//                            builder << '+';
//                        }
//                        break;
//                }
//
//                Formatting::Representation representation = *formatting.representation;
//
//                if constexpr (is_integer_type<Type>::value) {
//                    switch (representation) {
//                        case Formatting::Representation::Decimal: {
//                            if (formatting.wildcard.has_custom_value()) {
//                            }
//                            if (formatting.precision.has_custom_value()) {
//                            }
//
//                            builder << value;
//
//                            if (formatting.separator.has_custom_value()) {
//                                std::string internal = builder.str();
//                                builder.str(std::string()); // Clear stream internals.
//
//                                builder << separate(internal, *formatting.separator);
//                            }
//
//                            break;
//                        }
//                        case Formatting::Representation::Binary: {
//                            // For binary representations of integer values, the wildcard specifier controls whether to add a base prefix to the result.
//                            bool use_base_prefix = *formatting.wildcard;
//                            if (use_base_prefix) {
//                                builder << "0b";
//                            }
//
//                            bool has_custom_precision = formatting.precision.has_custom_value();
//                            bool has_custom_separator = formatting.separator.has_custom_value();
//
//                            if (has_custom_precision && has_custom_separator) {
//                                unsigned group_size = *formatting.precision;
//                                char separator = *formatting.separator;
//
//                                // Group size must be a power of 2.
//                                bool is_power_of_two = (group_size != 0u) && ((group_size & (group_size - 1u)) == 0u);
//                                if (!is_power_of_two) {
//                                    throw FormatError("error formatting format string - group size must be a power of 2 (received: {})", group_size);
//                                }
//
//                                // If applicable, binary representation and base prefix are separated by the separator character.
//                                if (use_base_prefix) {
//                                    builder << separator;
//                                }
//                                builder << to_binary(value, group_size, separator);
//                            }
//                            else if (!has_custom_precision && !has_custom_separator) {
//                                builder << to_binary(value);
//                            }
//                            else {
//                                throw FormatError("error formatting format string - missing value for {} (group size and separator character must both be explicitly specified for structured binary representations)", has_custom_precision ? "separator character" : "group size");
//                            }
//                            break;
//                        }
//                        case Formatting::Representation::Hexadecimal: {
//                            // For hexadecimal representations of integer values, the wildcard specifier controls whether to add a base prefix to the result.
//                            bool use_base_prefix = *formatting.wildcard;
//                            if (use_base_prefix) {
//                                builder << "0x";
//                            }
//
//                            bool has_custom_precision = formatting.precision.has_custom_value();
//                            bool has_custom_separator = formatting.separator.has_custom_value();
//
//                            if (has_custom_precision && has_custom_separator) {
//                                unsigned group_size = *formatting.precision;
//                                char separator = *formatting.separator;
//
//                                // Group size must be a power of 2.
//                                bool is_power_of_two = (group_size != 0u) && ((group_size & (group_size - 1u)) == 0u);
//                                if (!is_power_of_two) {
//                                    throw FormatError("error formatting format string - group size must be a power of 2 (received: {})", group_size);
//                                }
//
//                                // If the base prefix is applied, it is separated from the rest of the hexadecimal representation by the separator character.
//                                if (use_base_prefix) {
//                                    builder << separator;
//                                }
//
//                                builder << to_hexadecimal(value, group_size, separator);
//                            }
//                            else if (!has_custom_precision && !has_custom_separator) {
//                                builder << to_hexadecimal(value);
//                            }
//                            else {
//                                throw FormatError("error formatting format string - missing value for {} (group size and separator character must both be explicitly specified for structured hexadecimal representations)", has_custom_precision ? "separator character" : "group size");
//                            }
//                            break;
//                        }
//                        case Formatting::Representation::Scientific: {
//                            if (formatting.separator.has_custom_value()) {
//                            }
//
//                            if (formatting.wildcard.has_custom_value()) {
//                            }
//
//                            // Integer values must be cast to a floating point type to be displayed properly using scientific notation.
//                            builder << std::scientific << std::setprecision(*formatting.precision) << static_cast<long double>(value);
//                            break;
//                        }
//                        case Formatting::Representation::Fixed: {
//                            if (formatting.wildcard.has_custom_value()) {
//                            }
//
//                            if (formatting.separator.has_custom_value()) {
//                                std::string internal = builder.str();
//                                builder.str(std::string()); // Clear stream internals.
//
//                                builder << separate(internal, *formatting.separator);
//                            }
//                            else {
//                                builder << value;
//                            }
//
//                            // Integer values have no decimal places, but this can be faked manually if a fixed precision is requested.
//                            if (formatting.precision.has_custom_value()) {
//                                unsigned precision = *formatting.precision;
//                                if (precision > 0) {
//                                    builder << '.';
//                                    for (unsigned i = 0u; i < precision; ++i) {
//                                        builder << 0;
//                                    }
//                                }
//                            }
//                            break;
//                        }
//                    }
//                }
//                else {
//                    // Floating point type.
//                    switch (representation) {
//                        case Formatting::Representation::Decimal: {
//                            if (formatting.wildcard.has_custom_value()) {
//                            }
//
//                            // Precision is applied by default for floating point numbers.
//                            builder << std::setprecision(*formatting.precision) << value;
//
//                            if (formatting.separator.has_custom_value()) {
//                                std::string internal = builder.str();
//                                builder.str(std::string()); // Clear stream internals.
//
//                                builder << separate(internal, *formatting.separator);
//                            }
//                            break;
//                        }
//                        case Formatting::Representation::Scientific: {
//                            if (formatting.wildcard.has_custom_value()) {
//                            }
//
//                            if (formatting.separator.has_custom_value()) {
//                            }
//
//                            // Precision is applied by default for floating point numbers.
//                            builder << std::scientific << std::setprecision(*formatting.precision) << value;
//                            break;
//                        }
//                        case Formatting::Representation::Fixed: {
//                            if (formatting.wildcard.has_custom_value()) {
//                            }
//
//                            // Precision is applied by default for floating point numbers.
//                            builder << std::fixed << std::setprecision(*formatting.precision) << value;
//
//                            if (formatting.separator.has_custom_value()) {
//                                std::string internal = builder.str();
//                                builder.str(std::string()); // Clear stream internals.
//
//                                builder << separate(internal, *formatting.separator);
//                            }
//                            break;
//                        }
//
//                        // Unsupported representations.
//                        case Formatting::Representation::Binary:
//                        case Formatting::Representation::Hexadecimal:
//                            throw FormatError("error formatting format string - invalid format specifier '{}' ({} representation of floating point numbers is not supported)", to_specifier(representation), to_string(representation));
//                    }
//                }
//            }
//
//            // Apply external format specifiers.
//            return apply_justification(builder.str(), *formatting.justification, *formatting.width, *formatting.fill);
//        }
//
//        template <typename T>
//        [[nodiscard]] std::string container_to_string(const T& container, const Formatting& formatting) {
//            static_assert(is_const_iterable<T>, "container type provided to container_to_string must support const iteration (begin/end).");
//            auto current = std::begin(container);
//            auto end = std::end(container);
//
//            if ((end - current) == 0u) {
//                // Special formatting for when a container has no elements.
//                return "[]";
//            }
//
//            std::string result = "[ ";
//
//            // If custom formatting is specified, it is applied to container elements.
//            result.append(to_string(*current, formatting));
//            for (++current; current != end; ++current) {
//                result.append(", " + to_string(*current, formatting));
//            }
//
//            result.append(" ]");
//
//            return std::move(result);
//        }
//
//        template <typename T>
//        [[nodiscard]] std::string tuple_to_string(const T& value, const Formatting& formatting) {
//            using Type = typename std::decay<T>::type;
//            static_assert(is_pair<Type>::value || is_tuple<Type>::value, "type provided to tuple_to_string must be a tuple type");
//
//            std::string result = "{ ";
//
//            // Use std::apply + fold expression to iterate over and format the elements of the tuple.
//            std::apply([&result, &formatting](const auto&&... args) {
//                ((result.append(stringify(args, formatting) + ", ")), ...);
//            }, value);
//
//            // Overwrite the trailing ", " with " }".
//            std::size_t length = result.length();
//            result[length - 2u] = ' ';
//            result[length - 1u] = '}';
//            return result;
//        }
//
//        template <typename Tuple, std::size_t N = 0>
//        bool is_structured_argument_type(const Tuple& tuple, std::size_t index) {
//            if (N == index) {
//                using Type = typename std::decay<decltype(std::get<N>(tuple))>::type;
//                return is_format_arg<Type>::value;
//            }
//
//            if constexpr (N + 1 < std::tuple_size<Tuple>::value) {
//                return is_structured_argument_type<Tuple, N + 1>(tuple, index);
//            }
//
//            ASSERT(false, "tuple index {} is out of bounds", index);
//            return false;
//        }

    

    }


    template <Container C>
    [[nodiscard]] std::string join(const C& container, std::string_view glue) {
        std::stringstream builder { };
        auto iter = container.begin();

        builder << to_string(*iter);
        for (++iter; iter != container.end(); ++iter) {
            builder << glue;
            builder << to_string(*iter);
        }

        return std::move(builder.str());
    }



    


    
    

    
}

#endif // UTILS_STRING_TPP
