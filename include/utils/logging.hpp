
#ifndef UTILS_LOGGING_HPP
#define UTILS_LOGGING_HPP

#include "utils/string.hpp"
#include <spdlog/fwd.h>
#include <memory>

namespace utils {

    namespace logging {

        // Sets the pattern used to format every subsequent log message
        void set_pattern(std::string_view pattern);

        void add_sink(std::shared_ptr<spdlog::sinks::sink> sink);
        void remove_sink(std::shared_ptr<spdlog::sinks::sink> sink);

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
