
#include "utils/assert.hpp"
#include "utils/string.hpp"

namespace utils {
    namespace internal {
        
        // Assert implementation.
        void cppassert(const std::string& expression, std::source_location source, const std::string& message) {
            // TODO: hook up with logging functionality
            std::string error = utils::format("ERROR: assertion {} failed in {}, {}:{}: {}", expression, source.file_name(), source.line(), message);
            std::cerr << error << std::endl;
            std::abort();
        }
        
    }
    
    void cppassert(const std::string& expression, bool result, std::source_location source, const std::string& message) {
        if (!result) {
            internal::cppassert(expression, source, message);
        }
    }
    
}