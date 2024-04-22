
#include "utils/result.hpp"
#include <utility> // std::move

namespace utils {
    
    // Response implementation.
    
    Response::Response() = default;
    
    Response::Response(std::string error) : m_error(std::move(error)) {
    }
    
    Response::~Response() = default;
    
    Response Response::OK() {
        return { };
    }
    
    bool Response::ok() const {
        return m_error.empty();
    }
    
    const std::string& Response::what() const {
        return m_error;
    }
    
}