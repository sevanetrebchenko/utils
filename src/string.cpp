
#include "utils/string.hpp"

namespace utils {

    [[nodiscard]] std::vector<std::string> split(const std::string& in, const std::string& delimiter) {
        std::vector<std::string> components { };
        
        std::string_view src(in); // readonly

        std::size_t position;
        do {
            position = src.find(delimiter);
            components.emplace_back(src.substr(0, position));
            src = src.substr(position + delimiter.length());
        }
        while (position != std::string::npos);
    
        return std::move(components);
    }
    
    [[nodiscard]] std::string join(const std::vector<std::string>& components, const std::string& glue) {
        if (components.empty()) {
            return "";
        }
    
        if (components.size() == 1) {
            return components[0];
        }
    
        std::string result;
        for (std::size_t i = 0u; i < components.size() - 1; ++i) {
            result += components[i] + glue;
        }
        result += components[components.size() - 1];
        
        return result;
    }
    
    [[nodiscard]] std::string trim(const std::string& in) {
        static const char* ws = " \t\n\r";
        return in.substr(in.find_first_not_of(ws), in.length() - (in.find_last_not_of(ws) + 1));
    }

}