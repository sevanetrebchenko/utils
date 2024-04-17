
#ifndef UTILS_TUPLE_HPP
#define UTILS_TUPLE_HPP

#include "utils/concepts.hpp"

#include <cstddef> // std::size_t
#include <tuple> // std::tuple
#include <functional> // std::function
#include <stdexcept> // std::out_of_range

namespace utils {
    
    template <typename Tuple, typename Fn, std::size_t N = 0>
    auto runtime_get(const Tuple& tuple, std::size_t index, const Fn& fn) {
        static_assert(N < std::tuple_size<Tuple>::value);
        
        if (N == index) {
            return fn(std::get<N>(tuple));
        }
        
        if constexpr (N + 1 < std::tuple_size<Tuple>::value) {
            return runtime_get<Tuple, Fn, N + 1>(tuple, index, fn);
        }
        
        throw std::out_of_range(format("invalid tuple index {} provided to runtime_get", index));
    }
    
    template <typename T, typename F>
    [[nodiscard]] bool for_each(const T& tuple, const F& fn) requires returns_type<F, bool>;
    
    
}

#include "utils/detail/tuple.tpp"

#endif // UTILS_TUPLE_HPP
