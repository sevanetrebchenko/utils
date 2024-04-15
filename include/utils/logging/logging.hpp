
#pragma once

#ifndef UTILS_LOGGING_HPP
#define UTILS_LOGGING_HPP

namespace utils {
    
    class FormatStringWrapper;
    
    namespace logging {
        
        template <typename ...Ts>
        void info(FormatStringWrapper fmt, const Ts&... args);
        
        template <typename ...Ts>
        void debug(FormatStringWrapper fmt, const Ts&... args);
        
        template <typename ...Ts>
        void warning(FormatStringWrapper fmt, const Ts&... args);
        
        template <typename ...Ts>
        void error(FormatStringWrapper fmt, const Ts&... args);
        
        template <typename ...Ts>
        void fatal(FormatStringWrapper fmt, const Ts&... args);
        
    }
}

// Template definitions.
#include "utils/detail/logging.tpp"

#endif // UTILS_LOGGING_HPP
