
#ifndef UTILS_HASH_HPP
#define UTILS_HASH_HPP

#include <cstddef> // std::size_t

namespace utils {
    
    template <typename T>
    void hash_combine(std::size_t& seed, const T& value);
    
    void hash_combine(std::size_t& seed, const char* value);
    void hash_combine(std::size_t& seed, const char* value, std::size_t length);
    
}

#include "utils/detail/hash.tpp"

#endif // UTILS_HASH_HPP
