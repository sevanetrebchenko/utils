
#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

#include <vector>
#include <string>

namespace utils {

    // Returns a vector containing the result of splitting 'in' by 'delimiter'.
    [[nodiscard]] std::vector<std::string> split(const std::string& in, const std::string& delimiter);
    
    // Returns a string of 'components' joined by 'glue'.
    [[nodiscard]] std::string join(const std::vector<std::string>& components, const std::string& glue);
    
    // Trim off all whitespace characters on either side of 'in'.
    [[nodiscard]] std::string trim(const std::string& in);
    
}

#endif // UTILS_STRING_HPP
