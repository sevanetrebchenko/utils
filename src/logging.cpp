
#include "utils/string.hpp"
#include "utils/exceptions.hpp"
#include "utils/logging.hpp"
#include "utils/assert.hpp"
#include "utils/platform.hpp"

#include <source_location>
#include <iostream>
#if defined(PLATFORM_WINDOWS)
    #include <Windows.h>
#else
    #include <unistd.h> // getpid
#endif

namespace utils {
    namespace logging {
        
        class Logger {
            public:
                static Logger& instance();
                
                ~Logger();
                void set_default_format(std::string format);
                [[nodiscard]] std::string get_default_format();

                void add_sink(const std::shared_ptr<Sink>& sink);
                [[nodiscard]] std::shared_ptr<Sink> get_sink(std::string_view name);
                void remove_sink(std::string_view name);
                
                void log(Message& message);
                
            private:
                Logger();
                
                std::mutex m_format_lock;
                std::string m_format;
                
                std::mutex m_sink_lock;
                std::vector<std::shared_ptr<Sink>> m_sinks;
        };
        
        struct ConsoleSink : public Sink {
            explicit ConsoleSink(FILE* file);
            ~ConsoleSink() override;
            
            void log(std::string_view message, const Message& data) override;
            void flush() override;
            
            bool supports_colored_output;
            FILE* file;
        };
        
        thread_local std::vector<std::string> scopes;
        
        Logger& Logger::instance() {
            static Logger instance { };
            return instance;
        }
        
        Logger::Logger() : m_format("[{level}] {message}") {
            std::scoped_lock guard { m_format_lock, m_sink_lock };
            
            // Create standard output (stdout) and standard error (stderr) sinks
            std::shared_ptr<Sink> out = std::make_shared<ConsoleSink>(stdout);
            out->set_format(m_format);
            
            m_sinks.emplace_back(out);
            
            std::shared_ptr<Sink> err = std::make_shared<ConsoleSink>(stderr);
            err->set_format(m_format);
            
            m_sinks.emplace_back(err);
        }
        
        Logger::~Logger() = default;

        void Logger::set_default_format(std::string format) {
            std::lock_guard guard { m_format_lock };
            m_format = std::move(format);
        }
        
        std::string Logger::get_default_format() {
            std::lock_guard guard { m_format_lock };
            return m_format;
        }
        
        void Logger::add_sink(const std::shared_ptr<Sink>& sink) {
            std::lock_guard guard { m_sink_lock };
            
            // Do not add multiple sinks with the same name
            for (const std::shared_ptr<Sink>& s : m_sinks) {
                if (s->get_name() == sink->get_name()) {
                    return;
                }
            }
            
            m_sinks.emplace_back(sink);
        }
        
        std::shared_ptr<Sink> Logger::get_sink(std::string_view name) {
            std::lock_guard guard { m_sink_lock };
            
            for (const std::shared_ptr<Sink>& sink : m_sinks) {
                if (sink->get_name() == name) {
                    return sink;
                }
            }
            return nullptr;
        }
        
        void Logger::remove_sink(std::string_view name) {
            std::lock_guard guard { m_sink_lock };
            
            for (auto iter = m_sinks.begin(); iter != m_sinks.end(); ++iter) {
                if ((*iter)->get_name() == name) {
                    m_sinks.erase(iter);
                    break;
                }
            }
        }
        
        void Logger::log(Message& message) {
            message.thread_id = std::this_thread::get_id();
            #if defined(PLATFORM_WINDOWS)
                message.process_id = GetCurrentProcessId();
            #else
                message.process_id = getpid();
            #endif
            message.scope = scopes;
            
            std::lock_guard guard { m_sink_lock };
            for (std::shared_ptr<Sink>& sink : m_sinks) {
                sink->log(message);
            }
        }
        
        // Sink format string is set later to avoid a deadlock with Logger initialization
        ConsoleSink::ConsoleSink(FILE* file) : Sink(file == stdout ? "stdout" : "stderr", "", file == stdout ? Message::Level::Debug : Message::Level::Error),
                                               supports_colored_output(false),
                                               file(file) {
            ASSERT(file == stdout || file == stderr, "ConsoleSink must redirect output to standard streams");
            #if defined(PLATFORM_WINDOWS)
                HANDLE console;
                if (file == stdout) {
                    console = GetStdHandle(STD_OUTPUT_HANDLE);
                }
                else {
                    console = GetStdHandle(STD_ERROR_HANDLE);
                }
                
                DWORD mode;
                if (!GetConsoleMode(console, &mode)) {
                    return;
                }
                
                // Enable virtual terminal processing if supported
                mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                supports_colored_output |= (bool) SetConsoleMode(console, mode);
            #else
                // Based on https://github.com/agauniyal/rang/blob/master/include/rang.hpp
                const char* terminals[] = {
                    "ansi", "color", "console", "cygwin", "gnome", "konsole", "kterm", "linux", "msys", "putty", "rxvt", "screen", "vt100", "xterm"
                };
                const char* env = std::getenv("TERM");
                if (env) {
                    for (const char* terminal : terminals) {
                        if (std::strstr(env, terminal)) {
                            supports_colored_output = true;
                            break;
                        }
                    }
                }
            #endif
        }
        
        void ConsoleSink::log(std::string_view message, const Message& data) {
            if (supports_colored_output && data.level != Message::Level::Info) {
                switch (data.level) {
                    case Message::Level::Debug:
                        // Debug messages are printed in bright black (gray)
                        fwrite("\033[38;5;8m", sizeof(char), 9, file);
                        break;
                    case Message::Level::Warning:
                        // Warning messages are printed in bright yellow
                        fwrite("\033[38;5;11m", sizeof(char), 10, file);
                        break;
                    case Message::Level::Error:
                        // Error messages are printed in bright red
                        fwrite("\033[38;5;9m", sizeof(char), 9, file);
                        break;
                }
                
                fwrite(message.data(), sizeof(char), message.length(), file);
                fwrite("\x1b[0m", sizeof(char), 4, file);
            }
            else {
                // Info messages get printed using the default color
                fwrite(message.data(), sizeof(char), message.length(), file);
            }
            
            fwrite("\n", sizeof(char), 1, file);
        }
        
        void ConsoleSink::flush() {
            fflush(file);
        }
        
        ConsoleSink::~ConsoleSink() {
            // Standard streams should not be closed at the end of the program
            ConsoleSink::flush();
        }
        
        Message::Message(const std::string& fmt, std::source_location source) : level(Level::Debug),
                                                                                format(fmt),
                                                                                source(source),
                                                                                message(),
                                                                                timestamp(Timestamp::now()) {
        }
        
        Message::Message(std::string_view fmt, std::source_location source) : level(Level::Debug),
                                                                              format(fmt),
                                                                              source(source),
                                                                              message(),
                                                                              timestamp(Timestamp::now()) {
        }
        
        Message::Message(const char* fmt, std::source_location source) : level(Level::Debug),
                                                                         format(fmt),
                                                                         source(source),
                                                                         message(),
                                                                         timestamp(Timestamp::now()) {
        }
        
        Message::~Message() = default;
        
        void push_scope(std::string name) {
            scopes.emplace_back(std::move(name));
        }
        
        void pop_scope() {
            scopes.pop_back();
        }
        
        Sink::Sink(std::string name, std::optional<std::string> format, Message::Level level) : m_name(std::move(name)),
                                                                                                m_level(level),
                                                                                                m_lock(),
                                                                                                m_format(format ? std::move(*format) : Logger::instance().get_default_format()),
                                                                                                m_enabled(true) {
        }
        
        Sink::~Sink() = default;
        
        void Sink::log(const Message& data) {
            if (!m_enabled) {
                return;
            }
            
            if (data.level < m_level) {
                return;
            }
            
            std::lock_guard guard { m_lock };
            std::string message = utils::format(m_format, NamedArgument("message", data.message),
                                                          NamedArgument("level", data.level),
                                                          NamedArgument("timestamp", data.timestamp),
                                                          NamedArgument("date", data.timestamp.date),
                                                          NamedArgument("day", data.timestamp.date.day),
                                                          NamedArgument("month", data.timestamp.date.month),
                                                          NamedArgument("year", data.timestamp.date.year),
                                                          NamedArgument("time", data.timestamp.time),
                                                          NamedArgument("hour", data.timestamp.time.hour),
                                                          NamedArgument("minute", data.timestamp.time.minute),
                                                          NamedArgument("second", data.timestamp.time.second),
                                                          NamedArgument("millisecond", data.timestamp.time.millisecond),
                                                          NamedArgument("source", data.source),
                                                          NamedArgument("filename", data.source.file_name()),
                                                          NamedArgument("line", data.source.line()),
                                                          NamedArgument("thread_id", data.thread_id),
                                                          NamedArgument("tid", data.thread_id),
                                                          NamedArgument("process_id", data.process_id),
                                                          NamedArgument("pid", data.process_id));
            log(message, data);
        }
        
        void Sink::set_format(const std::string& format) {
            m_format = format.empty() ? Logger::instance().get_default_format() : format;
        }
        
        void Sink::reset_format() {
            m_format = Logger::instance().get_default_format();
        }
        
        void Sink::set_level(Message::Level level) {
            m_level = level;
        }
        
        std::string_view Sink::get_name() const {
            return m_name;
        }
        
        void Sink::enable() {
            m_enabled = true;
        }
        
        void Sink::disable() {
            m_enabled = false;
        }
        
        void Sink::flush() {
        }
        
        FileSink::FileSink(const std::filesystem::path& filepath, std::ios::openmode open_mode, std::optional<std::string> format, Message::Level level) : Sink(filepath.stem().string(), std::move(format), level) {
            if (!std::filesystem::create_directories(filepath)) {
                throw std::runtime_error(utils::format("failed to create directories for path '{}'", filepath.parent_path()));
            }
    
            m_file = std::ofstream(filepath, open_mode);
            if (!m_file.is_open()) {
                throw std::runtime_error(utils::format("failed to open file '{}'", filepath));
            }
        }
        
        FileSink::~FileSink() = default;
        
        void FileSink::log(std::string_view message, const Message& data) {
            m_file << message << '\n';
        }
        
        void FileSink::flush() {
            m_file.flush();
        }
        
        void set_default_format(std::string format) {
            if (format.empty()) {
                return;
            }
            Logger::instance().set_default_format(std::move(format));
        }
        
        void destroy_sink(std::string_view name) {
            Logger::instance().remove_sink(name);
        }
        
        std::shared_ptr<Sink> get_sink(std::string_view name) {
            return Logger::instance().get_sink(name);
        }
        
        namespace detail {
            
            void log(Message& message) {
                Logger::instance().log(message);
            }
            
            void add_sink(const std::shared_ptr<Sink>& sink) {
                Logger& logger = Logger::instance();
                std::string_view name = sink->get_name();
                
                if (logger.get_sink(name)) {
                    throw FormattedError("failed to register sink - sink with name '{}' already exists", name);
                }
                
                logger.add_sink(sink);
            }
            
        }
    }
    
    Formatter<logging::Message::Level>::Formatter() : Formatter<const char*>(),
                                                      uppercase(true) {
    }
    
    Formatter<logging::Message::Level>::~Formatter() = default;
    
    void Formatter<logging::Message::Level>::parse(const FormatSpec& spec) {
        ASSERT(spec.type() == FormatSpec::Type::SpecifierList, "format spec for Message::Level must be a specifier list");
        Formatter<const char*>::parse(spec);
        
        if (spec.has_specifier("uppercase", "lowercase")) {
            FormatSpec::SpecifierView specifier = spec.get_specifier("uppercase", "lowercase");
            std::string_view value = trim(specifier.value);
            
            if (icasecmp(specifier.name, "uppercase")) {
                if (icasecmp(value, "true") || icasecmp(value, "1")) {
                    uppercase = true;
                }
            }
            else { // if (icasecmp(specifier.name, "lowercase")) {
                if (icasecmp(value, "true") || icasecmp(value, "1")) {
                    uppercase = false;
                }
            }
        }
    }
    
    std::string Formatter<logging::Message::Level>::format(logging::Message::Level level) const {
        using namespace logging;
        
        const char* str;
        if (uppercase) {
            if (level == Message::Level::Debug) {
                str = "DEBUG";
            }
            else if (level == Message::Level::Info) {
                str = "INFO";
            }
            else if (level == Message::Level::Warning) {
                str = "WARNING";
            }
            else { // if (level == Message::Level::Error) {
                str = "ERROR";
            }
        }
        else {
            if (level == Message::Level::Debug) {
                str = "debug";
            }
            else if (level == Message::Level::Info) {
                str = "info";
            }
            else if (level == Message::Level::Warning) {
                str = "warning";
            }
            else { // if (level == Message::Level::Error) {
                str = "error";
            }
        }
        
        return Formatter<const char*>::format(str);
    }
    
}
