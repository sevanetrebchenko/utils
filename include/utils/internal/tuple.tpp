
#ifndef UTILS_TUPLE_TPP
#define UTILS_TUPLE_TPP

#include <cstddef> // std::size_t
#include <tuple> // std::tuple

namespace utils {
    namespace internal {
        
        template <typename T, typename F, std::size_t N = 0>
        auto for_each_helper(const T& tuple, const F& fn) {
            if constexpr (N + 1 < std::tuple_size_v<T>) {
                return fn(std::get<N>(tuple)) || for_each_helper<T, F, N + 1>(tuple, fn);
            }
            else {
                return fn(std::get<N>(tuple));
            }
        }
        
    }
    
    template <typename T, typename F>
    auto for_each(const T& tuple, const F& fn) {
        return internal::for_each_helper(tuple, fn);
    }
    
    
}

#endif // UTILS_TUPLE_TPP
