
#ifndef LOGGING_HPP
#define LOGGING_HPP

#include <string_view> // std::string_view
#include <source_location> // std::source_location

namespace utils {
    
    namespace logging {

        struct Message {
            // For types that can be implicitly converted to std::string_view (such as std::string)
            Message(std::string_view fmt, std::source_location source = std::source_location::current());
            
            // For inline strings
            Message(const char* fmt, std::source_location source = std::source_location::current());
            
            ~Message();
            
            enum class Level {
                Debug = 0,
                Info,
                Warning,
                Error,
                Fatal
            } level;
            
            std::string_view format;
            std::source_location source;
            
            std::string message;
        };
        
        template <typename ...Ts>
        void info(Message message, const Ts&... args);
        
        template <typename ...Ts>
        void debug(Message message, const Ts&... args);
        
        template <typename ...Ts>
        void warning(Message message, const Ts&... args);
        
        template <typename ...Ts>
        void error(Message message, const Ts&... args);
        
        template <typename ...Ts>
        void fatal(Message message, const Ts&... args);
        
        void set_format(const char* fmt);
        void clear_format();
        
    }
}

#endif // LOGGING_HPP

// Template definitions.
#include "utils/detail/logging.tpp"

