
#ifndef UTILS_HASH_HPP
#define UTILS_HASH_HPP

#include <cstddef> // std::size_t

namespace utils {
    
    template <typename T>
    void hash_combine(std::size_t& seed, const T& value);
    
    void hash_combine(std::size_t& seed, const char* value);
    void hash_combine(std::size_t& seed, const char* value, std::size_t length);

    // FNV-1a string hash
    constexpr std::uint64_t hash_fnv1a(std::string_view name) {
        std::uint64_t hash = 14695981039346656037ull;
        for (const char c : name) {
            hash ^= static_cast<std::uint64_t>(static_cast<unsigned char>(c));
            hash *= 1099511628211ull;
        }
        return hash;
    }
    
}

#include "utils/detail/hash.tpp"

#endif // UTILS_HASH_HPP
