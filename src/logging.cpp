////
////#include "utils/logging/logging.hpp"
////#include "utils/string.hpp"
////
////namespace utils::logging {
////
////    namespace internal {
////
////        AdapterConfiguration::AdapterConfiguration(const std::string& name) : adapter(std::shared_ptr<Adapter>(nullptr)),
////                                                                              name(name),
////                                                                              format("{message}"),
////                                                                              level(MessageLevel::Debug),
////                                                                              enabled(true) {
////        }
////
////        AdapterConfiguration::~AdapterConfiguration() = default;
////
////        FileAdapter::FileAdapter(const std::string& file, std::ios_base::openmode open_mode) : m_file(file, open_mode) {
////            if (!m_file.is_open()) {
////                throw FormatError("");
////            }
////        }
////
////        // std::ofstream destructor automatically closes the file.
////        FileAdapter::~FileAdapter() = default;
////
////        void FileAdapter::operator<<(const std::string& message) {
////            m_file << message;
////        }
////
////        CallbackAdapter::CallbackAdapter(CallbackAdapter::CallbackType callback) : m_callback(std::move(callback)) {
////        }
////
////        CallbackAdapter::~CallbackAdapter() = default;
////
////        void CallbackAdapter::operator<<(const std::string& message) {
////            m_callback(message);
////        }
////
////    }
////
////    Adapter::Adapter() : m_adapter(std::numeric_limits<std::size_t>::max()) {
////    }
////
////    Adapter::~Adapter() = default;
////
////    void Adapter::enable() {
////    }
////
////    void Adapter::disable() {
////    }
////
////    void Adapter::set_format(const std::string& fmt) {
////    }
////
////    void Adapter::set_message_level(MessageLevel level) {
////    }
////
////
////
////    Adapter::Builder::Builder(const std::string& name) {
////        using namespace internal;
////
////        for (const AdapterConfiguration& configuration : adapter_configurations) {
////            if (name == configuration.name) {
////                throw FormatError("adapter with name '{}' already exists (did you mean to use get_adapter instead?)");
////            }
////        }
////
////        // Configure new adapter.
////        m_adapter = adapter_configurations.size();
////        adapter_configurations.emplace_back(name);
////    }
////
////    Adapter::Builder::~Builder() = default;
////
////    Adapter::Builder& Adapter::Builder::with_callback(const std::function<void(const std::string&)>& callback) {
////        using namespace internal;
////        adapter_configurations[m_adapter].adapter = std::make_shared<CallbackAdapter>(callback);
////    }
////
////    Adapter::Builder& Adapter::Builder::with_filename(const std::string& path, std::ios_base::openmode open_mode) {
////        using namespace internal;
////        adapter_configurations[m_adapter].adapter = std::make_shared<FileAdapter>(path, open_mode);
////    }
////
////    Adapter::Builder& Adapter::Builder::with_format(const std::string& fmt) {
////    }
////
////    Adapter::Builder& Adapter::Builder::with_message_level(MessageLevel level) {
////    }
////
////    Adapter::Builder create_adapter(const std::string& name) {
////        return Adapter::Builder(name);
////    }
////
////}
////
////
//

#include "utils/string.hpp"
#include "utils/logging.hpp"

#include <source_location>
#include <iostream>

namespace utils {
    
    using namespace logging;
    
    template <>
    struct Formatter<Message::Level> {
        void parse(const FormatSpec& spec) {
        }
        
        std::string format(Message::Level level) const {
            switch (level) {
                case Message::Level::Debug:
                    return "DEBUG";
                case Message::Level::Info:
                    return "INFO";
                case Message::Level::Warning:
                    return "WARNING";
                case Message::Level::Error:
                    return "ERROR";
                case Message::Level::Fatal:
                    return "FATAL";
            }
        }
    };
    
    namespace logging {
        
        namespace detail {
    
            void log(const Message& message) {
                std::cout << utils::format("[{level}] - {message}, from {source}", NamedArgument("level", message.level), NamedArgument("message", message.message), NamedArgument("source", message.source)) << '\n';
            }
    
        }
        
        Message::Message(std::string_view fmt, std::source_location source) : level(Level::Debug),
                                                                              format(fmt),
                                                                              source(source),
                                                                              message() {
        }
        
        Message::Message(const char* fmt, std::source_location source) : level(Level::Debug),
                                                                         format(fmt),
                                                                         source(source),
                                                                         message() {
            
        }
        
        Message::~Message() = default;
        
    }
    
}
