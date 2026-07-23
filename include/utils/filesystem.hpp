
#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <string> // std::string
#include <filesystem> // std::filesystem

namespace utils {
    
    [[nodiscard]] std::string read(const std::filesystem::path& path);
    
}

#endif // FILESYSTEM_HPP
