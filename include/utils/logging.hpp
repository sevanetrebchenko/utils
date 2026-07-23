
#ifndef UTILS_LOGGING_HPP
#define UTILS_LOGGING_HPP

#include "utils/string.hpp"

namespace utils {

    namespace logging {

        template <typename ...Args>
        void info(FormatString fmt, const Args&... args);

        template <typename ...Args>
        void debug(FormatString fmt, const Args&... args);

        template <typename ...Args>
        void warning(FormatString fmt, const Args&... args);

        template <typename ...Args>
        void error(FormatString fmt, const Args&... args);

        // Logs an error message and raises a std::runtime_error
        template <typename ...Args>
        [[noreturn]] void fatal(FormatString fmt, const Args&... args);

    }

}

// Template definitions
#include "utils/detail/logging.tpp"

#endif // UTILS_LOGGING_HPP
