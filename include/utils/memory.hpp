
#ifndef MEMORY_HPP
#define MEMORY_HPP

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
    
    constexpr std::size_t bytes(std::size_t b);
    constexpr std::size_t kilobytes(std::size_t kb);
    constexpr std::size_t megabytes(std::size_t mb);
    
}

#include "utils/detail/memory.tpp"

#endif // MEMORY_HPP
