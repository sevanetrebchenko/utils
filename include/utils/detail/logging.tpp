
#ifndef UTILS_LOGGING_TPP
#define UTILS_LOGGING_TPP

#include "utils/logging/level.hpp"
#include "utils/string.hpp"

#include <source_location> // std::source_location

namespace utils::logging {
    
    namespace detail {
        
        void log(MessageLevel level, const std::string& message, std::source_location source);
        
    }
    
    template <typename ...Ts>
    void info(FormatStringWrapper fmt, const Ts&... args) {
        detail::log(MessageLevel::Info, utils::format(fmt, args...), fmt.source());
    }
    
    template <typename ...Ts>
    void debug(FormatStringWrapper fmt, const Ts&... args) {
        detail::log(MessageLevel::Debug, utils::format(fmt, args...), fmt.source());
    }
    
    template <typename ...Ts>
    void warning(FormatStringWrapper fmt, const Ts&... args) {
        detail::log(MessageLevel::Warning, utils::format(fmt, args...), fmt.source());
    }
    
    template <typename ...Ts>
    void error(FormatStringWrapper fmt, const Ts&... args) {
        detail::log(MessageLevel::Error, utils::format(fmt, args...), fmt.source());
    }
    
    template <typename ...Ts>
    void fatal(FormatStringWrapper fmt, const Ts&... args) {
        detail::log(MessageLevel::Fatal, utils::format(fmt, args...), fmt.source());
    }
}

#endif // UTILS_LOGGING_TPP