
#ifndef UTILS_LOGGING_TPP
#define UTILS_LOGGING_TPP

#include "utils/string.hpp"

#include <vector> // std::vector
#include <fstream> // std::ofstream
#include <memory> // std::shared_ptr

namespace utils::logging {
    
    namespace internal {
        
        struct Log {
            MessageLevel level;
            std::string message;
            std::source_location source;
        };
        
        class Adapter {
            public:
                Adapter(std::string fmt, MessageLevel level);
                virtual ~Adapter();
                
                virtual void operator<<(const Log& log) = 0;
            
            protected:
                std::string m_format;
                MessageLevel m_level;
        };
        
        class FileAdapter final : public Adapter {
            public:
                FileAdapter(const std::string& file, const std::string& fmt, MessageLevel level);
                ~FileAdapter() override;
                
                void operator<<(const Log& data) override;
                
            private:
                std::ofstream m_file;
        };
        
        class CallbackAdapter final : public Adapter {
            public:
                using CallbackType = std::function<void(const std::string&)>;
                
                CallbackAdapter(CallbackType callback, const std::string& fmt, MessageLevel level);
                ~CallbackAdapter();
                
                void operator<<(const Log& data) override;
                
            private:
                std::function<void(const std::string&)> m_callback;
        };
        
        extern std::vector<std::shared_ptr<Adapter>> adapters;
        
        void log(MessageLevel level, const std::string& message, std::source_location source);
        
    }
    
    // logging::info implementation
    template <typename... Ts>
    info<Ts...>::info(const std::string& fmt, const Ts& ... args, std::source_location source) {
        internal::log(MessageLevel::Info, utils::format(fmt, args...), source);
    }
    
    template <typename... Ts>
    info<Ts...>::~info() = default;
    
    // logging::debug implementation
    template <typename... Ts>
    debug<Ts...>::debug(const std::string& fmt, const Ts& ... args, std::source_location source) {
        internal::log(MessageLevel::Debug, utils::format(fmt, args...), source);
    }
    
    template <typename... Ts>
    debug<Ts...>::~debug() = default;
    
    // logging::warning implementation
    template <typename... Ts>
    warning<Ts...>::warning(const std::string& fmt, const Ts& ... args, std::source_location source) {
        internal::log(MessageLevel::Warning, utils::format(fmt, args...), source);
    }
    
    template <typename... Ts>
    warning<Ts...>::~warning() = default;
    
    // logging::error implementation
    template <typename... Ts>
    error<Ts...>::error(const std::string& fmt, const Ts& ... args, std::source_location source) {
        internal::log(MessageLevel::Error, utils::format(fmt, args...), source);
    }
    
    template <typename... Ts>
    error<Ts...>::~error() = default;
    
    // logging::fatal implementation
    template <typename... Ts>
    fatal<Ts...>::fatal(const std::string& fmt, const Ts& ... args, std::source_location source) {
        internal::log(MessageLevel::Fatal, utils::format(fmt, args...), source);
    }
    
    template <typename... Ts>
    fatal<Ts...>::~fatal() = default;
    
}

#endif // UTILS_LOGGING_TPP