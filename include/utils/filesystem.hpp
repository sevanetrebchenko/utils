
#ifndef UTILS_FILESYSTEM_HPP
#define UTILS_FILESYSTEM_HPP

#include <string> // std::string
#include <filesystem> // std::filesystem
#include <string_view> // std::string_view

namespace utils {
    
    [[nodiscard]] std::string read(const std::filesystem::path& path);

    void write(const std::filesystem::path& path, std::string_view content);

}

#endif // UTILS_FILESYSTEM_HPP
