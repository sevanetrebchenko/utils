
#ifndef UTILS_LEVEL_HPP
#define UTILS_LEVEL_HPP

namespace utils {
    namespace logging {
    
        enum class MessageLevel {
            Info = 0,
            Debug,
            Warning,
            Error,
            Fatal
        };
        
    }
}

#endif // UTILS_LEVEL_HPP
