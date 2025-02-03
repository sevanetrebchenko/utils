
#pragma once

#ifndef LOGGING_HPP
#define LOGGING_HPP

#include "utils/datetime.hpp"
#include "utils/string.hpp"
#include "utils/platform.hpp"

#include <string_view> // std::string_view
#include <source_location> // std::source_location
#include <filesystem> // std::filesystem
#include <fstream> // std::ofstream
#include <functional> // std::function
#include <span> // std::span
#include <mutex> // std::mutex

#if defined(PLATFORM_WINDOWS)
    #include <Windows.h>
#else
    #include <unistd.h> // pid_t
#endif

namespace utils {
    
    namespace logging {

        struct Message {
            Message(const std::string& fmt, std::source_location source = std::source_location::current());
            Message(std::string_view fmt, std::source_location source = std::source_location::current());
            Message(const char* fmt, std::source_location source = std::source_location::current());
            
            ~Message();
            
            enum class Level {
                Debug = 0,
                Info,
                Warning,
                Error,
            } level;
            
            std::string_view format;
            std::source_location source;
            Timestamp timestamp;
            std::string message;
            
            std::thread::id thread_id;
            
            #if defined(PLATFORM_WINDOWS)
                DWORD process_id;
            #else
                pid_t process_id;
            #endif
            
            // Scope that this message was logged from
            std::span<std::string> scope;
        };
        
        template <typename ...Ts>
        void info(Message message, const Ts&... args);
        
        template <typename ...Ts>
        void debug(Message message, const Ts&... args);
        
        template <typename ...Ts>
        void warning(Message message, const Ts&... args);
        
        template <typename ...Ts>
        void error(Message message, const Ts&... args);
        
        // Logs error message and raises an exception in one go (std::runtime_error)
        template <typename ...Ts>
        void fatal(Message message, const Ts&... args);
        
        // Scopes are thread local
        void push_scope(std::string name);
        void pop_scope();
        
        
        // Sinks are inherently thread-safe
        // The following placeholders can be used in sink format strings:
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
        //   - source:
        //   - filename:
        //   - line:
        //   - thread_id, tid: thread ID
        //   - process_id, pid: process ID
        
        class Sink {
            public:
                // Sinks must provide a name that can be used to retrieve them
                explicit Sink(std::string name, std::optional<std::string> format = { }, Message::Level level = Message::Level::Info);
                virtual ~Sink();
                
                void log(const Message& data);
                virtual void flush();
                
                void set_format(const std::string& format);
                void reset_format(); // Resets Sink format string to the default
                
                void set_level(Message::Level level);
                [[nodiscard]] Message::Level get_level() const;
                
                [[nodiscard]] std::string_view get_name() const;
                [[nodiscard]] std::string_view get_format() const;
                
                void enable();
                void disable();

            private:
                virtual void log(std::string_view message, const Message& data) = 0;
                
                std::string m_name;
                Message::Level m_level;
                std::mutex m_lock;
                std::string m_format;
                bool m_enabled;
        };
        
        class FileSink : public Sink {
            public:
                FileSink(const std::filesystem::path& filepath, std::ios::openmode open_mode, std::optional<std::string> format = { }, Message::Level level = Message::Level::Info);
                ~FileSink() override;
                
            private:
                void log(std::string_view message, const Message& data) override;
                void flush() override;
                
                std::ofstream m_file;
        };
        
        void set_default_format(std::string format);
        
        template <typename ...Ts>
        std::shared_ptr<Sink> create_sink(Ts&&... args);
        void destroy_sink(std::string_view name);
        
        // There are several sinks that are created by default:
        //   stdout - standard output stream
        //   stderr - standard error stream
        std::shared_ptr<Sink> get_sink(std::string_view name);

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

// Template definitions.
#include "utils/detail/logging.tpp"

#endif // LOGGING_HPP
