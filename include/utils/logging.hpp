
#pragma once

#ifndef LOGGING_HPP
#define LOGGING_HPP

namespace utils {
    
    class FormatString;
    
    namespace logging {

        template <typename ...Ts>
        void info(FormatString fmt, const Ts&... args);
        
        template <typename ...Ts>
        void debug(FormatString fmt, const Ts&... args);
        
        template <typename ...Ts>
        void warning(FormatString fmt, const Ts&... args);
        
        template <typename ...Ts>
        void error(FormatString fmt, const Ts&... args);
        
        template <typename ...Ts>
        void fatal(FormatString fmt, const Ts&... args);
        
        void set_format(const FormatString& fmt);
        void clear_format();
        
    }
}

#endif // LOGGING_HPP

// Template definitions.
#include "utils/detail/logging.tpp"

