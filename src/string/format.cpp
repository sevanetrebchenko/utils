
#include "utils/string/format.hpp"

namespace utils {

    Formatting::Specifier::Specifier(std::string value) : m_raw(std::move(value)) {
    }
    
    Formatting::Specifier::~Specifier() = default;
    
    bool Formatting::Specifier::operator==(const Formatting::Specifier& other) const {
        return m_raw == other.m_raw;
    }
    
    Formatting::Specifier& Formatting::Specifier::operator=(const std::string& value) {
        m_raw = value;
        return *this;
    }
    
    // Formatting implementation

    Formatting::Formatting() = default;
    
    Formatting::~Formatting() = default;
    
    Formatting::Specifier& Formatting::operator[](const std::string& key) {
        return m_specifiers[key];
    }
    
    Formatting::Specifier Formatting::operator[](const std::string& key) const {
        auto iter = m_specifiers.find(key);
        if (iter != m_specifiers.end()) {
            return iter->second;
        }
        return { };
    }
    
    bool Formatting::operator==(const Formatting& other) const {
        return m_specifiers == other.m_specifiers;
    }
    
    // FormatString implementation
    
    FormatString::FormatString(std::string_view in) {
        if (in.empty()) {
            return;
        }
        
        std::size_t length = in.length();
        std::size_t placeholder_start;
        std::size_t placeholder_offset = 0u;

        for (std::size_t i = 0u; i < length; ++i) {
            if (in[i] == '{') {
                if (i + 1u == length) {
                    throw FormattedError("unterminated '{' at index {}", i);
                }
                else if (in[i + 1u] == '{') {
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
                    if (std::isdigit(in[i])) {
                        ++i;
                        
                        // Positional placeholders must only contain numbers
                        while (std::isdigit(in[i])) {
                            ++i;
                        }
                        
                        if (in[i] != '|' && in[i] != '}') {
                            throw FormattedError("invalid character '{}' at index {} - expecting formatting separator '|' or placeholder terminator '}'", in[i], i);
                        }
                        
                        std::size_t position;
                        std::string_view str = in.substr(identifier_start, i - identifier_start);
                        std::from_chars_result result = std::from_chars(str.data(), str.data() + str.length(), position);
                        ASSERT(result.ec == std::errc(), "error while converting positional placeholder '{}': {}", str, make_error_code(result.ec).message());

                        identifier = Identifier(position);
                    }
                    else if (std::isalpha(in[i]) || (in[i] == '_')) {
                        ++i;
                        
                        // Named placeholders follow the same identifier rules as standard C/C++ identifiers
                        while (std::isalpha(in[i]) || std::isdigit(in[i]) || (in[i] == '_')) {
                            ++i;
                        }
                        
                        if (in[i] != '|' && in[i] != '}') {
                            throw FormattedError("invalid character '{}' at index {} - expecting formatting separator '|' or placeholder terminator '}'", in[i], i);
                        }
                        
                        identifier = Identifier(std::string(in.substr(identifier_start, i - identifier_start)));
                    }
                    else {
                        // Identifiers for auto-numbered placeholders are default-initialized
                        if (in[i] != '|' && in[i] != '}') {
                            throw FormattedError("invalid character '{}' at index {} - expecting formatting separator '|' or placeholder terminator '}'", in[i], i);
                        }
                    }
                    
                    // Parse custom formatting
                    Formatting formatting { };
                    while (in[i] != '}') {
                        // Skip formatting separator '|' or comma separator ','
                        ++i;
                        
                        std::size_t specifier_start = i;
                        
                        // Identifiers can contain letters, digits, or underscores, and must begin with a letter or an underscore
                        while (std::isalpha(in[i]) || (in[i] == '_') || (i != specifier_start && std::isdigit(in[i]))) {
                            ++i;
                        }
                        if (in[i] != ':' && in[i] != '=') {
                            throw FormattedError("invalid character '{}' at index {} - formatting specifier separator must be ':' or '='", in[i], i);
                        }

                        std::string_view specifier = in.substr(specifier_start, i - specifier_start);
                        
                        // Skip separator ':' or '='
                        ++i;
                        
                        // Format specifier values must be contained within square braces
                        if (in[i] != '[') {
                            throw FormattedError("invalid character '{}' at index {} - formatting specifier value must be contained within square braces: [ ... ]", in[i], i);
                        }
                        
                        std::size_t specifier_value_start = i;
                        
                        // Skip opening value brace '['
                        ++i;
                        
                        std::string value;
                        while (true) {
                            if (in[i] == '[') {
                                if (i + 1u == length) {
                                    break;
                                }
                                
                                if (in[i + 1u] == '[') {
                                    // Escaped value brace '['
                                    ++i;
                                }
                                else {
                                    throw FormattedError("unescaped '[' at index {} - opening formatting brace literals must be escaped to '[[' inside specifier values", i);
                                }
                            }
                            else if (in[i] == ']') {
                                if (i + 1u != length && in[i + 1u] == ']') {
                                    // Escaped value brace ']'
                                    ++i;
                                }
                                else {
                                    break;
                                }
                            }

                            value += in[i];
                            ++i;
                        }
                        
                        if (in[i] != ']') {
                            // This is only true when we reach the end of the string before finding a closing format specifier brace in the while loop above
                            throw FormattedError("unterminated formatting specifier value at index {}", specifier_value_start);
                        }
                        
                        formatting[std::string(specifier)] = value;
                        
                        // Skip closing value brace ']'
                        ++i;
                        
                        if (in[i] != ',' && in[i] != '}') {
                            throw FormattedError("invalid character '{}' at index {} - expecting format specifier separator ',' or placeholder terminator '}'", in[i], i);
                        }
                    }
                    
                    register_placeholder(identifier, formatting, placeholder_start - placeholder_offset);
                    placeholder_offset += (i - placeholder_start) + 1;
                }
            }
            else if (in[i] == '}') {
                if ((i + 1u != length) && in[i + 1u] == '}') {
                    // Skip escaped '}' brace
                    ++i;
                }
                else {
                    throw FormattedError("invalid '}' at index {} - closing brace literals must be escaped to '}}' inside format strings", i);
                }
            }
            else {
                m_format += in[i];
            }
        }
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
            
            throw FormattedError("format string placeholders must be homogeneous - {} format string has {} placeholder at index {}", format_string_type, placeholder_type, position);
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
    
    // FormatStringWrapper implementation
    
    FormatStringWrapper::FormatStringWrapper(const char* fmt, std::source_location source) : m_format(fmt),
                                                                                             m_source(source) {
    }

    FormatStringWrapper::FormatStringWrapper(const std::string& fmt, std::source_location source) : m_format(fmt),
                                                                                                    m_source(source) {
    }
    
    FormatStringWrapper::FormatStringWrapper(const FormatString& fmt, std::source_location source) : m_format(fmt),
                                                                                                     m_source(source) {
    }
    
    FormatStringWrapper::~FormatStringWrapper() = default;
    
    FormatString FormatStringWrapper::format_string() const {
        return m_format;
    }
    
    std::source_location FormatStringWrapper::source() const {
        return m_source;
    }
    
}