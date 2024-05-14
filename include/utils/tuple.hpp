
#ifndef TUPLE_HPP
#define TUPLE_HPP

#include "utils/concepts.hpp"
#include "utils/format.hpp"

#include <cstddef> // std::size_t
#include <tuple> // std::tuple

namespace utils {
    
    template <typename T, typename Fn, std::size_t N = 0>
    auto runtime_get(const T& tuple, std::size_t index, const Fn& fn);
    
    template <typename Fn, typename ...Ts>
    void apply(const Fn& fn, const std::tuple<Ts...>& tup);
    
    template <typename Fn, typename ...Ts>
    void apply(const Fn& fn, const std::tuple<Ts...>& tup, std::size_t index);
    
}

#include "utils/detail/tuple.tpp"

#endif // TUPLE_HPP
