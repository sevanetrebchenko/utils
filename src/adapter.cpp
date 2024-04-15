
#include "utils/logging/adapter.hpp"

namespace utils::logging {
    
    Adapter::Adapter(std::string name) {
    
    }
    
    Adapter::~Adapter() {
    }
    
    void Adapter::enable() {
        m_enabled = true;
    }
    
    void Adapter::disable() {
        m_enabled = false;
    }
    
    void Adapter::set_format(std::string format) {
    
    }
    
    void Adapter::set_level(MessageLevel level) {
    
    }
    
    void Adapter::enable_buffering(std::size_t size) {
    
    }
    
    void Adapter::flush() {
    
    }
    
    void Adapter::flush_every(Duration duration) {
    
    }
    
    void Adapter::disable_buffering() {
    
    }
    
    std::string_view Adapter::name() const {
        return std::string_view();
    }
    
    std::string_view Adapter::format_string() const {
        return std::string_view();
    }
}