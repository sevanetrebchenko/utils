
#ifndef UTILS_FILESYSTEM_HPP
#define UTILS_FILESYSTEM_HPP

#include <string> // std::string
#include <filesystem> // std::filesystem

namespace utils {
    
    [[nodiscard]] std::string read(const std::filesystem::path& path);
    
}

#endif // UTILS_FILESYSTEM_HPP
