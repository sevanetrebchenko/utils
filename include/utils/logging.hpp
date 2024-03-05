
#ifndef UTILS_LOGGING_HPP
#define UTILS_LOGGING_HPP

#include <string> // std::string
#include <source_location> // std::source_location
#include <functional> // std::function
#include <memory> // std::shared_ptr

#define DEFINE_LOGGING_LEVEL(LEVEL)                                           \
    template <typename ...Ts>                                                 \
    struct LEVEL {                                                            \
        LEVEL(const std::string& fmt, const Ts&... args,                      \
              std::source_location source = std::source_location::current()); \
        ~LEVEL();                                                             \
    };                                                                        \
    template <typename ...Ts>                                                 \
    LEVEL(const std::string&, const Ts&...) -> LEVEL<Ts...>

namespace utils {
    namespace logging {

        enum class MessageLevel {
            Info = 0,
            Debug,
            Warning,
            Error,
            Fatal
        };
        
        // logging::info(...);
        DEFINE_LOGGING_LEVEL(info);
        
        // logging::debug(...);
        DEFINE_LOGGING_LEVEL(debug);
        
        // logging::warning(...);
        DEFINE_LOGGING_LEVEL(warning);
        
        // logging::error(...);
        DEFINE_LOGGING_LEVEL(error);
        
        // logging::fatal(...);
        DEFINE_LOGGING_LEVEL(fatal);
        
        // Adapters control where messages are published to.
        // The following placeholders can be used to format custom adapters:
        //   - message: message content
        //   - level: message level
        //   - date: message date (default formatting: MM/DD/YYYY)
        //   - day: timestamp day (01 - 31)
        //   - month: timestamp month (01 - 12)
        //   - year: timestamp year
        //   - time: message timestamp
        //   - hour:
        //   - minute:
        //   - second:
        //   - millisecond:
        //   - filename:
        //   - pathname:
        //   - function:
        //   - line:
        //   - pid:
        //   - thread ID:
        
        class Adapter {
            public:
                Adapter(std::string fmt, MessageLevel level);
                virtual ~Adapter();
                
                void enable();
                void disable();
            
                void set_format(const std::string& fmt);
                void set_message_level(MessageLevel level);
                
            protected:
                class Builder;
                
                // Adapters can only be created through the add_adapter interface.
                friend Builder add_adapter(const std::string&);
                
                virtual void operator<<(const std::string& message) = 0;
                
                std::string m_format;
                MessageLevel m_level;
                
                bool m_enabled;
        };
        
        class Adapter::Builder {
            public:
                Builder(const std::string& name);
                ~Builder();
                
                Builder& with_callback(std::function<void(const std::string&)> callback);
                Builder& with_filename(const std::string& path);
                
                Builder& with_format(const std::string& fmt);
                Builder& with_message_level(MessageLevel level);
                
            private:
                std::shared_ptr<Adapter> adapter;
        };
        
        [[nodiscard]] Adapter::Builder add_adapter(const std::string& name);
        [[nodiscard]] std::shared_ptr<Adapter> get_adapter(const std::string& name);
        void remove_adapter(const std::string& name);
        
    }
}

// Template definitions.
#include "utils/internal/logging.tpp"

#endif // UTILS_LOGGING_HPP
