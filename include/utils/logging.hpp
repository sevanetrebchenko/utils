
#pragma once

#ifndef LOGGING_HPP
#define LOGGING_HPP

namespace utils {
    
    class FormatString;
    
    namespace logging {

        struct Log {
            template <String T>
            Log(T fmt, std::source_location source = std::source_location::current());
            ~Log();
            
            std::string format;
            
            std::string message;
            MessageLevel level;
            datetime::Timestamp timestamp;
            std::source_location source;
        };
        
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

