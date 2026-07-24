
#include "utils/filesystem.hpp"
#include <fstream> // std::ifstream, std::ofstream

namespace utils {

    std::string read(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        std::streamsize length = file.tellg();

        std::string content(length, '\0');
        file.seekg(0);
        file.read(&content[0], length);

        return std::move(content);
    }

    void write(const std::filesystem::path& path, std::string_view content) {
        std::ofstream file(path, std::ios::binary);
        file.write(content.data(), content.size());
        // File automatically closed by the destructor
    }

}
