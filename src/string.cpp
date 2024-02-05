
#include "utils/string.hpp"
#include "utils/result.hpp"
#include "utils/internal/string.tpp"

#include <limits>
#include <charconv>

namespace utils {
    namespace internal {
        
        Result<Placeholder::Identifier> Placeholder::Identifier::parse(std::string_view identifier) noexcept {
            if (identifier.empty()) {
                // Detected auto-numbered placeholder - {}.
                return Result<Identifier> { };
            }
            else {
                // A placeholder identifier should not have any whitespace characters.
                for (unsigned i = 0u; i < identifier.length(); ++i) {
                    if (std::isspace(identifier[i])) {
                        return Result<Identifier>::NOT_OK("whitespace character at index {}", i);
                    }
                }
                
                // Note: regex_match checks the entire input string. This behaviour can be simulated by using
                // regex search and anchoring the input string by using '^' (start) and '$' (end), but has been
                // omitted for simplicity.
                
                // Determine if placeholder is positional or named.
                if (std::regex_match(identifier.begin(), identifier.end(), std::regex("[0-9]+"))) {
                    // Positional placeholders can only be positive integers.
                    int position;
                    
                    const char* start = identifier.data();
                    const char* end = start + identifier.length();
                    std::from_chars_result conversion_result = std::from_chars(start, end, position, 10);
                    
                    // Regex check asserts that std::from_chars(...) will only return std::errc::result_out_of_range if the position value exceeds that of an integer.
                    // Note that this value is ~2.14 billion and should be considered a hard limit on the number of positional arguments for a single format string.
                    if (conversion_result.ec == std::errc::result_out_of_range) {
                        return Result<Identifier>::NOT_OK("positional placeholder value '{}' is out of range", identifier);
                    }
                    return Result<Identifier>(Identifier(position));
                }
                else if (std::regex_match(identifier.begin(), identifier.end(), std::regex("[a-zA-Z_]\\w*"))) {
                    // Named placeholders follow the same naming convention as C++ identifiers:
                    //  - start with a letter or underscore
                    //  - followed by any combination of letters, digits, or underscores (\w)
                    return Result<Identifier>(Identifier(std::string(identifier)));
                }
                else {
                    return Result<Identifier>::NOT_OK("named placeholder identifier '{}' is not valid", identifier);
                }
            }
        }
        
        Placeholder::Identifier::Identifier(std::string_view in) {
            Result<Identifier> result = parse(in);
            if (!result.ok()) {
                throw std::invalid_argument(format("error parsing identifier '{}' - {}", in, result.what()));
            }
            *this = *result;
        }
        
        Placeholder::Identifier::Identifier() : type(Type::None),
                                                position(-1),
                                                name()
                                                {
        }
        
        Placeholder::Identifier::Identifier(int position) : type(Type::Position),
                                                            position(position),
                                                            name()
                                                            {
        }
        
        Placeholder::Identifier::Identifier(std::string name) : type(Type::Name),
                                                                position(-1),
                                                                name(std::move(name))
                                                                {
        }
        
        Placeholder::Identifier::~Identifier()= default;
        
        Placeholder::Formatting::Formatting() : justification(Right),
                                                representation(Decimal),
                                                sign(NegativeOnly),
                                                fill(' '),
                                                separator('\0'), // empty
                                                width(std::numeric_limits<unsigned>::max()),
                                                precision(8)
                                                {
        }
        
        Placeholder::Formatting::~Formatting() = default;
        
        Placeholder::Formatting::Formatting(std::string_view in) {
            Result<Formatting> result = parse(in);
            if (!result.ok()) {
                throw std::invalid_argument(format("error parsing placeholder format specifiers '{}' - {}", in, result.what()));
            }
            *this = *result;
        }
        
        Result<Placeholder::Formatting> Placeholder::Formatting::parse(std::string_view specifiers) noexcept {
            return Result<Formatting> { };
        }
        
        Placeholder::Placeholder(std::string_view in) : identifier(),
                                                        formatting()
                                                        {
            Result<Placeholder> result = parse(in);
            if (!result.ok()) {
                throw std::invalid_argument(format("error parsing placeholder '{}' - {}", in, result.what()));
            }
            *this = *result;
        }
        
        Result<Placeholder> Placeholder::parse(std::string_view in) noexcept {
            if (in.empty() || (in[0] != '{' || in[in.length() - 1u] != '}')) {
                // A placeholder must, at minimum, contain an opening and closing brace ({}).
                return Result<Placeholder>::NOT_OK("missing opening/terminating placeholder brace(s)");
            }
            
            // Remove opening / closing brace.
            in = in.substr(1).substr(0, in.length() - 2u);
            Result<Placeholder> placeholder { };
            
            // Split by the first ':' character, if present.
            // This approach has the added benefit of catching any extra ':' characters in the placeholder during parsing.
            std::size_t split_position = in.find(':');
            
            {
                Result<Placeholder::Identifier> result = Placeholder::Identifier::parse(in.substr(0, split_position));
                if (!result.ok()) {
                    return Result<Placeholder>::NOT_OK(result.what());
                }
                placeholder->identifier = *result;
            }

            // Formatting specifiers are optional.
            if (split_position != std::string::npos) {
                Result<Placeholder::Formatting> result = Placeholder::Formatting::parse(in.substr(split_position + 1));
                if (!result.ok()) {
                    return Result<Placeholder>::NOT_OK(result.what());
                }
                placeholder->formatting = *result;
            }
            
            return placeholder;
        }
        
        Placeholder::Placeholder() = default;
        
        Placeholder::~Placeholder() = default;
        
        
        // FormatString implementation
        FormatString::FormatString(std::string_view in) {
            Result<FormatString> result = parse(in);
            if (!result.ok()) {
                throw std::runtime_error(format("error encountered while parsing format string - {}", result.what()));
            }
            *this = *result;
        }
        
        Result<FormatString> FormatString::parse(std::string_view in) {
            // To save on processing power, resulting string is only updated when a placeholder is encountered.
            Result<FormatString> format_string { };
            
            bool processing_placeholder = false;
            std::size_t placeholder_start;
            processing_placeholder = false;
            
            std::size_t last_update_position = 0u;
            
            for (std::size_t i = 0u; i < in.length(); ++i) {
                bool is_last_character = (i == in.length() - 1u);
                
                if (processing_placeholder) {
                    if (in[i] == '}') {
                        // Substring to get only the placeholder (including opening / closing braces).
                        Result<Placeholder> placeholder = Placeholder::parse(in.substr(placeholder_start, i - placeholder_start + 1u));
                        if (!placeholder.ok()) {
                            return Result<FormatString>::NOT_OK("error parsing format string - {}", placeholder.what());
                        }
                        
                        format_string->m_placeholders.emplace_back(*placeholder);
                        format_string->m_insert_positions.emplace_back(i);
                    }
                }
                else {
                    if (in[i] == '{') {
                        if (!is_last_character && in[i + 1u] == '{') {
                            // Escaped '{' character.
                            format_string->m_format.append(in.substr(last_update_position, i - last_update_position));
                            last_update_position = i + 1u;
                        }
                        else {
                            processing_placeholder = true;
                            placeholder_start = i;
                        }
                    }
                    else if (in[i] == '}') {
                        if (i == 0u) {
                            // return Result<FormatString>::NOT_OK("");
                        }
                        else if (in[i - 1u] == '}') {
                            // Escaped '}' character.
                            format_string->m_format.append(in.substr(last_update_position, i - last_update_position));
                            last_update_position = i + 1u;
                        }
                    }
                }
            }
            
            if (processing_placeholder) {
            }
            
            return format_string;
        }
        
        FormatString::FormatString() = default;
        
        FormatString::~FormatString() = default;
        
    }
    
    
    
    
    [[nodiscard]] std::vector<std::string> split(const std::string& in, const std::string& delimiter) {
        std::vector<std::string> components { };
        
        std::string_view src(in); // readonly

        std::size_t position;
        do {
            position = src.find(delimiter);
            components.emplace_back(src.substr(0, position));
            src = src.substr(position + delimiter.length());
        }
        while (position != std::string::npos);
    
        return std::move(components);
    }
    
    [[nodiscard]] std::string join(const std::initializer_list<std::string>& components, const std::string& glue) {
         return join<std::initializer_list<std::string>>(components, glue);
    }
    
    [[nodiscard]] std::string trim(const std::string& in) {
        static const char* ws = " \t\n\r";
        return in.substr(in.find_first_not_of(ws), in.length() - (in.find_last_not_of(ws) + 1));
    }

    arg::~arg() = default;

    arg::operator std::string() const {
        return value;
    }
    
    
}