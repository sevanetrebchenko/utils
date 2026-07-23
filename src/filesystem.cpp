
#include "utils/filesystem.hpp"
#include <fstream> // std::ifstream

namespace utils {

    std::string read(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        std::streamsize length = file.tellg();

        std::string content(length, '\0');
        file.seekg(0);
        file.read(&content[0], length);

        return std::move(content);
    }

}