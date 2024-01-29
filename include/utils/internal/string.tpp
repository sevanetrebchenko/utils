
#ifndef UTILS_STRING_TPP
#define UTILS_STRING_TPP

#include "utils/constexpr.hpp"
#include <functional>
#include <cstdio> // std::snprintf
#include <charconv> // std::to_chars
#include <memory> // std::unique_ptr
#include <stdexcept> // std::runtime_error
#include <tuple> // std::tuple_cat, std::make_tuple

namespace utils {

    namespace internal {
    
        template <typename T>
        [[nodiscard]] std::string pointer_to_string(T pointer) {
            using Type = typename std::remove_reference<typename std::remove_cv<T>::type>::type;
            static_assert(std::is_pointer<Type>::value || std::is_null_pointer<Type>::value, "non-pointer type provided to pointer_to_string");
            
            static std::unique_ptr<char[]> buffer = nullptr;
            static std::size_t buffer_size = 0u;
            if (buffer_size == 0u) {
                int num_characters = std::snprintf(nullptr, 0u, "0x%p", static_cast<void*>(pointer)); // Guaranteed to be the same during program execution.
                if (num_characters < 0) {
                    throw std::runtime_error("encoding error (initial allocation) pointer_to_string");
                }
                
                buffer_size = static_cast<std::size_t>(num_characters) + 1u; // Account for null terminator.
                buffer = std::unique_ptr<char[]>(new char[buffer_size]);
            }
            
            int num_characters = std::snprintf(buffer.get(), buffer_size, "0x%p", static_cast<void*>(pointer));
            if (num_characters < 0) {
                throw std::runtime_error("encoding error in pointer_to_string");
            }
            
            return { buffer.get(), static_cast<std::size_t>(num_characters) };
        }
        
        template <typename T>
        [[nodiscard]] std::string to_string(const T& value) {
            using Type = typename std::remove_reference<typename std::remove_cv<T>::type>::type;
            
            if constexpr (std::is_fundamental<Type>::value) {
                if constexpr (std::is_null_pointer<Type>::value) {
                    // std::nullptr_t (nullptr)
                    // Note: passing as void* instead of std::nullptr_t
                    return pointer_to_string(nullptr);
                }
                else {
                    // fundamental types (boolean, character, integer, floating point)
                    
                    // TODO: doesn't work for boolean types.
                    
                    char buffer[32] = { '\0' };
                    auto [p, error_code] = std::to_chars(buffer, buffer + sizeof(buffer) / sizeof(buffer[0]), value);
                    return std::string(buffer, p);
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
                result.append(to_string(*iter));
                for (++iter; iter != end; ++iter) {
                    result.append(", " + to_string(*iter));
                }
                result.append(" ]");

                return std::move(result);
            }
            else if constexpr (is_pair<Type>::value) {
                // std::pair
                std::string result = "{ ";
                result.append(to_string(value.first));
                result.append(", ");
                result.append(to_string(value.second));
                result.append(" }");
                return std::move(result);
            }
            else if constexpr (is_tuple<Type>::value) {
                // std::tuple
                std::string result = "{ ";
                
                // Use std::apply + fold expression to iterate over and format the elements of the tuple.
                std::apply([&result](auto&&... args) {
                    ((result.append(to_string(args) + ", ")), ...);
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
            else {
                // Utilize T::operator std::string() for all other custom types.
                return std::string(value);
            }
        }
        
        template <typename T, typename ...Rest>
        auto to_string_tuple(const T& value, const Rest&... rest) {
            return std::tuple_cat(std::make_tuple(to_string(value)), internal::to_string_tuple<Rest>(rest)...);
        }
        
    }
    
    template <typename Container>
    [[nodiscard]] std::string join(const Container& components, const std::string& glue) {
        static_assert(std::is_same<typename std::decay<decltype(*std::begin(std::declval<Container>()))>::type, std::string>::value, "container must be of string type");
        if (empty(components)) {
            return "";
        }
    
        auto iter = components.begin();
        
        std::string result;
        result.append(*iter);
        for (++iter; iter != components.end(); ++iter) {
            result.append(glue);
            result.append(*iter);
        }
        
        return std::move(result);
    }

    template <typename T>
    arg::arg(std::string name, const T& value) : name(std::move(name)),
                                                 value(internal::to_string(value)) {
    }
    
    template <typename ...Ts>
    std::string format(const std::string& format, const Ts&... ts) {
        auto formatted = internal::to_string_tuple<Ts...>(ts...);
        return "";
    }
    
}

#endif // UTILS_STRING_TPP
