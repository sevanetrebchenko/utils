
#ifndef HASH_TPP
#define HASH_TPP

#include <string_view> // std::string_view

namespace utils {

    template <typename T>
    void hash_combine(std::size_t& seed, const T& value) {
        std::size_t hash;
        
        if constexpr (std::is_same<typename std::decay<T>::type, char*>::value) {
            hash = std::hash<std::string_view>{}(value, strlen(value));
        }
        else {
            hash = std::hash<T>{}(value);
        }
        
        // Inspired by Boost
        seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

}

#endif // HASH_TPP
