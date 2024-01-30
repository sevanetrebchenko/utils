
#ifndef UTILS_CONCEPTS_HPP
#define UTILS_CONCEPTS_HPP

#include <string> // std::string

namespace utils {
    
    // Checks if a type can be converted to a string using operator std::string().
    template <typename T>
    concept convertible_to_string = requires(T value) {
        std::string(value);
    };
    
}

#endif // UTILS_CONCEPTS_HPP
