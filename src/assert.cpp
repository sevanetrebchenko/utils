
#include "utils/assert.hpp"
#include "utils/logging.hpp"

namespace utils {
    namespace internal {
        
        // Assert implementation.
        void cppassert(const std::string& expression, std::source_location source, const std::string& message) {
            logging::fatal("Assertion '{}' failed in {}:{}: {}", expression, source.file_name(), source.line(), message);
        }
        
    }
    
    void cppassert(const std::string& expression, bool result, std::source_location source, const std::string& message) {
        if (!result) {
            internal::cppassert(expression, source, message);
        }
    }
    
}