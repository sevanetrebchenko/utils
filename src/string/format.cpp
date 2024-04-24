
#include "utils/string/format.hpp"
#include "utils/exceptions.hpp"
#include "utils/assert.hpp"
#include "utils/string/formatting.hpp"

#include <charconv> // std::from_chars

namespace utils {

    // FormatString implementation
    
    FormatString::FormatString(std::string_view fmt, std::source_location source) : m_format(fmt),
                                                                                    m_source(source) {
        parse();
    }
    
    FormatString::FormatString(const char* fmt, std::source_location source) : m_format(fmt),
                                                                               m_source(source) {
        parse();
    }
    
    FormatString::FormatString(std::string fmt, std::source_location source) : m_format(std::move(fmt)),
                                                                               m_source(source) {
        parse();
    }
    
    FormatString::~FormatString() = default;
    
    std::size_t FormatString::get_placeholder_count() const {
        return m_formatted_placeholders.size();
    }
    
    std::size_t FormatString::get_positional_placeholder_count() const {
        std::size_t highest_position = 0u;
        
        // The number of positional placeholders depends on the highest placeholder value encountered in the format string.
        for (const Identifier& identifier : m_placeholder_identifiers) {
            if (identifier.type == Identifier::Type::Position) {
                // Positional placeholders indices start with 0.
                highest_position = std::max(identifier.position + 1, highest_position);
            }
        }
        
        return highest_position;
    }
    
    std::size_t FormatString::get_named_placeholder_count() const {
        std::size_t count = 0u;
        
        for (const Identifier& identifier : m_placeholder_identifiers) {
            if (identifier.type == Identifier::Type::Name) {
                ++count;
            }
        }
        
        return count;
    }

    void FormatString::parse() {
        std::string_view fmt = std::string_view(m_format);
        
        if (fmt.empty()) {
            return;
        }
        
        std::size_t length = fmt.length();
        std::size_t placeholder_start;
        std::size_t placeholder_offset = 0u;

        for (std::size_t i = 0u; i < length; ++i) {
            if (fmt[i] == '{') {
                if (i + 1u == length) {
                    // throw FormattedError("unterminated '{' at index {}", i);
                }
                else if (fmt[i + 1u] == '{') {
                    // Escaped opening brace '{'
                    ++i;
                }
                else {
                    placeholder_start = i;
                    
                    // Skip opening brace '{'
                    ++i;

                    std::size_t identifier_start = i;
                    
                    // Parse placeholder identifier
                    Identifier identifier { };
                    if (std::isdigit(fmt[i])) {
                        ++i;
                        
                        // Positional placeholders must only contain numbers
                        while (std::isdigit(fmt[i])) {
                            ++i;
                        }
                        
                        if (fmt[i] != '|' && fmt[i] != '}') {
                            // throw FormattedError("invalid character '{}' at index {} - expecting formatting separator '|' or placeholder terminator '}'", fmt[i], i);
                        }
                        
                        std::size_t position;
                        from_string(fmt.substr(identifier_start, i - identifier_start), position);
                        
                        identifier = Identifier(position);
                    }
                    else if (std::isalpha(fmt[i]) || (fmt[i] == '_')) {
                        ++i;
                        
                        // Named placeholders follow the same identifier rules as standard C/C++ identifiers
                        while (std::isalpha(fmt[i]) || std::isdigit(fmt[i]) || (fmt[i] == '_')) {
                            ++i;
                        }
                        
                        if (fmt[i] != ':' && fmt[i] != '}') {
                            throw FormattedError("invalid character '{}' at index {} - expecting formatting separator ':' or placeholder terminator '}'", fmt[i], i);
                        }
                        
                        identifier = Identifier(std::string(fmt.substr(identifier_start, i - identifier_start)));
                    }
                    else {
                        // Identifiers for auto-numbered placeholders are default-initialized
                        if (fmt[i] != ':' && fmt[i] != '}') {
                            throw FormattedError("invalid character '{}' at index {} - expecting formatting separator ':' or placeholder terminator '}'", fmt[i], i);
                        }
                    }
                    
                    // Parse custom formatting
                    // TODO: support for nested formatting
                    Formatting formatting { };
                    Formatting* nested = nullptr;
                    
                    while (fmt[i] != '}') {
                        // Skip formatting separator ':' or comma separator ','
                        ++i;
                        
                        std::size_t specifier_start = i;
                        
                        // Identifiers can contain letters, digits, or underscores, and must begin with a letter or an underscore
                        while (std::isalpha(fmt[i]) || (fmt[i] == '_') || (i != specifier_start && std::isdigit(fmt[i]))) {
                            ++i;
                        }
                        if (fmt[i] != '=') {
                            throw FormattedError("invalid character '{}' at index {} - formatting specifier separator must be '='", fmt[i], i);
                        }

                        std::string_view specifier = fmt.substr(specifier_start, i - specifier_start);
                        
                        // Skip separator '='
                        ++i;
                        
                        // Format specifier values must be contained within square braces
                        if (fmt[i] != '[') {
                            throw FormattedError("invalid character '{}' at index {} - formatting specifier value must be contained within square braces: [ ... ]", fmt[i], i);
                        }
                        
                        std::size_t specifier_value_start = i;
                        
                        // Skip opening value brace '['
                        ++i;
                        
                        std::string value;
                        while (true) {
                            if (fmt[i] == '[') {
                                if (i + 1u == length) {
                                    break;
                                }
                                
                                if (fmt[i + 1u] == '[') {
                                    // Escaped value brace '['
                                    ++i;
                                }
                                else {
                                    throw FormattedError("unescaped '[' at index {} - opening formatting brace literals must be escaped to '[[' inside specifier values", i);
                                }
                            }
                            else if (fmt[i] == ']') {
                                if (i + 1u != length && fmt[i + 1u] == ']') {
                                    // Escaped value brace ']'
                                    ++i;
                                }
                                else {
                                    break;
                                }
                            }

                            value += fmt[i];
                            ++i;
                        }
                        
                        if (fmt[i] != ']') {
                            // This is only true when we reach the end of the string before finding a closing format specifier brace in the while loop above
                            throw FormattedError("unterminated formatting specifier value at index {}", specifier_value_start);
                        }
                        
                        if (specifier == "justification" || specifier == "justify" || specifier == "alignment" || specifier == "align") {
                            if (value == "left") {
                                formatting.justification = Formatting::Justification::Left;
                            }
                            else if (value == "right") {
                                formatting.justification = Formatting::Justification::Right;
                            }
                            else if (value == "center") {
                                formatting.justification = Formatting::Justification::Center;
                            }
                            else {
                                // Unknown
                            }
                        }
                        else if (specifier == "sign") {
                            if (value == "negative" || value == "negativeonly") {
                                formatting.sign = Formatting::Sign::NegativeOnly;
                            }
                            else if (value == "align" || value == "aligned") {
                                formatting.sign = Formatting::Sign::Aligned;
                            }
                            else if (value == "both") {
                                formatting.sign = Formatting::Sign::Both;
                            }
                        }
                        else if (specifier == "representation") {
                        }
                        else if (specifier == "precision") {
                        }
                        else if (specifier == "width") {
                        }
                        else if (specifier == "fill") {
                        }
                        else if (specifier == "separator") {
                        }
                        else if (specifier == "use_base_prefix") {
                        }
                        else if (specifier == "spacing" || specifier == "group_size") {
                        }
                        else {
                            // Unknown
                        }
                        
                        // Skip closing value brace ']'
                        ++i;

                        if (fmt[i] != ',' && fmt[i] != '}') {
                            throw FormattedError("invalid character '{}' at index {} - expecting format specifier separator ',' or placeholder terminator '}'", fmt[i], i);
                        }
                        else if (fmt[i] == ':') {
                        
                        }
                    }
                    
                    register_placeholder(identifier, formatting, placeholder_start - placeholder_offset);
                    placeholder_offset += (i - placeholder_start) + 1;
                }
            }
            else if (fmt[i] == '}') {
                if ((i + 1u != length) && fmt[i + 1u] == '}') {
                    // Skip escaped '}' brace
                    ++i;
                }
                else {
                    // throw FormattedError("invalid '}' at index {} - closing brace literals must be escaped to '}}' inside format strings", i);
                }
            }
            else {
                m_result += fmt[i];
            }
        }
    }
    
    void FormatString::register_placeholder(const Identifier& identifier, const Formatting& formatting, std::size_t position) {
        // Verify format string homogeneity.
        bool homogeneous = true;
        for (const Identifier& current : m_placeholder_identifiers) {
            if (identifier.type == Identifier::Type::Auto && current.type != Identifier::Type::Auto || identifier.type != Identifier::Type::Auto && current.type == Identifier::Type::Auto) {
                homogeneous = false;
                break;
            }
        }

        if (!homogeneous) {
            std::string format_string_type = m_placeholder_identifiers[0].type == Identifier::Type::Auto ? "unstructured" : "structured";
            
            std::string placeholder_type;
            if (identifier.type == Identifier::Type::Auto) {
                placeholder_type = "auto-numbered";
            }
            else if (identifier.type == Identifier::Type::Position) {
                placeholder_type = "positional";
            }
            else {
                placeholder_type = "named";
            }
            
            // throw FormattedError("format string placeholders must be homogeneous - {} format string has {} placeholder at index {}", format_string_type, placeholder_type, position);
        }
        
        std::size_t placeholder_index = m_placeholder_identifiers.size(); // invalid index

        if (identifier.type != Identifier::Type::Auto) {
            // Auto-numbered placeholders should not be de-duped and always count as a new placeholder
            for (std::size_t i = 0u; i < m_placeholder_identifiers.size(); ++i) {
                if (identifier == m_placeholder_identifiers[i]) {
                    placeholder_index = i;
                    break;
                }
            }
        }
        
        if (placeholder_index == m_placeholder_identifiers.size()) {
            m_placeholder_identifiers.emplace_back(identifier);
        }
        
        bool found = false;
        for (FormattedPlaceholder& placeholder : m_formatted_placeholders) {
            if (placeholder.identifier_index == placeholder_index && placeholder.formatting == formatting) {
                found = true;
                placeholder.add_insertion_point(position);
            }
        }
        
        if (!found) {
            FormattedPlaceholder& placeholder = m_formatted_placeholders.emplace_back(placeholder_index, formatting);
            placeholder.add_insertion_point(position);
        }
    }
    
    std::string_view FormatString::format_string() const {
        return m_format;
    }
    
    std::source_location FormatString::source() const {
        return m_source;
    }
    
    // FormatString::Identifier implementation
    
    FormatString::Identifier::Identifier() : type(Type::Auto),
                                             position(-1),
                                             name() {
    }

    FormatString::Identifier::Identifier(std::size_t position) : type(Type::Position),
                                                                 position(position),
                                                                 name() {
    }
    
    FormatString::Identifier::Identifier(std::string name) : type(Type::Name),
                                                             position(std::numeric_limits<std::size_t>::max()),
                                                             name(std::move(name)) {
    }
    
    FormatString::Identifier::~Identifier() = default;
    
    bool FormatString::Identifier::operator==(const Identifier& other) const {
        bool matching_types = type == other.type;
        
        if (type == Type::Auto) {
            return matching_types;
        }
        else if (type == Type::Position) {
            return matching_types && position == other.position;
        }
        else {
            return matching_types && name == other.name;
        }
    }

    // FormatString::FormattedPlaceholder implementation
    
    FormatString::FormattedPlaceholder::FormattedPlaceholder(std::size_t identifier_index, const Formatting& formatting) : identifier_index(identifier_index),
                                                                                                                           formatting(formatting) {
    }
    
    FormatString::FormattedPlaceholder::~FormattedPlaceholder() = default;
    
    void FormatString::FormattedPlaceholder::add_insertion_point(std::size_t position) {
        insertion_points.emplace_back(position);
    }
    
}