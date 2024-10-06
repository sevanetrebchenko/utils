
#include "utils/string.hpp"
#include "utils/logging.hpp"
#include "utils/assert.hpp"

#include <source_location>
#include <iostream>
#if defined(_WIN32) || defined(_WIN64)
    #include <windows.h>
#else
    #include <unistd.h> // isatty, getpid
#endif

namespace utils {
    
    namespace logging {

        struct ConsoleSink : public Sink {
            explicit ConsoleSink(FILE* file);
            ~ConsoleSink() override;
            
            void log(std::string_view message, std::optional<Color> color) override;
            void flush() override;
            
            bool supports_colored_output;
            FILE* file;
        };
        
        ConsoleSink::ConsoleSink(FILE* file) : supports_colored_output(false),
                                               file(file) {
            #if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
                // Windows
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
                supports_colored_output |= SetConsoleMode(console, mode);
            #elif defined(__unix__) || defined(__unix) || defined(__linux__) || defined(__APPLE__) || defined(__MACH__)
                // Linux / Mac OS
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
            #else
                #error "unknown platform!"
            #endif
        }
        
        void ConsoleSink::log(std::string_view message, std::optional<Color> color) {
            if (!supports_colored_output) {
                fwrite(0, sizeof(char), 5, file);
                fwrite(message.data(), sizeof(char), message.length(), file);
            }
            else {
                fwrite(message.data(), sizeof(char), message.length(), file);
            }
        }
        
        void ConsoleSink::flush() {
            fflush(file);
        }
        
        ConsoleSink::~ConsoleSink() {
            ConsoleSink::flush();
        }

//
//            Adapter::Adapter(std::string format, Message::Level level)  : level(level),
//                                                                          format(std::move(format)) {
//            }
//
//            Adapter::~Adapter() = default;
//
//            void Adapter::log(const utils::logging::Message& message) {
//                if (message.level < level) {
//                    return;
//                }
//
//                std::lock_guard guard { lock };
//                log(utils::format(std::string_view(format), NamedArgument("message", message.message),
//                                                            NamedArgument("level", message.level),
//                                                            NamedArgument("date", message.timestamp.date),
//                                                            NamedArgument("day", message.timestamp.date.day),
//                                                            NamedArgument("month", message.timestamp.date.month),
//                                                            NamedArgument("year", message.timestamp.date.year),
//                                                            NamedArgument("time", message.timestamp.time)));
//            }
//
//            void Adapter::remove_ansi_color_codes(std::string& out) const {
//                std::size_t length = out.length();
//                std::size_t write_position = 0;
//                size_t i = 0;
//
//                while (i < length) {
//                    // A valid ANSI color code starts with '\033[' and ends with 'm'
//                    if (out[i] == '\27' && (i + 1 < length) && out[i + 1] == '[') {
//                        // Skip '\033['
//                        i += 2;
//
//                        // Skip until the color code terminator 'm'
//                        while (i < length && out[i] != 'm') {
//                            i++;
//                        }
//
//                        // Skip the 'm' character itself
//                        i++;
//                    }
//                    else {
//                        out[write_position++] = out[i++];
//                    }
//                }
//
//                out.resize(write_position);
//            }
//
//            class FileAdapter : public Adapter {
//                public:
//                    FileAdapter(FILE* stream, std::string format, Message::Level level);
//                    FileAdapter(const std::filesystem::path& filepath, std::ios::openmode open_mode, std::string format, Message::Level level);
//                    ~FileAdapter() override;
//
//                protected:
//                    bool is_standard_stream() const;
//                    FILE* file;
//
//                private:
//                    void log(std::string message) override;
//                    const char* convert_to_c_open_mode(std::ios::openmode mode) const;
//            };
//
//            FileAdapter::FileAdapter(FILE* file, std::string format, Message::Level level) : Adapter(std::move(format), level),
//                                                                                             file(file) {
//                ASSERT(is_standard_stream(), "stream FileAdapter constructor must take a standard output stream");
//            }
//
//            FileAdapter::FileAdapter(const std::filesystem::path& filepath, std::ios::openmode mode, std::string format, Message::Level level) : Adapter(std::move(format), level) {
//                if (!std::filesystem::create_directories(filepath)) {
//                    throw std::runtime_error(utils::format("failed to create directories for path '{}'", filepath.parent_path()));
//                }
//
//                const std::string& path = filepath.string();
//
//                // fopen creates a new file if it does not exist
//                file = fopen(path.c_str(), convert_to_c_open_mode(mode));
//                if (!file) {
//                    throw std::runtime_error(utils::format("failed to open file '{}'", path));
//                }
//            }
//
//            FileAdapter::~FileAdapter() {
//                fflush(file);
//                if (!is_standard_stream()) {
//                    fclose(file);
//                }
//            }
//
//            bool FileAdapter::is_standard_stream() const {
//                return file == stdout || file == stderr;
//            }
//
//            void FileAdapter::log(std::string message) {
//                remove_ansi_color_codes(message);
//                fwrite(message.data(), sizeof(char), message.length(), file);
//            }
//
//            const char* FileAdapter::convert_to_c_open_mode(std::ios::openmode mode) const {
//                bool read = mode & std::ios::in;
//                bool write = mode & std::ios::out;
//                bool append = mode & std::ios::app;
//                bool truncate = mode & std::ios::trunc;
//                bool binary = mode & std::ios::binary;
//                if (write) {
//                    if (append) {
//                        return binary ? "ab" : "a";
//                    }
//                    if (read) {
//                        if (truncate) {
//                            return binary ? "wb+" : "w+";
//                        }
//
//                        return binary ? "rb+" : "r+";
//                    }
//                    return binary ? "wb" : "w";
//                }
//
//                if (read) {
//                    return binary ? "rb" : "r";
//                }
//
//                return nullptr;
//            }
//
//            class ConsoleAdapter : public FileAdapter {
//                public:
//                    ConsoleAdapter(FILE* stream, std::string format, Message::Level level);
//                    ~ConsoleAdapter() override = default;
//
//                private:
//                    void log(std::string message) override;
//                    bool enable_colored_output() const;
//            };
//
//            ConsoleAdapter::ConsoleAdapter(FILE* stream, std::string format, Message::Level level) : FileAdapter(stream, std::move(format), level) {
//            }
//
//            void ConsoleAdapter::log(std::string message) {
//                static bool has_color_support = enable_colored_output();
//                if (!has_color_support) {
//                    remove_ansi_color_codes(message);
//                }
//
//                fwrite(message.data(), sizeof(char), message.length(), file);
//            }
            
            // Returns whether console supports colored output
            bool ConsoleAdapter::enable_colored_output() const {
                #if defined(_WIN32) || defined(_WIN64)
                    // Windows
                    
                    HANDLE console;
                    if (file == stdout) {
                        console = GetStdHandle(STD_OUTPUT_HANDLE);
                    }
                    else {
                        ASSERT(file == stderr, "unknown standard stream");
                        console = GetStdHandle(STD_ERROR_HANDLE);
                    }
                    
                    DWORD mode;
                    if (!GetConsoleMode(console, &mode)) {
                        return false;
                    }
                    
                    // Enable virtual terminal processing if supported
                    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
                    return SetConsoleMode(console, mode);
                #else
                    // Unix
                    throw std::runtime_error("unimplemented error");
                #endif
            }
            
            struct Logger {
                std::vector<Adapter*> adapters;
            };
            
            thread_local std::vector<std::string> scopes;
            
            void log(const Message& message) {
                static Logger logger { };
                std::cout << utils::format("[{level}] - {message}, from {source}", NamedArgument("level", message.level), NamedArgument("message", message.message), NamedArgument("source", message.source)) << '\n';
            }
    
        }
        
        std::string default_format = "";
        
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
        
    }
    
    void set_format(std::string_view fmt) {
    }
    
    void set_format(const char* fmt) {
    }
    
    void clear_format() {
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
