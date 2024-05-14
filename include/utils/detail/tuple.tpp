
#ifndef TUPLE_TPP
#define TUPLE_TPP

#include "utils/format.hpp"

#include <functional>

namespace utils {

    namespace detail {
        
        // Concepts to detect callable signatures
        
        // Concept for default use: Callback takes each element of the tuple
        template <typename Fn, typename T>
        concept InvocableWithValue = requires(Fn fn, T value) {
            { fn(value) };
        };
        
        // Concept for index use: Callback takes each element of the tuple and its runtime index
        template <typename Fn, typename T>
        concept InvocableWithIndex = requires(Fn fn, T value, std::size_t index) {
            { fn(value, index) };
        };
        
        // Concept for compile-time index use: Callback takes each element of the tuple and its compile-time index as a template parameter
        template <typename Fn, typename T, std::size_t I>
        concept InvocableWithCompileTimeIndex = requires(Fn fn, T value) {
            { fn.template operator()<T, I>(value) };
        };
        
        template <typename Fn, typename... Ts, std::size_t... I>
        inline void apply_impl(const Fn& fn, const std::tuple<Ts...>& tup, std::index_sequence<I...>) requires (InvocableWithValue<Fn, decltype(std::get<I>(tup))> && ...) {
            (fn(std::get<I>(tup)), ...);
        }
        
        template <typename Fn, typename ...Ts, std::size_t... I>
        void apply_impl(const Fn& fn, const std::tuple<Ts...>& tup, std::size_t index, std::index_sequence<I...>) requires (InvocableWithValue<Fn, decltype(std::get<I>(tup))> && ...)  {
            bool executed = ((index == I && (fn(std::get<I>(tup)), true)) || ...);
            if (!executed) {
                throw std::out_of_range("Index out of range");
            }
        }
        
        template <typename Fn, typename... Ts, std::size_t... I>
        inline void apply_impl(const Fn& fn, const std::tuple<Ts...>& tup, std::index_sequence<I...>) requires (InvocableWithIndex<Fn, decltype(std::get<I>(tup))> && ...) {
            (fn(std::get<I>(tup), I), ...);
        }
        
        template <typename Fn, typename ...Ts, std::size_t... I>
        void apply_impl(const Fn& fn, const std::tuple<Ts...>& tup, std::size_t index, std::index_sequence<I...>) requires (InvocableWithIndex<Fn, decltype(std::get<I>(tup))> && ...)  {
            bool executed = ((index == I && (fn(std::get<I>(tup, I)), true)) || ...);
            if (!executed) {
                throw std::out_of_range("Index out of range");
            }
        }
        
        template <typename Fn, typename... Ts, std::size_t... I>
        inline void apply_impl(const Fn& fn, const std::tuple<Ts...>& tup, std::index_sequence<I...>) requires (InvocableWithCompileTimeIndex<Fn, decltype(std::get<I>(tup)), I> && ...) {
            (fn.template operator()<Ts, I>(std::get<I>(tup)), ...);
        }
        
        template <typename Fn, typename ...Ts, std::size_t... I>
        void apply_impl(const Fn& fn, const std::tuple<Ts...>& tup, std::size_t index, std::index_sequence<I...>) requires (InvocableWithCompileTimeIndex<Fn, decltype(std::get<I>(tup)), I> && ...) {
            bool executed = (((index == I ? (fn.template operator()<Ts, I>(std::get<I>(tup)), true) : false)) || ...);
            if (!executed) {
                throw std::out_of_range("Index out of range");
            }
        }
        
    }
    
    template <typename T, typename Fn, std::size_t N>
    auto runtime_get(const T& tuple, std::size_t index, const Fn& fn) {
        constexpr std::size_t size = std::tuple_size<T>::value;
        static_assert(N < size);
        
        if (N == index) {
            return fn(std::get<N>(tuple));
        }
        
        if constexpr (N + 1 < size) {
            return runtime_get<T, Fn, N + 1>(tuple, index, fn);
        }
        else {
            throw FormattedError("tuple index {} is out of bounds", index);
        }
    }
    
    template <typename Fn, typename ...Ts>
    void apply(const Fn& fn, const std::tuple<Ts...>& tup) {
        detail::apply_impl(fn, tup, std::index_sequence_for<Ts...> { });
    }
    
    template <typename Fn, typename ...Ts>
    void apply(const Fn& fn, const std::tuple<Ts...>& tup, std::size_t index) {
        detail::apply_impl(fn, tup, index, std::index_sequence_for<Ts...> { });
    }
    
}

#endif // TUPLE_TPP
