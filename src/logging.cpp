
#include "utils/logging.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/logger.h>

namespace utils::logging {

    namespace detail {

        spdlog::logger create_default_logger() {
            std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            stdout_sink->set_level(spdlog::level::trace); // Every level is written to stdout

            std::shared_ptr<spdlog::sinks::stderr_color_sink_mt> stderr_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
            stderr_sink->set_level(spdlog::level::err); // Only errors are mirrored to stderr

            spdlog::logger logger("lightswitch", { stdout_sink, stderr_sink });
            logger.set_level(spdlog::level::trace);
            logger.set_pattern("[%l] %v");

            return logger;
        }

        spdlog::logger& logger() {
            static spdlog::logger instance = create_default_logger();
            return instance;
        }

    }

}
