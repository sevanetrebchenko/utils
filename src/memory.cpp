
#include "utils/memory.hpp"

namespace utils {
    
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