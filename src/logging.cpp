
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
                
                void log(const Message& message);
                
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
            
            void log(std::string_view message, std::optional<Style> style, std::optional<Color> color) override;
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
        
        void Logger::log(const Message& message) {
            std::lock_guard guard { m_sink_lock };
            for (std::shared_ptr<Sink>& sink : m_sinks) {
                sink->log(message);
            }
        }
        
        // Sink format string is set later to avoid a deadlock with Logger initialization
        ConsoleSink::ConsoleSink(FILE* file) : Sink(file == stdout ? "stdout" : "stderr", "", file == stdout ? Message::Level::Info : Message::Level::Error),
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
            
            if (supports_colored_output) {
                disable_structured_styling();
            }
        }
        
        void ConsoleSink::log(std::string_view message, std::optional<Style> style, std::optional<Color> color) {
            // Messages logged through the ConsoleSink already contain ANSI characters, as this is the default color / style formatting option
            // This is explicitly enabled with the call to disable_structured_styling in the constructor
            // If the targeted console does not support ANSI codes for color / styling, they are parsed out into structured parameters in Sink::log, which allows the ConsoleSink::log function to ignore them and only print the log message
            fwrite(message.data(), sizeof(char), message.length(), file);
        }
        
        void ConsoleSink::flush() {
            fflush(file);
        }
        
        ConsoleSink::~ConsoleSink() {
            // Standard streams should not be closed at the end of the program
            ConsoleSink::flush();
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
                                                                                                m_parse_ansi_codes(true),
                                                                                                m_enabled(true) {
        }
        
        Sink::~Sink() = default;
        
        void Sink::log(const Message& message) {
            if (!m_enabled) {
                return;
            }
            
            if (message.level < m_level) {
                return;
            }

            std::thread::id thread_id = std::this_thread::get_id();

            #if defined(PLATFORM_WINDOWS)
                DWORD process_id = GetCurrentProcessId();
            #else
                pid_t process_id = getpid();
            #endif
            
            std::lock_guard guard { m_lock };
            std::string formatted = utils::format(m_format, NamedArgument("message", message.message),
                                                            NamedArgument("level", message.level),
//                                                            NamedArgument("date", message.timestamp.date),
                                                            NamedArgument("day", message.timestamp.date.day),
//                                                            NamedArgument("month", message.timestamp.date.month),
                                                            NamedArgument("year", message.timestamp.date.year),
//                                                            NamedArgument("time", message.timestamp.time),
                                                            NamedArgument("hour", message.timestamp.time.hour),
                                                            NamedArgument("minute", message.timestamp.time.minute),
                                                            NamedArgument("second", message.timestamp.time.second),
                                                            NamedArgument("millisecond", message.timestamp.time.millisecond),
                                                            NamedArgument("filename", message.source.file_name()),
                                                            NamedArgument("source", message.source),
                                                            NamedArgument("line", message.source.line()),
                                                            NamedArgument("thread_id", thread_id),
                                                            NamedArgument("tid", thread_id),
                                                            NamedArgument("process_id", process_id),
                                                            NamedArgument("pid", process_id));
            
            if (!m_parse_ansi_codes) {
                // Parsing of ANSI codes explicitly disabled, pass the log message through unmodified
                log(formatted, { }, { });
                return;
            }

            using Formatting = std::pair<Style, std::optional<Color>>;
            std::stack<Formatting> formatting;
            
            std::size_t last_read_position = 0;

            while (last_read_position < formatted.length()) {
                // Styling parameters are specified using ANSI escape codes
                
                // Find the start of the next ANSI escape sequence
                std::size_t start = formatted.find("\x1b[", last_read_position);
                if (start == std::string::npos) {
                    break;
                }
                
                // Print string contents before the start of the escape sequence (no color / style applied)
                log(formatted.substr(last_read_position, start - last_read_position), { }, { });
                
                // Find the end of the escape sequence, denoted by an 'm'
                std::size_t end = formatted.find('m', start);
                if (end == std::string::npos) {
                    throw FormattedError("unterminated ANSI escape sequence at position {}", start);
                }

                std::string_view sequence = std::string_view(formatted).substr(start + 2, end - start - 2);
                std::size_t offset = 0;
                
                std::optional<Style> style { };
                std::optional<Color> color { };
                
                // Parse individual codes separated by ';'
                while (offset < sequence.length()) {
                    std::size_t next = sequence.find(';', offset);
                    std::string_view code = sequence.substr(offset, next - offset);
                    
                    if (code == "1") {
                        // Bold
                        style = Style::Bold;
                    }
                    else if (code == "3") {
                        // Italicized
                        style = Style::Italicized;
                    }
                    else if (code == "38") {
                        // RGB color code
                        offset += 5; // Skip 38;2;
                        std::uint8_t r, g, b;
                        
                        auto parse_color_code = [&sequence, &offset](std::uint8_t& out) {
                            std::size_t pos = sequence.find(';', offset);
                            std::string_view value;
                            if (pos == std::string::npos) {
                                value = sequence.substr(offset); // No more semicolon, substring to the end
                            }
                            else {
                                value = sequence.substr(offset, pos - offset);
                            }
                            
                            std::from_chars(value.data(), value.data() + value.length(), out);

                            if (pos == std::string::npos) {
                                offset = sequence.length();
                            }
                            else {
                                offset = pos + 1; // Skip ';'
                            }
                        };
                        
                        parse_color_code(r);
                        
                        if (offset == sequence.length()) {
                            throw FormattedError("invalid ANSI escape sequence at position {} - color sequence must contain values for r, g, and b (38;2;{{r}};{{g}};{{b}}m)", start);
                        }
                        
                        parse_color_code(g);
                        
                        if (offset == sequence.length()) {
                            throw FormattedError("invalid ANSI escape sequence at position {} - color sequence must contain values for r, g, and b (38;2;{{r}};{{g}};{{b}}m)", start);
                        }
                        
                        parse_color_code(b);
                        
                        color = { r, g, b };
                    }
                }
                
                start = formatted.find("\x1b[0m", end);
                
                std::string_view value = std::string_view(formatted).substr(end + 1, start - (end + 1));
                
                last_read_position = start + 4; // Skip past
            }
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
        
        void Sink::disable_structured_styling() {
            m_parse_ansi_codes = false;
        }
        
        void Sink::flush() {
        }
        

        
        FileSink::FileSink(const std::filesystem::path& filepath, std::ios::openmode open_mode, std::optional<std::string> format, Message::Level level) : Sink(filepath.stem().string(), std::move(format), level) {
            if (!std::filesystem::create_directories(filepath)) {
                throw std::runtime_error(utils::format("failed to create directories for path '{}'", filepath.parent_path()));
            }
    
            const std::string& path = filepath.string();
    
            // fopen creates a new file if it does not exist
            m_file = fopen(path.c_str(), convert_to_c_open_mode(open_mode));
            if (!m_file) {
                throw std::runtime_error(utils::format("failed to open file '{}'", path));
            }
        }
        
        FileSink::~FileSink() {
            fclose(m_file);
        }
        
        void FileSink::log(std::string_view message, std::optional<Style> style, std::optional<Color> color) {
        }
        
        void FileSink::flush() {
            fflush(m_file);
        }
        
        const char* FileSink::convert_to_c_open_mode(std::ios::openmode open_mode) {
            bool read = open_mode & std::ios::in;
            bool write = open_mode & std::ios::out;
            bool append = open_mode & std::ios::app;
            bool truncate = open_mode & std::ios::trunc;
            bool binary = open_mode & std::ios::binary;
            if (write) {
                if (append) {
                    return binary ? "ab" : "a";
                }
                if (read) {
                    if (truncate) {
                        return binary ? "wb+" : "w+";
                    }

                    return binary ? "rb+" : "r+";
                }
                return binary ? "wb" : "w";
            }

            if (read) {
                return binary ? "rb" : "r";
            }

            return nullptr;
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
            
            void log(const Message& message) {
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
            if (level == Message::Level::Trace) {
                str = "TRACE";
            }
            else if (level == Message::Level::Debug) {
                str = "DEBUG";
            }
            else if (level == Message::Level::Info) {
                str = "INFO";
            }
            else if (level == Message::Level::Warning) {
                str = "WARNING";
            }
            else if (level == Message::Level::Error) {
                str = "ERROR";
            }
            else { // if (level == Message::Level::Fatal) {
                str = "FATAL";
            }
        }
        else {
            if (level == Message::Level::Trace) {
                str = "trace";
            }
            if (level == Message::Level::Debug) {
                str = "debug";
            }
            else if (level == Message::Level::Info) {
                str = "info";
            }
            else if (level == Message::Level::Warning) {
                str = "warning";
            }
            else if (level == Message::Level::Error) {
                str = "error";
            }
            else { // if (level == Message::Level::Fatal) {
                str = "fatal";
            }
        }
        
        return Formatter<const char*>::format(str);
    }
    
}
