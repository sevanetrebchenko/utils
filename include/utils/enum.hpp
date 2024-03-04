
#ifndef UTILS_ENUM_HPP
#define UTILS_ENUM_HPP

#include <type_traits>

#define DEFINE_ENUM_BITFIELD_OPERATIONS(T)                                      \
    [[nodiscard]] inline constexpr T operator|(T first, T second) {             \
        using U = std::underlying_type<T>::type;                                \
        return static_cast<T>(static_cast<U>(first) | static_cast<U>(second));  \
    }                                                                           \
    inline constexpr T& operator|=(T& target, T value) {                        \
        return target = target | value;                                         \
    }                                                                           \
    [[nodiscard]] inline constexpr T operator&(T first, T second) {             \
        using U = std::underlying_type<T>::type;                                \
        return static_cast<T>(static_cast<U>(first) & static_cast<U>(second));  \
    }                                                                           \
    inline constexpr T& operator&=(T& target, T value) {                        \
        return target = target & value;                                         \
    }                                                                           \
    [[nodiscard]] inline constexpr T operator^(T first, T second) {             \
        using U = std::underlying_type<T>::type;                                \
        return static_cast<T>(static_cast<U>(first) ^ static_cast<U>(second));  \
    }                                                                           \
    inline constexpr T& operator^=(T& target, T value) {                        \
        return target = target ^ value;                                         \
    }                                                                           \
    [[nodiscard]] inline constexpr T operator~(T value) {                       \
        using U = std::underlying_type<T>::type;                                \
        return static_cast<T>(~static_cast<U>(value));                          \
    }                                                                           \
    [[nodiscard]] inline constexpr bool test(T target, T value) {               \
        return (target & value) == value;                                       \
    }

namespace utils {
}


#endif // UTILS_ENUM_HPP
