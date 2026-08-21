
#ifndef UTILS_ALLOCATOR_HPP
#define UTILS_ALLOCATOR_HPP

#include "utils/memory.hpp"
#include <cstddef>
#include <vector>

namespace utils {

    // Arena bump allocator
    class BumpAllocator {
        public:
            // Accepts the initial block size to use
            explicit BumpAllocator(std::size_t size = kilobytes(4), float growth_factor = 2.0f);
            ~BumpAllocator();

            // Bump allocator should not be copied, only moved
            BumpAllocator(const BumpAllocator&) = delete;
            BumpAllocator& operator=(const BumpAllocator&) = delete;

            BumpAllocator(BumpAllocator&& other) noexcept;
            BumpAllocator& operator=(BumpAllocator&& other) noexcept;

            [[nodiscard]] void* allocate(std::size_t size, std::size_t alignment);
            void reset();

        private:
            struct Block {
                void* data;
                std::size_t size;  // Current allocation size
                std::size_t capacity;
            };

            // Appends a new block sized to fit at least 'size', continuing the geometric growth using the size of the last block allocated
            void allocate_block(std::size_t size);

            float m_growth_factor;
            std::vector<Block> m_blocks;
            std::size_t m_current_block_index;  // Index into m_blocks currently being allocated from
    };

}

#endif // UTILS_ALLOCATOR_HPP
