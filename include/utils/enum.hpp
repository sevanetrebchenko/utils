
#ifndef ENUM_HPP
#define ENUM_HPP

#include <type_traits> // std::underlying_type

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

#endif // ENUM_HPP
