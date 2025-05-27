
#ifndef ALLOCATORS_HPP
#define ALLOCATORS_HPP

#include <vector> // std::vector

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
    
    
    // Hybrid linear allocator that supports tracking non-trivially destructible types
    class LinearAllocator {
        public:
            LinearAllocator(std::size_t size);
            ~LinearAllocator();
            
            template <typename T>
            T* allocate();
            void reset();
        
        private:
            struct AllocationMetadata {
                AllocationMetadata();
                
                union {
                    CopyConstructor copy;
                    MoveConstructor move;
                } copy_constructor;
                Destructor destructor;
                
                // Bit-packed allocation metadata
                unsigned int size : 30;
                enum class CopyConstructorType {
                    None = 0,
                    Copy,
                    Move,
                } copy_op : 2;
            };
            
            void reallocate();
            
            void* m_data;
            std::size_t m_capacity; // Number of available bytes (total)
            std::size_t m_size; // Number of allocations
            std::size_t m_offset;
    };
    
}

#include "utils/detail/allocators.tpp"

#endif // ALLOCATORS_HPP
