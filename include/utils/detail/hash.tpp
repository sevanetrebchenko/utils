
#ifndef UTILS_HASH_TPP
#define UTILS_HASH_TPP

#include <string_view> // std::string_view

namespace std {

    // std::hash is not required by the standard to support member pointer types
    // This partial specialization covers both data member and member function pointers
    template <typename R, typename C>
    struct hash<R C::*> {
        std::size_t operator()(R C::* const& ptr) const noexcept {
            std::size_t result = 0;
            const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&ptr);
            for (std::size_t i = 0; i < sizeof(ptr); ++i) {
                utils::hash_combine(result, bytes[i]);
            }
            return result;
        }
    };

}

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

#endif  // UTILS_HASH_TPP
