
#ifndef UTILS_STRING_TPP
#define UTILS_STRING_TPP

#include "utils/constexpr.hpp"
#include "utils/concepts.hpp"
#include "utils/result.hpp"
#include <functional>
#include <cstdio> // std::snprintf
#include <memory> // std::unique_ptr
#include <stdexcept> // std::runtime_error
#include <tuple> // std::tuple_cat, std::make_tuple
#include <regex> // std::regex_match

namespace utils {

    namespace internal {
    
        template <typename T>
        [[nodiscard]] std::string pointer_to_string(T pointer) {
            using Type = typename std::remove_reference<typename std::remove_cv<T>::type>::type;
            static_assert(std::is_pointer<Type>::value || std::is_null_pointer<Type>::value, "non-pointer type provided to pointer_to_string");
            
            static char buffer[2u * sizeof(void*) + 3u] { '\0' }; // Enough space to store a pointer address + an optional '0x' prefix (2 bytes) + null terminator (1 byte).
            static std::size_t buffer_size = sizeof(buffer) / sizeof(buffer[0]);
            
            int num_characters = std::snprintf(buffer, buffer_size, "%p", static_cast<void*>(pointer));
            if (num_characters < 0) {
                throw std::runtime_error("encoding error in pointer_to_string");
            }
            
            // The inclusion of the '0x' prefix is implementation dependent, and not all compilers may include it.
            if (buffer[0] == '0' && buffer[1] == 'x') {
                return { buffer, static_cast<std::size_t>(num_characters) };
            }
            else {
                return "0x" + std::string(buffer, static_cast<std::size_t>(num_characters));
            }
        }
        
        template <typename T>
        [[nodiscard]] std::string stringify(const T& value) {
            using Type = typename std::remove_reference<typename std::remove_cv<T>::type>::type;
            
            if constexpr (std::is_fundamental<Type>::value) {
                if constexpr (std::is_null_pointer<Type>::value) {
                    // std::nullptr_t (nullptr)
                    return pointer_to_string(value);
                }
                else if constexpr (std::is_same<Type, bool>::value) {
                    return value ? "true" : "false";
                }
                else {
                    // TODO: other fundamental types.
                    return "";
                }
            }
            else if constexpr (is_standard_container<Type>::value) {
                // Standard container types.
                auto iter = std::begin(value);
                auto end = std::end(value);
                
                if ((end - iter) == 0u) {
                    // Container has no elements.
                    return "[ ]";
                }
                
                std::string result = "[ ";
                result.append(stringify(*iter));
                for (++iter; iter != end; ++iter) {
                    result.append(", " + stringify(*iter));
                }
                result.append(" ]");

                return std::move(result);
            }
            else if constexpr (is_pair<Type>::value) {
                // std::pair
                std::string result = "{ ";
                result.append(stringify(value.first));
                result.append(", ");
                result.append(stringify(value.second));
                result.append(" }");
                return std::move(result);
            }
            else if constexpr (is_tuple<Type>::value) {
                // std::tuple
                std::string result = "{ ";
                
                // Use std::apply + fold expression to iterate over and format the elements of the tuple.
                std::apply([&result](auto&&... args) {
                    ((result.append(stringify(args) + ", ")), ...);
                }, value);
                
                // Overwrite the trailing ", " with " }".
                std::size_t length = result.length();
                result[length - 2u] = ' ';
                result[length - 1u] = '}';
                
                return std::move(result);
            }
            else if constexpr (std::is_same<Type, std::string>::value) {
                // std::string
                return "\"" + value + "\"";
            }
            else if constexpr (std::is_pointer<Type>::value) {
                // pointer types
                return pointer_to_string(value);
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
        
        template <typename T, typename ...Rest>
        auto to_string_tuple(const T& value, const Rest&... rest) {
            return std::tuple_cat(std::make_tuple(stringify(value)), internal::to_string_tuple<Rest>(rest)...);
        }
        
        struct FormatString {
            FormatString() {}
            std::string str;
        };

        struct TypeFormatArgs {
            // Justification
            bool justify_left;
            bool justify_right;
            bool justify_center;
            
        };
        
        inline Result<TypeFormatArgs> parse_placeholder_format_specifiers(const std::string& specifiers) {
            return Result<TypeFormatArgs> { };
        }
        
        
        inline Result<FormatString> parse_format_string(const std::string& format_string) {
            Result<FormatString> result { };
            
            // To save on processing power, resulting string is only updated when a placeholder is encountered.
            
            bool processing_placeholder = false;
            std::size_t placeholder_start;
            processing_placeholder = false;
            
            std::size_t position = 0u;
            
            for (std::size_t i = 0u; i < format_string.length(); ++i) {
                if (processing_placeholder) {
                    if (format_string[i] == '}') {
                        // Parse the placeholder without the starting or ending braces.
                        std::string placeholder = format_string.substr(placeholder_start + 1u, i - placeholder_start - 1u);
                        
                        if (placeholder.empty() || placeholder[0] == ':') {
                            // Detected auto-numbered placeholder - {}.
                            // Note: auto-numbered placeholders may still contain format specifiers for type formatting.
                            
                        }
                        else {
                            // A placeholder should not have any whitespace characters.
                            for (std::size_t j = 0u; j < placeholder.length(); ++j) {
                                if (std::isspace(placeholder[j])) {
                                    return Result<FormatString>::NOT_OK("error while processing placeholder '{{{}}}' (whitespace character at position {})", placeholder, j);
                                }
                            }
                            
                            // Determine if placeholder is positional or named.
                            std::vector<std::string> components = split(placeholder, ":");
                            if (std::regex_match(components[0], std::regex("^[0-9]+$"))) {
                                // Positional placeholders can only be positive integers.
                            }
                            else if (std::regex_match(components[0], std::regex("^[a-zA-Z_]\\w*$"))) {
                                // Named placeholders follow the same naming convention as C++ identifiers:
                                //  - start with a letter or underscore
                                //  - followed by any combination of letters, digits, or underscores (\w)
                            }
                            else {
                                return Result<FormatString>::NOT_OK("error while processing placeholder '{{{}}}' (placeholder name '{}' is not valid)", placeholder, components[0]);
                            }
                        }
                    }
                }
                else {
                    if (format_string[i] == '{') {
                        if (i != format_string.length() - 1u && format_string[i + 1u] == '{') {
                            // Escaped '{' character.
                        }
                        else {
                            processing_placeholder = true;
                            placeholder_start = i;
                        }
                    }
                    else if (format_string[i] == '}') {
                        if (i == 0u) {
                            return Result<FormatString>::NOT_OK("");
                        }
                        else if (format_string[i - 1u] == '}') {
                            // Escaped '}' character.
                        }
                    }
                }
            }
            
            return Result<FormatString> { };
        }
        
    }
    
    
    template <typename Container>
    [[nodiscard]] std::string join(const Container& container, const std::string& glue) {
        // TODO: support custom containers with iterators
//        static_assert(is_const_iterable<Container>, "container must support std::begin/std::end");
        
        auto iter = std::begin(container);
        std::string result;

        result.append(internal::stringify(*iter));
        for (++iter; iter != std::end(container); ++iter) {
            result.append(glue);
            result.append(internal::stringify(*iter));
        }

        return std::move(result);
    }

    template <typename T>
    arg::arg(std::string name, const T& value) : name(std::move(name)),
                                                 value(internal::stringify(value)) {
    }
    
    template <typename ...Ts>
    std::string format(const std::string& fmt, const Ts&... ts) {
        using namespace internal;
        
        Result<FormatString> format_string = parse_format_string(fmt);
        if (!format_string.ok()) {
            throw std::runtime_error(format_string.what());
        }
        
        auto formatted = internal::to_string_tuple<Ts...>(ts...);
        return "";
    }
    
}

#endif // UTILS_STRING_TPP
