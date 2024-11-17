
#pragma once

#ifndef LOGGING_TPP
#define LOGGING_TPP

#include "utils/string.hpp"

namespace utils::logging {
    
    namespace detail {
        
        void log(Message& message);
        void add_sink(const std::shared_ptr<Sink>& sink);
    
    }
    
    template <typename ...Ts>
    void info(Message message, const Ts&... args) {
        message.message = utils::format(message.format, args..., NamedArgument("__source", message.source));
        message.level = Message::Level::Info;
        detail::log(message);
    }
    
    template <typename ...Ts>
    void debug(Message message, const Ts&... args) {
        message.message = utils::format(message.format, args..., NamedArgument("__source", message.source));
        message.level = Message::Level::Debug;
        detail::log(message);
    }
    
    template <typename ...Ts>
    void warning(Message message, const Ts&... args) {
        message.message = utils::format(message.format, args..., NamedArgument("__source", message.source));
        message.level = Message::Level::Warning;
        detail::log(message);
    }
    
    template <typename ...Ts>
    void error(Message message, const Ts&... args) {
        message.message = utils::format(message.format, args..., NamedArgument("__source", message.source));
        message.level = Message::Level::Error;
        detail::log(message);
    }
    
    template <typename ...Ts>
    void fatal(Message message, const Ts&... args) {
        std::string str = utils::format(message.format, args..., NamedArgument("__source", message.source));
        
        // Log error message
        message.message = str;
        message.level = Message::Level::Error;
        detail::log(message);
        
        throw std::runtime_error(str);
    }
    
    template <typename T, typename ...Ts>
    std::shared_ptr<T> create_sink(Ts&&... args) {
        static_assert(std::is_base_of<Sink, T>::value, "type provided to create_sink must derive from Sink");
        std::shared_ptr<T> sink = std::make_shared<T>(std::move(args)...);
        detail::add_sink(sink);
        return std::move(sink);
    }
    
}

#endif // LOGGING_TPP