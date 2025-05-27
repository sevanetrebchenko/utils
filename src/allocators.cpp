
#include "utils/allocators.hpp"
#include "utils/bits.hpp"

namespace utils {
    
    LinearAllocator::LinearAllocator(std::size_t size) : m_data(malloc(size)),
                                                         m_capacity(size),
                                                         m_size(0),
                                                         m_offset(0) {
    }
    
    LinearAllocator::~LinearAllocator() {
        free(m_data);
    }
    
    void LinearAllocator::reset() {
        std::byte* block = reinterpret_cast<std::byte*>(m_data);
        
        for (std::size_t i = 0; i < m_size; ++i) {
            const AllocationMetadata& allocation = *reinterpret_cast<AllocationMetadata*>(block);
            
            if (allocation.destructor) {
                // Non-trivially destructible type, need to call destructor for this object before resetting allocator
                allocation.destructor(block + sizeof(AllocationMetadata));
            }
            
            // Step to next block
            block += sizeof(AllocationMetadata) + allocation.size;
        }
        
        // Reset allocator internals
        m_size = 0;
        m_offset = 0;
    }
    
    void LinearAllocator::reallocate() {
        // Increase allocator capacity
        m_capacity *= 2;
        void* new_data = malloc(m_capacity);
        
        std::byte* dst = reinterpret_cast<std::byte*>(new_data);
        std::byte* src = reinterpret_cast<std::byte*>(m_data);
        std::byte* last_copied_from = src;
        
        // Copy over elements + allocation metadata
        // Consecutive allocations that are trivially copyable are copied over in bulk
        for (std::size_t i = 0; i < m_size; ++i) {
            const AllocationMetadata& allocation = *reinterpret_cast<AllocationMetadata*>(src);
            std::size_t total_size = sizeof(AllocationMetadata) + allocation.size;
            
            bool is_trivial = allocation.copy_op == AllocationMetadata::CopyConstructorType::None;
            
            if (!is_trivial) {
                // Bulk copy any trivially copyable elements before this allocation
                if (src != last_copied_from) {
                    std::size_t block_size = src - last_copied_from;
                    std::memcpy(dst, last_copied_from, block_size);
                    dst += block_size;
                }
                
                // Copy allocation header
                std::memcpy(dst, src, sizeof(AllocationMetadata));
                
                // Copy block
                std::size_t offset = sizeof(AllocationMetadata);
                switch (allocation.copy_op) {
                    case AllocationMetadata::CopyConstructorType::Copy:
                        allocation.copy_constructor.copy(dst + offset, src + offset);
                        break;
                    case AllocationMetadata::CopyConstructorType::Move:
                        allocation.copy_constructor.move(dst + offset, src + offset);
                        break;
                }
                
                dst += total_size;
                last_copied_from = src + total_size;
            }
            // Simply step over trivially-copyable blocks, as batch memcpy will copy the data of this block later
            // else { ... }
            
            src += total_size;
        }
        
        // Copy any trailing trivially-copyable blocks
        if (src != last_copied_from) {
            std::size_t block_size = src - last_copied_from;
            std::memcpy(dst, last_copied_from, block_size);
            // dst += block_size;
        }
        
        free(m_data);
        m_data = new_data;
    }
    
    LinearAllocator::AllocationMetadata::AllocationMetadata() : copy_constructor(nullptr),
                                                                destructor(nullptr),
                                                                size(0),
                                                                copy_op(CopyConstructorType::None) {
    }
    
}