#include "utils/allocator.hpp"
#include <memory>

namespace utils {

    BumpAllocator::BumpAllocator(std::size_t size, float growth_factor) : m_growth_factor(growth_factor),
                                                                          m_current_block_index(0) {
        allocate_block(size);
    }

    BumpAllocator::~BumpAllocator() {
        for (Block& block : m_blocks) {
            free(block.data);
            block.data = nullptr;
        }
    }

    BumpAllocator::BumpAllocator(BumpAllocator&& other) noexcept : m_growth_factor(other.m_growth_factor),
                                                                   m_blocks(std::move(other.m_blocks)),
                                                                   m_current_block_index(other.m_current_block_index) {
        other.m_current_block_index = 0;
    }

    BumpAllocator& BumpAllocator::operator=(BumpAllocator&& other) noexcept {
        if (this == &other) {
            return *this;
        }

        for (Block& block : m_blocks) {
            free(block.data);
            block.data = nullptr;
        }

        m_growth_factor = other.m_growth_factor;
        m_blocks = std::move(other.m_blocks);
        m_current_block_index = other.m_current_block_index;

        other.m_current_block_index = 0;

        return *this;
    }

    void* BumpAllocator::allocate(std::size_t size, std::size_t alignment) {
        while (true) {
            Block& block = m_blocks[m_current_block_index];

            void* data = static_cast<std::byte*>(block.data) + block.size;
            std::size_t remaining = block.capacity - block.size;

            if (std::align(alignment, size, data, remaining)) {
                // Block can fit the (aligned) allocation
                block.size = static_cast<std::size_t>(static_cast<std::byte*>(data) - block.data) + size;
                return data;
            }

            if (m_current_block_index + 1 == m_blocks.size()) {
                // No blocks can accommodate a block of the requested size
                allocate_block(size);
            }

            ++m_current_block_index;
        }
    }

    void BumpAllocator::reset() {
        for (Block& block : m_blocks) {
            block.size = 0;
        }
        m_current_block_index = 0;
    }

    void BumpAllocator::allocate_block(std::size_t size) {
        std::size_t capacity = 1;
        if (!m_blocks.empty()) {
            capacity = m_blocks.back().capacity;
        }

        while (capacity < size) {
            capacity = static_cast<std::size_t>(static_cast<float>(capacity) * m_growth_factor);
        }

        void* data = malloc(capacity);
        m_blocks.emplace_back(data, 0, capacity);
    }

}
