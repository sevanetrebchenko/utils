
#include "utils/string.hpp"
#include "utils/result.hpp"
#include "utils/internal/string.tpp"

#include <limits>

namespace utils {

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
    
    namespace internal {
        
        Result<FormatString> parse_format_string(const std::string& format_string) {
            // To save on processing power, resulting string is only updated when a placeholder is encountered.
            std::string string;
            
            bool processing_placeholder = false;
            std::size_t placeholder_start;
            processing_placeholder = false;
            
            std::size_t last_update_position = 0u;
            
            for (std::size_t i = 0u; i < format_string.length(); ++i) {
                if (processing_placeholder) {
                    if (format_string[i] == '}') {
                        // Parse the placeholder without the starting or ending braces.
                        std::string placeholder = format_string.substr(placeholder_start + 1u, i - placeholder_start - 1u);
                        
                        std::vector<std::string> components { };
                        
                        // Split by the first ':' character, if present.
                        std::size_t split_position = placeholder.find(':');
                        components.emplace_back(placeholder.substr(0, split_position));
                        if (split_position != std::string::npos) {
                            components.emplace_back(placeholder.substr(split_position + 1));
                        }
                        
                        Result<Placeholder::Identifier> identifier = parse_placeholder_identifier(components[0]);
                        if (!identifier.ok()) {
                            return Result<FormatString>::NOT_OK("error parsing format string - {}", identifier.what());
                        }
                        
                        if (components.size() > 1) {
                            parse_placeholder_format_specifiers(components[1]);
                        }
                    }
                }
                else {
                    if (format_string[i] == '{') {
                        if (i != format_string.length() - 1u && format_string[i + 1u] == '{') {
                            // Escaped '{' character.
                            string.append(format_string.substr(last_update_position, i - last_update_position));
                            last_update_position = i + 1u;
                        }
                        else {
                            processing_placeholder = true;
                            placeholder_start = i;
                        }
                    }
                    else if (format_string[i] == '}') {
                        if (i == 0u) {
                            // return Result<FormatString>::NOT_OK("");
                        }
                        else if (format_string[i - 1u] == '}') {
                            // Escaped '}' character.
                            string.append(format_string.substr(last_update_position, i - last_update_position));
                            last_update_position = i + 1u;
                        }
                    }
                }
            }
            
            return Result<FormatString> { };
        }
        
        Placeholder::Identifier::Identifier() : type(None),
                                                position(-1)
                                                {
        }
        
        Placeholder::Identifier::Identifier(int position) : type(Position),
                                                            position(position)
                                                            {
        }
        
        Placeholder::Identifier::Identifier(std::string name) : type(Name),
                                                                position(-1),
                                                                name(std::move(name))
                                                                {
        }
        
        Placeholder::Formatting::Formatting() : justification(Right),
                                                representation(Decimal),
                                                sign(NegativeOnly),
                                                fill(' '),
                                                separator('\0'), // empty
                                                width(std::numeric_limits<unsigned>::max()),
                                                precision(8)
                                                {
        }
        
        Placeholder::Formatting::~Formatting() {
        }
        
        Result<Placeholder::Identifier> parse_placeholder_identifier(const std::string& identifier) {
            if (identifier.empty()) {
                // Detected auto-numbered placeholder - {}.
                return Result<Placeholder::Identifier>();
            }
            else {
                // A placeholder identifier should not have any whitespace characters.
                for (unsigned i = 0u; i < identifier.length(); ++i) {
                    if (std::isspace(identifier[i])) {
                        return Result<Placeholder::Identifier>::NOT_OK("whitespace character at position {}", i);
                    }
                }
                
                // Determine if placeholder is positional or named.
                if (std::regex_match(identifier, std::regex("^[0-9]+$"))) {
                    // Positional placeholders can only be positive integers.
                    char* end;
                    unsigned position = std::strtoul(identifier.c_str(), &end, 10);
                    return Result<Placeholder::Identifier>(position);
                }
                else if (std::regex_match(identifier, std::regex("^[a-zA-Z_]\\w*$"))) {
                    // Named placeholders follow the same naming convention as C++ identifiers:
                    //  - start with a letter or underscore
                    //  - followed by any combination of letters, digits, or underscores (\w)
                    return Result<Placeholder::Identifier>(identifier);
                }
                else {
                    return Result<Placeholder::Identifier>::NOT_OK("placeholder identifier '{}' is not valid", identifier);
                }
            }
        }
        
        Result<Placeholder::Formatting> parse_placeholder_format_specifiers(const std::string& format_specifiers) {
        
        }
        

        
    }
    
    
    



    
}