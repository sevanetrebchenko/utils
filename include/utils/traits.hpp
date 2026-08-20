
#ifndef UTILS_TRAITS_HPP
#define UTILS_TRAITS_HPP

#include <tuple> // std::tuple

namespace utils {

    // Extracts a callable's return type and parameter types
    // For use with lambdas, member function pointers, and free functions / function pointers
    template <typename T>
    struct FunctionTraits : FunctionTraits<decltype(&T::operator())> { };

    template <typename C, typename R, typename ...Args>
    struct FunctionTraits<R(C::*)(Args...)> {
        using ReturnType = R;
        using ClassType = C;
        using ArgsTuple = std::tuple<Args...>;
    };

    template <typename C, typename R, typename ...Args>
    struct FunctionTraits<R(C::*)(Args...) const> {
        using ReturnType = R;
        using ClassType = C;
        using ArgsTuple = std::tuple<Args...>;
    };

    template <typename R, typename ...Args>
    struct FunctionTraits<R(Args...)> {
        using ReturnType = R;
        using ArgsTuple = std::tuple<Args...>;
    };

    template <typename R, typename ...Args>
    struct FunctionTraits<R(*)(Args...)> {
        using ReturnType = R;
        using ArgsTuple = std::tuple<Args...>;
    };

}

#endif // UTILS_TRAITS_HPP
