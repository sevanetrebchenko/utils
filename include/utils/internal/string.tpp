
#ifndef UTILS_STRING_TPP
#define UTILS_STRING_TPP

#include <sstream> // std::stringstream

namespace utils {

    template <typename Container>
    [[nodiscard]] std::string join(const Container& components, const std::string& glue) {
        static_assert(std::is_same<typename std::decay<decltype(*std::begin(std::declval<Container>()))>::type, std::string>::value, "container must be of string type");
        if (empty(components)) {
            return "";
        }
    
        std::stringstream builder { };
        
        auto iter = components.begin();
        builder << *iter++;
        
        for (; iter != components.end(); ++iter) {
            builder << glue << *iter;
        }
        
        return builder.str();
    }

}

#endif // UTILS_STRING_TPP
