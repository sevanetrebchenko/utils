

#ifndef UTILS_LOGGING_TPP
#define UTILS_LOGGING_TPP

#include <spdlog/spdlog.h>

namespace utils::logging {

    namespace detail {

        spdlog::logger& logger();

    }

    template <typename ...Args>
    void info(FormatString fmt, const Args&... args) {
        spdlog::logger& logger = detail::logger();
        logger.log(spdlog::source_loc(fmt.source.file_name(), static_cast<int>(fmt.source.line()), fmt.source.function_name()), spdlog::level::info, fmt::runtime(fmt.format), args...);
    }

    template <typename ...Args>
    void debug(FormatString fmt, const Args&... args) {
        spdlog::logger& logger = detail::logger();
        logger.log(spdlog::source_loc(fmt.source.file_name(), static_cast<int>(fmt.source.line()), fmt.source.function_name()), spdlog::level::debug, fmt::runtime(fmt.format), args...);
    }

    template <typename ...Args>
    void warning(FormatString fmt, const Args&... args) {
        spdlog::logger& logger = detail::logger();
        logger.log(spdlog::source_loc(fmt.source.file_name(), static_cast<int>(fmt.source.line()), fmt.source.function_name()), spdlog::level::warn, fmt::runtime(fmt.format), args...);
    }

    template <typename ...Args>
    void error(FormatString fmt, const Args&... args) {
        spdlog::logger& logger = detail::logger();
        logger.log(spdlog::source_loc(fmt.source.file_name(), static_cast<int>(fmt.source.line()), fmt.source.function_name()), spdlog::level::err, fmt::runtime(fmt.format), args...);
    }

    template <typename ...Args>
    [[noreturn]] void fatal(FormatString fmt, const Args&... args) {
        std::string message = utils::format(fmt, args...);
        spdlog::logger& logger = detail::logger();
        logger.log(spdlog::source_loc(fmt.source.file_name(), static_cast<int>(fmt.source.line()), fmt.source.function_name()), spdlog::level::err, message);
        throw std::runtime_error(message);
    }

}

#endif  // UTILS_LOGGING_TPP
