
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
        
        class Placeholder {
            public:
                class Identifier {
                    public:
                        enum Type {
                            None = 0,
                            Position,
                            Name
                        };
        
                        Identifier();
                        explicit Identifier(std::string_view identifier);
                        static Result<Identifier> parse(std::string_view identifier) noexcept;
                        ~Identifier();
        
                        Type type;
                        int position; // Allows for ~2 billion unique positions in a single format string.
                        std::string name;
                        
                    private:
                        explicit Identifier(int position);
                        explicit Identifier(std::string name);
                };
                
                struct Formatting {
                    enum Justification {
                        Right = 0,
                        Left,
                        Center
                    };
                    
                    enum Representation {
                        Decimal = 0,
                        Binary,
                        Unicode,
                        Octal,
                        Hexadecimal
                    };
                    
                    enum Sign {
                        NegativeOnly = 0,
                        Aligned,
                        Both
                    };
                    
                    explicit Formatting(std::string_view specifiers);
                    static Result<Formatting> parse(std::string_view specifiers) noexcept;
                    
                    Formatting();
                    ~Formatting();
                
                    Justification justification;
                    Representation representation;
                    Sign sign;
                    char fill;
                    char separator;
                    unsigned width;
                    unsigned precision;
                };
                
                Placeholder(std::string_view placeholder);
                static Result<Placeholder> parse(std::string_view placeholder) noexcept;
                
                Placeholder();
                ~Placeholder();
                
                Identifier identifier;
                Formatting formatting;
        };
        
        struct FormatString {
            FormatString() {}
            
            std::vector<Placeholder> placeholders;
        };
        
        [[nodiscard]] Result<FormatString> parse_format_string(const std::string& format_string);
        
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
