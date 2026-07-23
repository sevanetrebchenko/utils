
#ifndef UTILS_MEMORY_HPP
#define UTILS_MEMORY_HPP

#include <cstddef> // std::size_t
#include <utility> // std::move

namespace utils {
    
    using Destructor = void (*)(void* obj);
    using CopyConstructor = void (*)(void* dst, const void* src);
    using MoveConstructor = void (*)(void* dst, void* src);
    
    template <typename T>
    constexpr Destructor get_destructor();
    
    template <typename T>
    constexpr CopyConstructor get_copy_constructor();
    
    template <typename T>
    constexpr MoveConstructor get_move_constructor();
    
    constexpr std::size_t bytes(std::size_t b) {
        return b;
    }

    constexpr std::size_t kilobytes(std::size_t kb) {
        return kb * 1024;
    }

    constexpr std::size_t megabytes(std::size_t mb) {
        return mb * 1024 * 1024;
    }
    
}

#include "utils/detail/memory.tpp"

#endif // UTILS_MEMORY_HPP
