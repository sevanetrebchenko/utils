
#include "utils/logging.hpp"
#include "utils/internal/logging.tpp"

#include "utils/string.hpp"

namespace utils::logging {
    
    namespace internal {
        
        Adapter::Adapter(std::string fmt, MessageLevel level) : m_format(std::move(fmt)),
                                                                m_level(level) {
            // Validate format string.
            using namespace utils::internal;
            FormatString str = FormatString(m_format);
        }
        
        Adapter::~Adapter() = default;
        
        FileAdapter::FileAdapter(const std::string& file, const std::string& fmt, MessageLevel level) : Adapter(fmt, level) {
            m_file.open(file);
            if (!m_file.is_open()) {
            }
        }
        
        // std::fstream destructor automatically closes the file.
        FileAdapter::~FileAdapter() = default;
        
        void FileAdapter::operator<<(const Log& data) {
        }
        
        CallbackAdapter::CallbackAdapter(CallbackAdapter::CallbackType callback, const std::string& fmt, MessageLevel level) : Adapter(fmt, level),
                                                                                                                               m_callback(std::move(callback)) {
        }
        
        CallbackAdapter::~CallbackAdapter() = default;
        
        void CallbackAdapter::operator<<(const Log& data) {
            m_callback(utils::format(m_format, arg("message", data.message)));
        }
        
    }
    
    Adapter::Builder utils::logging::add_adapter(const std::string& name) {
        return Adapter::Builder(name);
    }
    
}


