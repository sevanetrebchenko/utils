
#include "utils/string.hpp"
#include "utils/exceptions.hpp"

namespace utils {
    
    [[nodiscard]] std::vector<std::string> split(std::string_view in, const std::string& delimiter) {
        std::vector<std::string> components { };

        std::size_t position;
        do {
            position = in.find(delimiter);
            components.emplace_back(in.substr(0, position));
            in = in.substr(position + delimiter.length());
        }
        while (position != std::string::npos);
    
        return std::move(components);
    }
    
    [[nodiscard]] std::string join(const std::initializer_list<std::string>& components, const std::string& glue) {
         return ""; // return join<std::initializer_list<std::string>>(components, glue);
    }
    
    [[nodiscard]] std::string_view trim(std::string_view in) {
        if (in.empty()) {
            return "";
        }
        
        std::size_t start = 0u;
        while (std::isspace(in[start])) {
            ++start;
        }
        
        std::size_t end = in.length() - 1u;
        while (end != start && std::isspace(in[end])) {
            --end;
        }
        
        return in.substr(start, end + 1);
    }
    
}