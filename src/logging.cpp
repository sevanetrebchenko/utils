//
//#include "utils/logging/logging.hpp"
//#include "utils/string.hpp"
//
//namespace utils::logging {
//
//    namespace internal {
//
//        AdapterConfiguration::AdapterConfiguration(const std::string& name) : adapter(std::shared_ptr<Adapter>(nullptr)),
//                                                                              name(name),
//                                                                              format("{message}"),
//                                                                              level(MessageLevel::Debug),
//                                                                              enabled(true) {
//        }
//
//        AdapterConfiguration::~AdapterConfiguration() = default;
//
//        FileAdapter::FileAdapter(const std::string& file, std::ios_base::openmode open_mode) : m_file(file, open_mode) {
//            if (!m_file.is_open()) {
//                throw FormatError("");
//            }
//        }
//
//        // std::ofstream destructor automatically closes the file.
//        FileAdapter::~FileAdapter() = default;
//
//        void FileAdapter::operator<<(const std::string& message) {
//            m_file << message;
//        }
//
//        CallbackAdapter::CallbackAdapter(CallbackAdapter::CallbackType callback) : m_callback(std::move(callback)) {
//        }
//
//        CallbackAdapter::~CallbackAdapter() = default;
//
//        void CallbackAdapter::operator<<(const std::string& message) {
//            m_callback(message);
//        }
//
//    }
//
//    Adapter::Adapter() : m_adapter(std::numeric_limits<std::size_t>::max()) {
//    }
//
//    Adapter::~Adapter() = default;
//
//    void Adapter::enable() {
//    }
//
//    void Adapter::disable() {
//    }
//
//    void Adapter::set_format(const std::string& fmt) {
//    }
//
//    void Adapter::set_message_level(MessageLevel level) {
//    }
//
//
//
//    Adapter::Builder::Builder(const std::string& name) {
//        using namespace internal;
//
//        for (const AdapterConfiguration& configuration : adapter_configurations) {
//            if (name == configuration.name) {
//                throw FormatError("adapter with name '{}' already exists (did you mean to use get_adapter instead?)");
//            }
//        }
//
//        // Configure new adapter.
//        m_adapter = adapter_configurations.size();
//        adapter_configurations.emplace_back(name);
//    }
//
//    Adapter::Builder::~Builder() = default;
//
//    Adapter::Builder& Adapter::Builder::with_callback(const std::function<void(const std::string&)>& callback) {
//        using namespace internal;
//        adapter_configurations[m_adapter].adapter = std::make_shared<CallbackAdapter>(callback);
//    }
//
//    Adapter::Builder& Adapter::Builder::with_filename(const std::string& path, std::ios_base::openmode open_mode) {
//        using namespace internal;
//        adapter_configurations[m_adapter].adapter = std::make_shared<FileAdapter>(path, open_mode);
//    }
//
//    Adapter::Builder& Adapter::Builder::with_format(const std::string& fmt) {
//    }
//
//    Adapter::Builder& Adapter::Builder::with_message_level(MessageLevel level) {
//    }
//
//    Adapter::Builder create_adapter(const std::string& name) {
//        return Adapter::Builder(name);
//    }
//
//}
//
//

#include "utils/logging/logging.hpp"

#include <iostream>

namespace utils::logging {
    
    namespace detail {
        
        void log(MessageLevel level, const std::string& message, std::source_location source) {
            std::cout << to_string(level) << " : " << message << ", from " << source.file_name() << ":" << source.line() << std::endl;
        }
        
    }
    
    std::string to_string(MessageLevel level) {
        if (level == MessageLevel::Info) {
            return "info";
        }
        else if (level == MessageLevel::Debug) {
            return "debug";
        }
        else if (level == MessageLevel::Warning) {
            return "warning";
        }
        else if (level == MessageLevel::Error) {
            return "error";
        }
        else {
            return "fatal";
        }
    }
    
}