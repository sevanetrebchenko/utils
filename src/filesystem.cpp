
#include "utils/filesystem.hpp"
#include <fstream> // std::ifstream

namespace utils {

    std::string load(const std::filesystem::path& path) {
        std::ifstream file(path, std::ios::in);
        if (!file.is_open()) {
            throw std::runtime_error("");
        }

        // Get the length of the file
        file.seekg(0, std::ifstream::end);
        std::streamsize length = file.tellg();
        file.seekg(0, std::ifstream::beg);

        std::string source;
        source.reserve(length);

        std::string line;
        while (std::getline(file, line)) {
            source += line;
            source += '\n';
        }

        return std::move(source);
    }

}