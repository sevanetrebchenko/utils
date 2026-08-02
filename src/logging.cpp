
#include "utils/logging.hpp"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/dist_sink.h>
#include <spdlog/logger.h>

#include <utility>

namespace utils::logging {

    namespace detail {

        std::shared_ptr<spdlog::sinks::dist_sink_mt> create_default_sink() {
            std::shared_ptr<spdlog::sinks::dist_sink_mt> sink = std::make_shared<spdlog::sinks::dist_sink_mt>();

            std::shared_ptr<spdlog::sinks::stdout_color_sink_mt> stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            stdout_sink->set_level(spdlog::level::trace);  // Everything is written to stdout
            sink->add_sink(stdout_sink);

            std::shared_ptr<spdlog::sinks::stderr_color_sink_mt> stderr_sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
            stderr_sink->set_level(spdlog::level::err);  // Only errors are written to stderr
            sink->add_sink(stderr_sink);

            return sink;
        }

        std::shared_ptr<spdlog::sinks::dist_sink_mt>& sink() {
            static std::shared_ptr<spdlog::sinks::dist_sink_mt> instance = create_default_sink();
            return instance;
        }

        spdlog::logger create_default_logger() {
            std::shared_ptr<spdlog::sinks::dist_sink_mt> sink = detail::sink();
            spdlog::logger logger("lightswitch", { sink });
            logger.set_level(spdlog::level::trace);
            logger.set_pattern("[%l] %v");
            return logger;
        }

        spdlog::logger& logger() {
            static spdlog::logger instance = create_default_logger();
            return instance;
        }

    }

    void set_pattern(std::string_view pattern) {
        spdlog::logger& logger = detail::logger();
        logger.set_pattern(std::string(pattern));
    }

    void add_sink(std::shared_ptr<spdlog::sinks::sink> s) {
        std::shared_ptr<spdlog::sinks::dist_sink_mt> sink = detail::sink();
        sink->add_sink(std::move(s));
    }

    void remove_sink(std::shared_ptr<spdlog::sinks::sink> s) {
        std::shared_ptr<spdlog::sinks::dist_sink_mt> sink = detail::sink();
        sink->remove_sink(std::move(s));
    }

}
