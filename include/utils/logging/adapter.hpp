
#ifndef UTILS_ADAPTER_HPP
#define UTILS_ADAPTER_HPP

#include "utils/logging.hpp"
#include "utils/datetime/duration.hpp"
#include "utils/string.hpp"

#include <memory> // std::shared_ptr
#include <string> // std::string
#include <fstream> // std::ofstream
#include <functional> // std::function
#include <typeindex> // std::type_index

namespace utils {
    namespace logging {
        
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
        
        class Adapter {
            public:
                Adapter(std::string name);
                virtual ~Adapter();
                
                void enable();
                void disable();
                
                void set_format(std::string format);
//                void set_level(MessageLevel level);
                
                void enable_buffering(std::size_t size);
                void flush();
                void flush_every(Duration duration);
                void disable_buffering();
                
                template <typename T>
                void set_format_override(std::string fmt);
                
                template <typename T>
                void clear_format_override();
                
                [[nodiscard]] std::string_view name() const;
                [[nodiscard]] std::string_view format_string() const;

//                template <typename ...Ts>
//                void info(const FormatString& fmt, const Ts&... args);
//
//                template <typename ...Ts>
//                void debug(const FormatString& fmt, const Ts&... args);
//
//                template <typename ...Ts>
//                void warning(const FormatString& fmt, const Ts&... args);
//
//                template <typename ...Ts>
//                void error(const FormatString& fmt, const Ts&... args);
//
//                template <typename ...Ts>
//                void fatal(const FormatString& fmt, const Ts&... args);
                
            protected:
//                virtual void operator<<(const Log& log) = 0;
                
                std::string m_name;
                
            private:
                // FormatString m_format;
//                MessageLevel m_level;
                bool m_enabled;
                
                std::unordered_map<std::type_index, std::string> m_format_overrides;
        };
        
        class StandardOutput final : public Adapter {
            public:
                StandardOutput();
                ~StandardOutput();
            
            private:
//                void operator<<(const Log& log) override;
        };
        
        struct StandardError final : public Adapter {
            public:
                StandardError();
                ~StandardError();
                
            private:
//                void operator<<(const Log& log) override;
        };
        
        class FileAdapter final : public Adapter {
            public:
                FileAdapter(std::string name, std::string file, std::ofstream::openmode open_mode);
                ~FileAdapter() override;
                
            private:
//                void operator<<(const Log& log) override;
                
                std::ofstream m_file;
        };
        
        class CallbackAdapter final : public Adapter {
            public:
                using CallbackType = std::function<void(const std::string& message)>;
                
                CallbackAdapter(CallbackType callback);
                ~CallbackAdapter() override;
                
            private:
//                void operator<<(const Log& log) override;
                
                CallbackType m_callback;
        };
        
        // Default adapter names:
        //   stdout - StandardOutput, redirects output to stdout
        //   stderr - StandardError, redirects output to stderr
        [[nodiscard]] std::shared_ptr<Adapter> get_adapter(const std::string& name);
        
    }
}


#endif // UTILS_ADAPTER_HPP
