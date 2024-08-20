
#pragma once

#include "utils/string.hpp"

namespace utils {
    namespace internal {
        
        [[noreturn]] inline void cppassert(const std::string& expression, std::source_location source, const std::string& message);
        
    }
    
    template <typename ...Ts>
    void cppassert(const std::string& expression, bool result, std::source_location source, const std::string& message, const Ts&... args) {
        if (!result) {
            internal::cppassert(expression, source, utils::format(message, args...));
        }
    }
    
}
