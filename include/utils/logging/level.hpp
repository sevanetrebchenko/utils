
#ifndef UTILS_LEVEL_HPP
#define UTILS_LEVEL_HPP

#include <string> // std::string

namespace utils {
    namespace logging {
    
        enum class MessageLevel {
            Info = 0,
            Debug,
            Warning,
            Error,
            Fatal
        };
        
        [[nodiscard]] std::string to_string(MessageLevel);
        
    }
}

#endif // UTILS_LEVEL_HPP
