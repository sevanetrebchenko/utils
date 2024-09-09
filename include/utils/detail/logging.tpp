
#pragma once

#ifndef LOGGING_TPP
#define LOGGING_TPP

#include "utils/string.hpp"
#include <source_location> // std::source_location

namespace utils::logging {
    
    namespace detail {
        
        void log(const Message& message);
        
    }
    
    template <typename ...Ts>
    void info(Message message, const Ts&... args) {
        message.message = utils::format(message.format, args...);
        message.level = Message::Level::Info;
        detail::log(message);
    }
    
    template <typename ...Ts>
    void debug(Message message, const Ts&... args) {
        message.message = utils::format(message.format, args...);
        message.level = Message::Level::Debug;
        detail::log(message);
    }
    
    template <typename ...Ts>
    void warning(Message message, const Ts&... args) {
        message.message = utils::format(message.format, args...);
        message.level = Message::Level::Warning;
        detail::log(message);
    }
    
    template <typename ...Ts>
    void error(Message message, const Ts&... args) {
        message.message = utils::format(message.format, args...);
        message.level = Message::Level::Error;
        detail::log(message);
    }
    
    template <typename ...Ts>
    void fatal(Message message, const Ts&... args) {
        message.message = utils::format(message.format, args...);
        message.level = Message::Level::Fatal;
        detail::log(message);
    }
    
}

#endif // LOGGING_TPP