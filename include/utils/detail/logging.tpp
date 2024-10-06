
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
    void trace(Message message, const Ts&... args) {
        message.message = utils::format(message.format, args...);
        message.level = Message::Level::Trace;
        detail::log(message);
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

namespace utils {
    
    template <typename T>
    struct Formatter<logging::detail::StyledArgument<T>> : public Formatter<T> {
        Formatter();
        ~Formatter() = default;
        
        void parse(const FormatSpec& spec);
        std::string format(const logging::detail::StyledArgument<T>& value) const;
        
        enum class Style {
            None = 0,
            Bold = 1,
            Italic = 3
        } style;
        
        std::optional<Color> color; // Foreground color (TODO: support for background colors?)
    };
    
    template <typename T>
    Formatter<logging::detail::StyledArgument<T>>::Formatter() : Formatter<T>(),
                                                                 style(Style::None),
                                                                 color() {
    }
    
    template <typename T>
    void Formatter<logging::detail::StyledArgument<T>>::parse(const FormatSpec& spec) {
        if (spec.type() == FormatSpec::Type::FormattingGroupList) {
            if (spec.has_group(0)) {
                const FormatSpec& group = spec.get_group(0);
                if (spec.has_specifier("color")) {
                
                }
            }
        }
        else {
        
        }
    }

    template <typename T>
    std::string Formatter<logging::detail::StyledArgument<T>>::format(const logging::detail::StyledArgument<T>& value) const {
        std::string&& _value = Formatter<T>::format(value.value);
    
        if (style) {
            if (color) {
                return utils::format("\033[{style};38;2;{red};{green};{blue}m{value}\033[0m", NamedArgument("style", style),
                                                                                              NamedArgument("red", color->r),
                                                                                              NamedArgument("green", color->g),
                                                                                              NamedArgument("blue", color->b),
                                                                                              NamedArgument("value", _value));
            }
            else {
                return utils::format("\033[{style}m{value}\033[0m", NamedArgument("style", style),
                                                                    NamedArgument("value", Formatter<T>::format(value.value)));
            }
        }
        else {
            if (color) {
                return utils::format("\033[38;2;{red};{green};{blue}m{value}\033[0m",NamedArgument("red", color->r),
                                                                                     NamedArgument("green", color->g),
                                                                                     NamedArgument("blue", color->b),
                                                                                     NamedArgument("value", Formatter<T>::format(value.value)));
            }
            else {
                return Formatter<T>::format(value.value);
            }
        }
    }
    
}


#endif // LOGGING_TPP