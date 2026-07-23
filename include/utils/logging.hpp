
#pragma once

#ifndef LOGGING_HPP
#define LOGGING_HPP

#include "utils/string.hpp"

namespace utils {

    namespace logging {

        template <typename ...Args>
        void info(FormatString fmt, Args&&... args);

        template <typename ...Args>
        void debug(FormatString fmt, Args&&... args);

        template <typename ...Args>
        void warning(FormatString fmt, Args&&... args);

        template <typename ...Args>
        void error(FormatString fmt, Args&&... args);

        // Logs an error message and raises a std::runtime_error
        template <typename ...Args>
        [[noreturn]] void fatal(FormatString fmt, Args&&... args);

    }

}

// Template definitions
#include "utils/detail/logging.tpp"

#endif // LOGGING_HPP
