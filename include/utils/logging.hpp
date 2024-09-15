
#pragma once

#ifndef LOGGING_HPP
#define LOGGING_HPP

#include "utils/datetime.hpp"
#include "utils/string.hpp"

#include <string_view> // std::string_view
#include <source_location> // std::source_location
#include <memory> // std::shared_ptr
#include <filesystem> // std::filesystem
#include <functional> // std::function

namespace utils {
    
    namespace logging {

        struct Message {
            // For types that can be implicitly converted to std::string_view (such as std::string)
            Message(std::string_view fmt, std::source_location source = std::source_location::current());
            
            // For inline strings
            Message(const char* fmt, std::source_location source = std::source_location::current());
            
            ~Message();
            
            enum class Level {
                Trace = 0,
                Debug,
                Info,
                Warning,
                Error,
                Fatal
            } level;
            
            std::string_view format;
            std::source_location source;
            std::string message;
            Timestamp time;
        };
        
        template <typename ...Ts>
        void trace(Message message, const Ts&... args);
        
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
        
        // Overrides the default log message format
        void set_format(std::string_view fmt);
        void set_format(const char* fmt);

        // Reverts log message format to the default
        void clear_format();
        
        
        // Adapters control where messages are published to
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
        //   - thread_id: thread ID
        
        class Adapter {
            public:
                explicit Adapter(std::string name);
                virtual ~Adapter();
                
                void enable();
                void disable();
                
                void set_format(std::string_view fmt);
                void set_format(const char* fmt);
                
                void clear_format();
                
                void enable_buffering(std::size_t size);
                void flush_every(Duration duration);
                void flush_on(Message::Level level);
                
                // Messages are logged immediately
                void disable_buffering();
                
                virtual void flush();
                
                virtual void log(const Message& message) = 0;
                
            private:
                std::vector<Message> m_messages;
                std::size_t m_index;
                
                std::string name;
                std::string m_format;
                bool m_enabled;
                bool m_buffered;
        };
        
        // For redirecting messages to the console / terminal
        class ConsoleAdapter : public Adapter {
            public:
                explicit ConsoleAdapter(FILE* stream);
                ~ConsoleAdapter() override;
            
            private:
                void log(const Message& message) override;
                void flush() override;
                
                FILE* m_stream;
        };
        
        // For redirecting messages to a file
        class FileAdapter : public Adapter {
            public:
                FileAdapter(std::filesystem::path filepath, std::ios::openmode open_mode);
                ~FileAdapter();
                
            private:
                void log(const Message& message) override;
                void flush() override;
                
                FILE* m_file;
        };
        
        // For redirecting messages to a user-provided callback function
        class CallbackAdapter : public Adapter {
            public:
                using T = std::function<void(const Message&)>;
                
                CallbackAdapter(T fn);
                ~CallbackAdapter() override;
                
            private:
                void log(const Message& message);
                void flush() override;
                
                T m_callback;
        };
        
        class Logger {
            public:
                virtual ~Logger();
                
                template <typename ...Ts>
                void trace(Message message, const Ts&... args);
                
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
                
                void enable();
                void disable();
                
                template <typename ...Ts>
                void attach_adapter(std::shared_ptr<Ts>... adapters);
                
                std::shared_ptr<Adapter> get_adapter(std::string_view name) const;
                
                void detach_adapter(std::string_view name);
                
            private:
                // Logger instances should be created using create_logger
                template <typename T>
                friend std::shared_ptr<T> create_logger(std::string name);
                
                Logger(std::string name);
                
                Message::Level m_level;
                bool m_enabled;
                std::vector<std::shared_ptr<Adapter>> m_adapters;
        };
        
        template <typename T>
        std::shared_ptr<T> create_logger(std::string name);
        
        void destroy_logger(std::string_view name);
        
        std::shared_ptr<Logger> get_default_logger(); // Shorthand for get_logger("default");
        std::shared_ptr<Logger> get_logger(std::string_view name);
        
        void set_default_logger(std::shared_ptr<Logger> logger);
        void set_default_logger(std::string_view name); // Throws exception if logger is not found
        
    }
    
    template <>
    struct Formatter<logging::Message::Level> : public Formatter<const char*> {
        Formatter();
        ~Formatter();
        
        void parse(const FormatSpec& spec);
        std::string format(logging::Message::Level level) const;
        
        bool uppercase;
    };
    
}

#endif // LOGGING_HPP

// Template definitions.
#include "utils/detail/logging.tpp"

