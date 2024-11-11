
#ifndef FILESYSTEM_HPP
#define FILESYSTEM_HPP

#include <string> // std::string
#include <filesystem> // std::filesystem

namespace utils {
    
    std::string load(const std::filesystem::path& path);
    
}

#endif // FILESYSTEM_HPP
