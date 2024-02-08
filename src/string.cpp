
#include "utils/string.hpp"
#include "utils/result.hpp"
#include "utils/internal/string.tpp"

#include <limits>
#include <charconv>

namespace utils {
    namespace internal {
        
        // Identifier implementation.
        Result<PlaceholderIdentifier> PlaceholderIdentifier::parse(std::string_view identifier) noexcept {
            if (identifier.empty()) {
                // Detected auto-numbered placeholder - {}.
                return Result<PlaceholderIdentifier> { };
            }
            else {
                // A placeholder identifier should not have any whitespace characters.
                for (unsigned i = 0u; i < identifier.length(); ++i) {
                    if (std::isspace(identifier[i])) {
                        return Result<PlaceholderIdentifier>::NOT_OK("whitespace character at index {}", i);
                    }
                }
                
                // Note: regex_match checks the entire input string. This behaviour can be simulated by using
                // regex search and anchoring the input string by using '^' (start) and '$' (end), but has been
                // omitted for simplicity.
                
                // Determine if placeholder is positional or named.
                if (std::regex_match(identifier.begin(), identifier.end(), std::regex("[0-9]+"))) {
                    // Positional placeholders can only be positive integers.
                    int index;
                    
                    const char* start = identifier.data();
                    const char* end = start + identifier.length();
                    std::from_chars_result conversion_result = std::from_chars(start, end, index, 10);
                    
                    // Regex check asserts that std::from_chars(...) will only return std::errc::result_out_of_range if the position value exceeds that of an integer.
                    // Note that this value is ~2.14 billion and should be considered a hard limit on the number of positional arguments for a single format string.
                    if (conversion_result.ec == std::errc::result_out_of_range) {
                        return Result<PlaceholderIdentifier>::NOT_OK("positional placeholder value '{}' is out of range", identifier);
                    }
                    return Result<PlaceholderIdentifier>(PlaceholderIdentifier(index));
                }
                else if (std::regex_match(identifier.begin(), identifier.end(), std::regex("[a-zA-Z_]\\w*"))) {
                    // Named placeholders follow the same naming convention as C++ identifiers:
                    //  - start with a letter or underscore
                    //  - followed by any combination of letters, digits, or underscores (\w)
                    return Result<PlaceholderIdentifier>(PlaceholderIdentifier(std::string(identifier)));
                }
                else {
                    return Result<PlaceholderIdentifier>::NOT_OK("named placeholder identifier '{}' is not valid", identifier);
                }
            }
        }
        
        PlaceholderIdentifier::PlaceholderIdentifier(std::string_view in) {
            Result<PlaceholderIdentifier> result = parse(in);
            if (!result.ok()) {
                throw std::invalid_argument(format("error parsing identifier '{}' - {}", in, result.what()));
            }
            *this = *result;
        }
        
        PlaceholderIdentifier::PlaceholderIdentifier() : type(Type::None),
                                                         position(-1),
                                                         name() {
        }
        
        PlaceholderIdentifier::PlaceholderIdentifier(int position) : type(Type::Position),
                                                                     position(position),
                                                                     name() {
        }
        
        PlaceholderIdentifier::PlaceholderIdentifier(std::string name) : type(Type::Name),
                                                                         position(-1),
                                                                         name(std::move(name)) {
        }
        
        PlaceholderIdentifier::~PlaceholderIdentifier()= default;
        
        bool PlaceholderIdentifier::operator==(const PlaceholderIdentifier& other) const {
            return type == other.type && position == other.position && name == other.name;
        }
        
        
        // Formatting implementation.
        PlaceholderFormatting::PlaceholderFormatting() : justification(Right),
                                                         representation(Decimal),
                                                         sign(NegativeOnly),
                                                         fill(' '),
                                                         separator('\0'), // empty
                                                         width(std::numeric_limits<unsigned>::max()),
                                                         precision(8) {
        }
        
        PlaceholderFormatting::~PlaceholderFormatting() = default;
        
        PlaceholderFormatting::PlaceholderFormatting(std::string_view in) {
            Result<PlaceholderFormatting> result = parse(in);
            if (!result.ok()) {
                throw std::invalid_argument(format("error parsing placeholder format specifiers '{}' - {}", in, result.what()));
            }
            *this = *result;
        }
        
        Result<PlaceholderFormatting> PlaceholderFormatting::parse(std::string_view specifiers) noexcept {
            return Result<PlaceholderFormatting> { };
        }
        
        bool PlaceholderFormatting::operator==(const PlaceholderFormatting& other) const {
            return justification == other.justification &&
                   representation == other.representation &&
                   sign == other.sign &&
                   fill == other.fill &&
                   separator == other.separator &&
                   width == other.width &&
                   precision == other.precision;
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
            
            for (std::size_t i = 0u; i < in.length(); ++i) {
                if (processing_placeholder) {
                    if (in[i] == '}') {
                        // Substring to get the placeholder without opening / closing braces.
                        std::string_view placeholder = in.substr(placeholder_start + 1u, i - placeholder_start - 1u);
                        std::size_t split_position = in.find(':');
                        
                        Result<PlaceholderIdentifier> identifier = PlaceholderIdentifier::parse(placeholder.substr(0, split_position));
                        if (!identifier.ok()) {
                            return Result<FormatString>::NOT_OK("error parsing format string - {}", identifier.what());
                        }

                        // A placeholder is registered to the position it should be inserted at accounting for other placeholders / escaped characters.
                        
                        // Placeholder formatting specifiers are optional.
                        if (split_position != std::string::npos) {
                            Result<PlaceholderFormatting> formatting = PlaceholderFormatting::parse(placeholder.substr(split_position + 1));
                            if (!formatting.ok()) {
                                return Result<FormatString>::NOT_OK("error parsing format string - {}", formatting.what());
                            }
                            
                            format_string->register_placeholder(*identifier, *formatting, processed_position);
                        }
                        else {
                            // Use default formatting.
                            format_string->register_placeholder(*identifier, { }, processed_position);
                        }
                        
                        processing_placeholder = false;
                        placeholder_start = in.length(); // invalid index
                        
                        // Placeholder braces are entirely omitted in the resulting string.
                        last_update_position = i + 1u;
                        
                        // Adjust the insertion position so that placeholders that are directly adjacent to one another do not get inserted at the same position and overlap.
                        ++processed_position;
                    }
                }
                else {
                    bool is_last_character = i == (in.length() - 1u);
                    bool is_escape_sequence = (in[i] == '{' && !is_last_character && in[i + 1u] == '{') ||
                                              (in[i] == '}' && !is_last_character && in[i + 1u] == '}');
                    
                    if (is_escape_sequence) {
                        // Escaped brace character.
                        // Resulting string will only have one brace (instead of the two in the format string), which affects the position for inserting processed placeholders.
                        ++processed_position;
                        
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
                        
                        // Placeholder braces exist only in the format string and will eventually be replaced with the formatted placeholder value.
                        // ++processed_position;
                        
                        // Update the resulting string with the contents of the format string starting from the last update position up until the brace character (exclusive).
                        format_string->m_format.append(in.substr(last_update_position, i - last_update_position));
                    }
                    else if (in[i] == '}') {
                        // This code path would never be hit if the '}' character belonged to a previously-opened placeholder scope, as this path is processed above.
                        // Therefore, this is an unescaped '}' character that does NOT belong to a scope, which is not a valid format string.
                        return Result<FormatString>::NOT_OK("");
                    }
                    else {
                        // Non-special characters are left unmodified.
                        ++processed_position;
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
        
        void FormatString::register_placeholder(const PlaceholderIdentifier& identifier, const PlaceholderFormatting& formatting, std::size_t position) {
            std::size_t placeholder_index = m_placeholder_identifiers.size(); // invalid
            for (std::size_t i = 0u; i < m_placeholder_identifiers.size(); ++i) {
                if (identifier == m_placeholder_identifiers[i]) {
                    placeholder_index = i;
                    break;
                }
            }
            
            if (placeholder_index == m_placeholder_identifiers.size()) {
                m_placeholder_identifiers.emplace_back(identifier);
            }
            
            bool found = false;
            for (FormattedPlaceholder& placeholder : m_formatted_placeholders) {
                if (placeholder.placeholder_index == placeholder_index && placeholder.formatting == formatting) {
                    found = true;
                    placeholder.add_insertion_point(position);
                }
            }
            
            if (!found) {
                FormattedPlaceholder& placeholder = m_formatted_placeholders.emplace_back(placeholder_index, formatting);
                placeholder.add_insertion_point(position);
            }
        }
        
        bool FormatString::verify_placeholder_homogeneity() const {
            if (m_placeholder_identifiers.empty()) {
                return true;
            }
            
            bool is_auto_numbered = m_placeholder_identifiers[0].type == PlaceholderIdentifier::Type::None;
            for (std::size_t i = 1u; i < m_placeholder_identifiers.size(); ++i) {
                if (is_auto_numbered && m_placeholder_identifiers[i].type != PlaceholderIdentifier::Type::None) {
                    // Detected placeholder of a different type.
                    return false;
                }
            }
            
            return true;
        }
        
        std::size_t FormatString::get_total_placeholder_count() const {
            std::size_t count = 0u;
            
            for (const FormattedPlaceholder& placeholder : m_formatted_placeholders) {
                count += placeholder.insertion_points.size();
            }
            
            return count;
        }
        
        std::size_t FormatString::get_unique_placeholder_count() const {
            return m_placeholder_identifiers.size();
        }
        
        std::size_t FormatString::get_positional_placeholder_count() const {
            std::size_t count = 0u;
            
            for (const PlaceholderIdentifier& identifier : m_placeholder_identifiers) {
                if (identifier.type == PlaceholderIdentifier::Type::Position) {
                    ++count;
                }
            }
            
            return count;
        }
        
        std::size_t FormatString::get_named_placeholder_count() const {
            std::size_t count = 0u;
            
            for (const PlaceholderIdentifier& identifier : m_placeholder_identifiers) {
                if (identifier.type == PlaceholderIdentifier::Type::Name) {
                    ++count;
                }
            }
            
            return count;
        }
        
        FormatString::FormattedPlaceholder::FormattedPlaceholder(std::size_t placeholder_index, const PlaceholderFormatting& formatting) : placeholder_index(placeholder_index),
                                                                                                                                           formatting(formatting) {
        }
        
        FormatString::FormattedPlaceholder::~FormattedPlaceholder() = default;
        
        void FormatString::FormattedPlaceholder::add_insertion_point(std::size_t position) {
            insertion_points.emplace_back(position);
        }
        
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
    
}