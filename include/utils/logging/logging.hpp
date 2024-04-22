
#pragma once

#ifndef UTILS_LOGGING_HPP
#define UTILS_LOGGING_HPP

namespace utils {
    
    class FormatString;
    
    namespace logging {
        
        template <typename ...Ts>
        void debug(const FormatString& fmt, const Ts&... args);
        
        template <typename ...Ts>
        void info(const FormatString& fmt, const Ts&... args);
        
        template <typename ...Ts>
        void warning(const FormatString& fmt, const Ts&... args);
        
        template <typename ...Ts>
        void error(const FormatString& fmt, const Ts&... args);
        
        template <typename ...Ts>
        void fatal(const FormatString& fmt, const Ts&... args);
        
        void set_format(const FormatString& fmt);
        void clear_format();
        
    }
}

// Template definitions.
#include "utils/detail/logging.tpp"

#endif // UTILS_LOGGING_HPP
