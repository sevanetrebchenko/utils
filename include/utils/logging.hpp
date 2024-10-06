
#pragma once

#ifndef LOGGING_HPP
#define LOGGING_HPP

#include "utils/datetime.hpp"
#include "utils/string.hpp"
#include "utils/colors.hpp"

#include <string_view> // std::string_view
#include <source_location> // std::source_location
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
            Timestamp timestamp;
            std::string_view scope;
            unsigned thread_id;
            unsigned process_id;
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
        //   - filename:
        //   - function:
        //   - line:
        //   - thread_id, tid: thread ID
        //   - process_id, pid: process ID
        
        class Sink {
            public:
                Sink();
                virtual ~Sink();
                
                void log(const Message& message);
                
                void enable();
                void disable();
                
            private:
                virtual void log(std::string_view message, std::optional<Color> color) = 0;
                virtual void flush();
                
                std::mutex m_lock;
                std::string m_format;
                Message::Level m_level;
                bool m_enabled;
        };
        
        class FileSink : public Sink {
            public:
                FileSink(std::filesystem::path filepath, std::ios::openmode open_mode);
                ~FileSink() override;
                
            private:
                void log(std::string_view message, std::optional<Color> color) override;
                void flush() override;
        };
        
        template <typename ...Ts>
        std::shared_ptr<Sink> create_sink(Ts&&... args);
        std::shared_ptr<Sink> get_sink(std::string_view name);
        void remove_sink(std::string_view name);

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
