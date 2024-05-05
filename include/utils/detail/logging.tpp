
#pragma once

#include "utils/logging/level.hpp"
#include "utils/format.hpp"

#include <source_location> // std::source_location

namespace utils::logging {
    
    namespace detail {
        
        void log(MessageLevel level, const std::string& message, std::source_location source);
        
    }
    
    template <typename ...Ts>
    void info(const FormatString& fmt, const Ts&... args) {
        detail::log(MessageLevel::Info, fmt.format(args...), fmt.source());
    }
    
    template <typename ...Ts>
    void debug(const FormatString& fmt, const Ts&... args) {
        detail::log(MessageLevel::Debug, fmt.format(args...), fmt.source());
    }
    
    template <typename ...Ts>
    void warning(const FormatString& fmt, const Ts&... args) {
        detail::log(MessageLevel::Warning, fmt.format(args...), fmt.source());
    }
    
    template <typename ...Ts>
    void error(const FormatString& fmt, const Ts&... args) {
        detail::log(MessageLevel::Error, fmt.format(args...), fmt.source());
    }
    
    template <typename ...Ts>
    void fatal(const FormatString& fmt, const Ts&... args) {
        detail::log(MessageLevel::Fatal, fmt.format(args...), fmt.source());
    }
}
