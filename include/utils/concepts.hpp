
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
        requires std::is_pointer_v<decltype(std::begin(container).operator->())>;
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
    concept String = requires(T str) {
        { is_string_type<T>::value };
    };
    
    template <typename Fn, typename T>
    concept returns_type = requires(const Fn& predicate) {
        { predicate() } -> std::same_as<T>;
    };
    
    template <typename Fn, typename T>
    concept accepts_type = requires(const Fn& predicate) {
        { predicate(std::declval<T>()) };
    };
    
}

#endif // UTILS_CONCEPTS_HPP
