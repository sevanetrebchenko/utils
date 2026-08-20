
#ifndef UTILS_CONCEPTS_HPP
#define UTILS_CONCEPTS_HPP

#include "utils/constexpr.hpp"

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
    concept Container = requires(const C container) {
        // Check container iterator
        requires std::is_pointer<decltype(std::begin(container).operator->())>::value;
        // 1. must have valid begin / end function definitions
        { std::begin(container) } -> std::same_as<decltype(std::end(container))>;
        // 2. must be incrementable
        { std::begin(container) } -> std::same_as<decltype(std::next(std::begin(container)))>;
        // 3. must be copy-constructible
        { *std::begin(container) } -> std::copy_constructible;
        // 4. must support comparison operators
        { std::begin(container) == std::begin(container) } -> std::same_as<bool>;
        { std::begin(container) != std::begin(container) } -> std::same_as<bool>;
    };
    
    template <typename T>
    concept String = is_string_type<T>::value;

    // Checks that no two types in a given parameter pack are the same
    template <typename ...Ts>
    inline constexpr bool contains_unique_types = true;

    template <typename First, typename... Rest>
    inline constexpr bool contains_unique_types<First, Rest...> = (!std::is_same_v<First, Rest> && ...) && contains_unique_types<Rest...>;

}

#endif // UTILS_CONCEPTS_HPP
