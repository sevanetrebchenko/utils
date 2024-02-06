
#include "utils/string.hpp"
#include "utils/result.hpp"
#include "utils/internal/string.tpp"

#include <limits>
#include <charconv>

namespace utils {
    namespace internal {
        
        // Identifier implementation.
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
        
        bool Placeholder::Identifier::operator==(const Placeholder::Identifier& other) const {
            return type == other.type && position == other.position && name == other.name;
        }
        
        
        // Formatting implementation.
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
        
        bool Placeholder::Formatting::operator==(const Placeholder::Formatting& other) const {
            return justification == other.justification &&
                   representation == other.representation &&
                   sign == other.sign &&
                   fill == other.fill &&
                   separator == other.separator &&
                   width == other.width &&
                   precision == other.precision;
        }
        
        
        // Placeholder implementation
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
        
        bool Placeholder::operator==(const Placeholder& other) const {
            return identifier == other.identifier && formatting == other.formatting;
        }
        
        
        // FormatString implementation
        FormatString::FormatString(std::string_view in) {
            Result<FormatString> result = parse(in);
            if (!result.ok()) {
                throw std::runtime_error(format("error encountered while parsing format string - {}", result.what()));
            }
            *this = *result;
        }
        
        Result<FormatString> FormatString::parse(std::string_view in) {
            Result<FormatString> format_string { };
            int processed_position = -1;
            
            // To save on processing power, the resulting string is only updated when a brace character is encountered.
            bool processing_placeholder = false;
            std::size_t placeholder_start = in.length();
            std::size_t last_update_position = 0u;
            
            for (std::size_t i = 0u; i < in.length(); ++i, ++processed_position) {
                if (processing_placeholder) {
                    if (in[i] == '}') {
                        // Substring to get only the placeholder + opening / closing braces.
                        Result<Placeholder> placeholder = Placeholder::parse(in.substr(placeholder_start, i - placeholder_start + 1u));
                        if (!placeholder.ok()) {
                            return Result<FormatString>::NOT_OK("error parsing format string - {}", placeholder.what());
                        }
                        
                        // Register this placeholder and the position it should be inserted at (accounting for other placeholders / escaped characters).
                        format_string->register_placeholder(*placeholder, processed_position);
                        processing_placeholder = false;
                        placeholder_start = in.length(); // invalid index
                        
                        // Placeholders are entirely omitted in the resulting string.
                        last_update_position = i + 1u;
                    }
                }
                else {
                    bool is_last_character = i == (in.length() - 1u);
                    bool is_escape_sequence = (in[i] == '{' && !is_last_character && in[i + 1u] == '{') ||
                                              (in[i] == '}' && !is_last_character && in[i + 1u] == '}');
                    
                    if (is_escape_sequence) {
                        // Escaped brace character.
                        // Resulting string will only have one brace (instead of the two in the format string), which affects the position for inserting processed placeholders.
                        processed_position -= 1;
                        
                        // Update the resulting string with the contents of the format string starting from the last update position up until the brace character (inclusive).
                        format_string->m_format.append(in.substr(last_update_position, (i + 1u) - last_update_position));
                        
                        // Skip over the second brace character.
                        i += 1u;
                        last_update_position = i + 1u;
                    }
                    else if (in[i] == '{') {
                        // Start of placeholder.
                        processing_placeholder = true;
                        placeholder_start = i;
                        
                        // Update the resulting string with the contents of the format string starting from the last update position up until the brace character (exclusive).
                        format_string->m_format.append(in.substr(last_update_position, i - last_update_position));
                    }
                    else if (in[i] == '}') {
                        // This code path would never be hit if the '}' character belonged to a previously-opened placeholder scope, as this path is processed above.
                        // Therefore, this is an unescaped '}' character that does NOT belong to a scope, which is not a valid format string.
                        return Result<FormatString>::NOT_OK("");
                    }
                }
            }
            
            if (processing_placeholder) {
                return Result<FormatString>::NOT_OK("");
            }
            
            if (!format_string->verify_placeholder_homogeneity()) {
                return Result<FormatString>::NOT_OK("format string placeholders must be homogeneous - auto-numbered placeholders cannot be mixed in with positional/named ones");
            }
            
            // Append any remaining characters present between the last brace and the end of the format string.
            format_string->m_format.append(in.substr(last_update_position));
            
            return format_string;
        }
        
        FormatString::FormatString() = default;
        
        FormatString::~FormatString() = default;
        
        void FormatString::register_placeholder(const Placeholder& placeholder, std::size_t position) {
            // Determine if this placeholder already exists
            std::size_t placeholder_index = m_placeholders.size();
            
            for (std::size_t i = 0u; i < m_placeholders.size(); ++i) {
                if (placeholder == m_placeholders[i]) {
                    placeholder_index = i;
                    break;
                }
            }
            
            if (placeholder_index == m_placeholders.size()) {
                m_placeholders.emplace_back(placeholder);
            }
            
            // Register insertion point.
            m_insertion_points.emplace_back(placeholder_index, position);
        }
        
        bool FormatString::verify_placeholder_homogeneity() const {
            if (m_placeholders.empty()) {
                return true;
            }
            
            bool is_auto_numbered = m_placeholders[0].identifier.type == Placeholder::Identifier::Type::None;
            for (std::size_t i = 1u; i < m_placeholders.size(); ++i) {
                if (is_auto_numbered && m_placeholders[i].identifier.type != Placeholder::Identifier::Type::None) {
                    // Detected placeholder of a different type.
                    return false;
                }
            }
            
            return true;
        }
        
        std::size_t FormatString::get_placeholder_count() const {
            return m_placeholders.size();
        }
        
        std::size_t FormatString::get_positional_placeholder_count() const {
            std::size_t count = 0u;
            
            for (const Placeholder& placeholder : m_placeholders) {
                if (placeholder.identifier.type == Placeholder::Identifier::Type::Position) {
                    ++count;
                }
            }
            
            return count;
        }
        
        std::size_t FormatString::get_named_placeholder_count() const {
            std::size_t count = 0u;
            
            for (const Placeholder& placeholder : m_placeholders) {
                if (placeholder.identifier.type == Placeholder::Identifier::Type::Name) {
                    ++count;
                }
            }
            
            return count;
        }
        
        FormatString::InsertionPoint::InsertionPoint(std::size_t placeholder_index, std::size_t insert_position) : placeholder_index(placeholder_index),
                                                                                                                   insert_position(insert_position)
                                                                                                                   {
        }
        
        FormatString::InsertionPoint::~InsertionPoint() = default;
        
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