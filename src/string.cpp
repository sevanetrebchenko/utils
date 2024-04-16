
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
    
    [[nodiscard]] std::string trim(std::string_view in) {
        static const char* ws = " \t\n\r";
        return std::string(in.substr(in.find_first_not_of(ws), in.length() - (in.find_last_not_of(ws) + 1)));
    }
    
}