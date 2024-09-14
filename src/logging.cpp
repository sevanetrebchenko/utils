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
#include "utils/assert.hpp"

#include <source_location>
#include <iostream>

namespace utils {
    
    namespace logging {
        
        namespace detail {
    
            void log(const Message& message) {
                std::cout << utils::format("[{level}] - {message}, from {source}", NamedArgument("level", message.level), NamedArgument("message", message.message), NamedArgument("source", message.source)) << '\n';
            }
    
        }
        
        Message::Message(std::string_view fmt, std::source_location source) : level(Level::Debug),
                                                                              format(fmt),
                                                                              source(source),
                                                                              message(),
                                                                              time(Timestamp::now()) {
        }
        
        Message::Message(const char* fmt, std::source_location source) : level(Level::Debug),
                                                                         format(fmt),
                                                                         source(source),
                                                                         message(),
                                                                         time(Timestamp::now()) {
        }
        
        Message::~Message() = default;
        
    }
    
    Formatter<logging::Message::Level>::Formatter() : Formatter<const char*>(),
                                                      uppercase(true) {
    }
    
    Formatter<logging::Message::Level>::~Formatter() = default;
    
    void Formatter<logging::Message::Level>::parse(const FormatSpec& spec) {
        ASSERT(spec.type() == FormatSpec::Type::SpecifierList, "format spec for Message::Level must be a specifier list");
        Formatter<const char*>::parse(spec);
        
        if (spec.has_specifier("uppercase", "lowercase")) {
            FormatSpec::SpecifierView specifier = spec.get_specifier("uppercase", "lowercase");
            std::string_view value = trim(specifier.value);
            
            if (icasecmp(specifier.name, "uppercase")) {
                if (icasecmp(value, "true") || icasecmp(value, "1")) {
                    uppercase = true;
                }
            }
            else { // if (icasecmp(specifier.name, "lowercase")) {
                if (icasecmp(value, "true") || icasecmp(value, "1")) {
                    uppercase = false;
                }
            }
        }
    }
    
    std::string Formatter<logging::Message::Level>::format(logging::Message::Level level) const {
        using namespace logging;
        
        const char* str;
        if (uppercase) {
            if (level == Message::Level::Debug) {
                str = "DEBUG";
            }
            else if (level == Message::Level::Info) {
                str = "INFO";
            }
            else if (level == Message::Level::Warning) {
                str = "WARNING";
            }
            else if (level == Message::Level::Error) {
                str = "ERROR";
            }
            else { // if (level == Message::Level::Fatal) {
                str = "FATAL";
            }
        }
        else {
            if (level == Message::Level::Debug) {
                str = "debug";
            }
            else if (level == Message::Level::Info) {
                str = "info";
            }
            else if (level == Message::Level::Warning) {
                str = "warning";
            }
            else if (level == Message::Level::Error) {
                str = "error";
            }
            else { // if (level == Message::Level::Fatal) {
                str = "fatal";
            }
        }
        
        return Formatter<const char*>::format(str);
    }
    
}
