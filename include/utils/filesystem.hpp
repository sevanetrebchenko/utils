
#ifndef UTILS_FILESYSTEM_HPP
#define UTILS_FILESYSTEM_HPP

#include <string> // std::string

namespace utils {
    
    [[nodiscard]] std::string to_native_separator(std::string_view in);
    
}

#endif // UTILS_FILESYSTEM_HPP
