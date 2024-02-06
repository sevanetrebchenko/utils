
#ifndef UTILS_TUPLE_HPP
#define UTILS_TUPLE_HPP

#include <cstddef> // std::size_t
#include <tuple> // std::tuple
#include <functional> // std::function
#include <stdexcept> // std::
#include <iostream>

namespace utils {
    
    template <typename T, typename F, std::size_t N = 0>
    auto get(const T& tuple, std::size_t index, const F& fn) {
        if (N == index) {
            return fn(std::get<N>(tuple));
        }
        
        if constexpr (N + 1 < std::tuple_size_v<T>) {
            return get<T, F, N + 1>(tuple, index, fn);
        }
        
        throw std::out_of_range(format("tuple index {} out of range", index));
    }
    
    
    template <typename T, typename F>
    auto for_each(const T& tuple, const F& fn);
    
    
    template <typename T, typename Tuple, typename F, std::size_t N = 0>
    void get_type(const Tuple& tuple, const F& fn) {
        const auto& value = std::get<N>(tuple);
        
        if constexpr (std::is_same_v<std::decay_t<decltype(value)>, T>) {
            fn(value);
        }
        
        if constexpr (N + 1 < std::tuple_size_v<Tuple>) {
            get_type<T, Tuple, F, N + 1>(tuple, fn);
        }
    }
    
    
    template <typename T, typename Tuple, std::size_t N = 0>
    std::size_t count_occurrences(const Tuple& tuple) {
        std::size_t count = 0u;
        if constexpr (std::is_same_v<std::decay_t<decltype(std::get<N>(tuple))>, T>) {
            ++count;
        }
        
        if constexpr (N + 1 < std::tuple_size_v<Tuple>) {
            return count + count_occurrences<T, Tuple, N + 1>(tuple);
        }
        else {
            return count;
        }
    }
    
    
}

#include "utils/internal/tuple.tpp"

#endif // UTILS_TUPLE_HPP
