
#ifndef UTILS_STRING_TPP
#define UTILS_STRING_TPP

#include "utils/exceptions.hpp"
#include "utils/result.hpp"

namespace utils {

    namespace detail {

        struct Identifier {
            Identifier();
            Identifier(std::size_t position);
            Identifier(std::string_view name);
            
            enum class Type {
                Auto = 0,
                Position,
                Name,
            } type;
            
            // This layout has the same size as std::variant<std::size_t, std::string>
            // Prefer this way so that accessing the underlying identifier is easier
            std::size_t position;
            std::string_view name;
        };
        
        struct Specifier {
            std::string_view name;
            std::string_view value;
        };
        
        struct Placeholder {
            std::size_t identifier_index;
            std::size_t specification_index;
            std::size_t position;
        };

        // Returns the number of characters read
        std::size_t parse_identifier(std::string_view in, Identifier& out);
        
        // Returns the index of the first invalid character
        std::size_t parse_format_spec(std::string_view in, FormatSpec& out, bool nested = false);
        
    }

    template <typename ...Ts>
    std::string format(Message message, const Ts&... args) {
        using namespace detail;
        
        // Parse format string
        if (message.format.empty()) {
            return "";
        }
        
        std::vector<Identifier> identifiers;
        std::vector<FormatSpec> specifications;
        std::vector<Placeholder> placeholders;
        
        std::string_view fmt = message.format;
        std::size_t length = fmt.length();
        
        std::size_t i = 0u;
        
        while (i < length) {
            if (fmt[i] == '{') {
                if (i + 1 == length) {
                    throw;
                }
                else if (fmt[i + 1] == '{') {
                    // Escaped brace
                }
                else {
                    // Placeholder opening brace '{'
                    ++i;
                    
                    Identifier identifier { };
                    i += parse_identifier(fmt.substr(i), identifier);
                    if (fmt[i] != ':' && fmt[i] != '}') {
                        throw;
                    }
                    
                    FormatSpec spec { };
                    if (fmt[i] == ':') {
                        // Format spec separator ':'
                        ++i;

                        i += parse_format_spec(fmt.substr(i), spec);
                        if (fmt[i] != '}') {
                            throw;
                        }
                    }
                    
                    // Skip placeholder closing brace '}'
                    ++i;
                    
                    // Verify placeholder homogeneity
                    if (!placeholders.empty()) {
                        // The identifier of the first placeholder dictates the type of format string
                        Identifier::Type type = identifiers[placeholders[0].identifier_index].type;
                        
                        bool homogeneous = (type == Identifier::Type::Auto && identifier.type == Identifier::Type::Auto) && (type != Identifier::Type::Auto && identifier.type != Identifier::Type::Auto);
                        if (!homogeneous) {
                            throw FormattedError("placeholder at index {} is invalid (format string placeholders must be homogeneous)", identifier.position);
                        }
                    }
                    
                    // Register placeholder
                    std::size_t num_identifiers = identifiers.size();
                    std::size_t identifier_index = num_identifiers;
                    
                    for () {
                    
                    }
                }
            }
        }
        
    }
    
}

#endif // UTILS_STRING_TPP