
#ifndef HASH_HPP
#define HASH_HPP

#include <cstddef> // std::size_t

namespace utils {
    
    template <typename T>
    void hash_combine(std::size_t& seed, const T& value);
    
    void hash_combine(std::size_t& seed, const char* value);
    void hash_combine(std::size_t& seed, const char* value, std::size_t length);
    
}

#include "utils/detail/hash.tpp"

#endif // HASH_HPP
