
#ifndef ALLOCATORS_TPP
#define ALLOCATORS_TPP

#include "utils/bits.hpp"
#include <utility> // std::move

namespace utils {

    template <typename T>
    constexpr Destructor get_destructor() {
        return +[](void* obj) -> void {
            static_cast<T*>(obj)->~T();
        };
    }
    
    template <typename T>
    constexpr CopyConstructor get_copy_constructor() {
        return +[](void* dest, const void* src) {
            new (dest) T(*static_cast<const T*>(src));
        };
    }
    
    template <typename T>
    constexpr MoveConstructor get_move_constructor() {
        return +[](void* dest, void* src) {
            new (dest) T(std::move(*static_cast<T*>(src)));
        };
    }
    
    template <typename T>
    T* LinearAllocator::allocate() {
        std::size_t block_size = sizeof(AllocationMetadata) + sizeof(T);
        std::size_t available_size = m_capacity - m_offset;
        if (available_size < block_size) {
            // Allocator is out of memory, need to reallocate the internal data store
            reallocate();
        }
        
        void* block = reinterpret_cast<std::byte*>(m_data) + m_offset;
        m_offset += block_size;
        
        // Initialize metadata block
        AllocationMetadata& allocation = *reinterpret_cast<AllocationMetadata*>(block);
        
        if constexpr (!std::is_trivially_destructible<T>::value && std::is_destructible<T>::value) {
            // Hybrid allocator automatically stores destructors for non-trivially destructible types
            allocation.destructor = get_destructor<T>();
        }
        // Prefer move over copy constructor, if available
        // Fallback to copy constructor
        if constexpr (!std::is_trivially_copy_constructible<T>::value && std::is_copy_constructible<T>::value) {
            allocation.copy_constructor.copy = get_copy_constructor<T>();
            allocation.copy_op = AllocationMetadata::CopyConstructorType::Copy;
        }
        if constexpr (!std::is_trivially_move_constructible<T>::value && std::is_move_constructible<T>::value) {
            allocation.copy_constructor.move = get_move_constructor<T>();
            allocation.copy_op = AllocationMetadata::CopyConstructorType::Move;
        }
        
        allocation.size = block_size;
        
        // Return the base of the data block
        return reinterpret_cast<std::byte*>(block) + sizeof(AllocationMetadata);
    }

}

#endif // ALLOCATORS_TPP