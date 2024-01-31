
#ifndef UTILS_CONCEPTS_HPP
#define UTILS_CONCEPTS_HPP

#include <string> // std::string
#include <type_traits>
#include <iterator>

namespace utils {
    
    // Checks if a type can be converted to a string using operator std::string().
    template <typename T>
    concept is_convertible_to_string = requires(T value) {
        std::string(value);
    };
    
    template <typename C>
    concept is_const_iterable = requires(const C container) {
        // const-incrementable
        { std::begin(container) } -> std::same_as<decltype(std::end(container))>;
        { std::begin(container) } -> std::same_as<decltype(std::next(std::begin(container)))>;
        
        { *std::begin(container) } -> std::copy_constructible;
        requires std::is_pointer_v<decltype(std::begin(container).operator->())>;
        { std::begin(container) == std::begin(container) } -> std::same_as<bool>;
        { std::begin(container) != std::begin(container) } -> std::same_as<bool>;
    };
    
}

#endif // UTILS_CONCEPTS_HPP
