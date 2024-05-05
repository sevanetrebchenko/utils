
#include "utils/format.hpp"
#include "utils/result.hpp"
#include "utils/string.hpp"

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
        return m_placeholders.size();
    }

    std::size_t FormatString::get_positional_placeholder_count() const {
        std::size_t highest_position = 0u;

        // The number of positional placeholders depends on the highest placeholder value encountered in the format string.
        for (const Identifier& identifier : m_identifiers) {
            if (identifier.type == Identifier::Type::Position) {
                // Positional placeholders indices start with 0.
                highest_position = std::max(identifier.position + 1, highest_position);
            }
        }

        return highest_position;
    }

    std::size_t FormatString::get_named_placeholder_count() const {
        std::size_t count = 0u;

        for (const Identifier& identifier : m_identifiers) {
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
                    throw FormattedError("unterminated '{' at index {}", i);
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
                    // Note: identifiers for auto-numbered placeholders are default-initialized and are not handled below
                    Identifier identifier { };
                    if (std::isdigit(fmt[i])) {
                        ++i;
                        
                        // Positional placeholders must only contain numbers
                        while (std::isdigit(fmt[i])) {
                            ++i;
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
                        
                        identifier = Identifier(std::string(fmt.substr(identifier_start, i - identifier_start)));
                    }
                    
                    if (fmt[i] != ':' && fmt[i] != '}') {
                        throw FormattedError("invalid character '{}' at index {} - expecting formatting separator ':' or placeholder terminator '}'", fmt[i], i);
                    }
                    
                    // Parse custom formatting
                    ParseResult<Specification, std::string_view> parse_result = parse_specification(fmt);
                    if (!parse_result.ok()) {
                        // Throw exception with the error position relative to the start of the string
                        throw FormattedError(parse_result.error(), NamedArgument("index", parse_result.offset() + i));
                    }
                    
                    register_placeholder(identifier, parse_result.result(), placeholder_start - placeholder_offset);
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
    
    void FormatString::register_placeholder(const Identifier& identifier, const Specification& spec, std::size_t position) {
        // Verify format string homogeneity.
        bool homogeneous = true;
        for (const Identifier& current : m_identifiers) {
            if (identifier.type == Identifier::Type::Auto && current.type != Identifier::Type::Auto || identifier.type != Identifier::Type::Auto && current.type == Identifier::Type::Auto) {
                homogeneous = false;
                break;
            }
        }

        if (!homogeneous) {
            std::string format_string_type = m_identifiers[0].type == Identifier::Type::Auto ? "unstructured" : "structured";
            
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
        
        std::size_t placeholder_index = m_identifiers.size(); // invalid index

        if (identifier.type != Identifier::Type::Auto) {
            // Auto-numbered placeholders should not be de-duped and always count as a new placeholder
            for (std::size_t i = 0u; i < m_identifiers.size(); ++i) {
                if (identifier == m_identifiers[i]) {
                    placeholder_index = i;
                    break;
                }
            }
        }
        
        if (placeholder_index == m_identifiers.size()) {
            m_identifiers.emplace_back(identifier);
        }
        
        bool found = false;
        for (Placeholder& placeholder : m_placeholders) {
            if (placeholder.identifier_index == placeholder_index && placeholder.spec == spec) {
                found = true;
                placeholder.add_insertion_point(position);
            }
        }
        
        if (!found) {
            Placeholder& placeholder = m_placeholders.emplace_back(placeholder_index, spec);
            placeholder.add_insertion_point(position);
        }
    }
    
    std::string_view FormatString::format_string() const {
        return m_format;
    }
    
    std::source_location FormatString::source() const {
        return m_source;
    }
    
    
    
    ParseResult<FormatString::Specification, std::string_view> FormatString::parse_specification(std::string_view in, bool nested) {
        std::size_t length = in.length();
        std::size_t i = 0u;
        
        Specification spec { };
        std::size_t group = 0u;
        
        char terminator = nested ? '|' : '}';
        while (in[i] != terminator) {
            if (in[i] == ':') {
                // Assuming the input string does not contain the separator between identifier and specification string
                
                // Specification separator
                // {identifier:representation=[...]}
                //            ^
                
                spec.add_group(spec);
                spec = Specification { }; // reset
                ++group;
                
                // Skip over separator ':'
                ++i;
            }
            else if (in[i] == '|') {
                // Nested formatting
                // {identifier:|representation=[...]:justification=[...]|}
                //             ^

                if (!spec.empty()) {
                    // Cannot mix nested spec with formatting specifiers
                    return ParseResult<Specification, std::string_view>::NOT_OK(i, "invalid nested format specification at {index}");
                }
                
                ParseResult<Specification, std::string_view> result = parse_specification(in.substr(i), true);
                if (!result.ok()) {
                    return ParseResult<Specification, std::string_view>::NOT_OK(result.offset() + i, result.error());
                }
                
                spec.add_group(result.result());
            }
            else {
                std::size_t specifier_start = i;
                
                // Identifiers can contain letters, digits, or underscores, and must begin with a letter or an underscore
                while (std::isalpha(in[i]) || (in[i] == '_') || (i && std::isdigit(in[i]))) {
                    ++i;
                }
                
                if (in[i] != '=') {
                    return ParseResult<Specification, std::string_view>::NOT_OK(i, "invalid character '{0}' at index {index} - formatting specifier separator must be '='", in[i]);
                }
                
                // Skip separator '='
                ++i;
                
                if (in[i] != '[') {
                    return ParseResult<Specification, std::string_view>::NOT_OK(i, "invalid character '{0}' at index {index} - formatting specifier value must be contained within square braces: [ ... ]", in[i]);
                }
                
                // Skip specifier value opening brace '['
                ++i;
                
                std::size_t specifier_value_start = i;
                while (true) {
                    if (in[i] == '[') {
                        if (i + 1u == in.length()) {
                            break;
                        }
                        
                        if (in[i + 1u] == '[') {
                            // Escaped value brace '['
                            ++i;
                        }
                        else {
                            return ParseResult<Specification, std::string_view>::NOT_OK(i, "unescaped '[' at index {index} - opening formatting brace literals must be escaped as '[[' inside specifier values");
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
        
                    ++i;
                }
                if (in[i] != ']') {
                    // This is only true when we reach the end of the string before finding a closing format specifier brace in the while loop above
                    return ParseResult<Specification, std::string_view>::NOT_OK(specifier_value_start, "unterminated formatting specifier value at index {index}");
                }
                
                // Skip closing value brace ']'
                ++i;
        
                if (in[i] != ',' && in[i] != '}') {
                    return ParseResult<Specification, std::string_view>::NOT_OK(i, "invalid character '{0}' at index {index} - expecting format specifier separator ',', formatting group terminator '|' or ':', or placeholder terminator '}'", in[i]);
                }
                
                spec.add_specifier(group, in.substr(specifier_start, i - specifier_start), in.substr(specifier_value_start, i - specifier_value_start));
            }
        }
        
        return ParseResult<Specification, std::string_view>::OK(i, spec);
    }
    
    // FormatString::Identifier implementation
    
    FormatString::Identifier::Identifier() : type(Type::Auto),
                                             position(std::numeric_limits<std::size_t>::max()),
                                             name() {
    }

    FormatString::Identifier::Identifier(std::size_t position) : type(Type::Position),
                                                                 position(position),
                                                                 name() {
    }
    
    FormatString::Identifier::Identifier(std::string_view name) : type(Type::Name),
                                                                  position(std::numeric_limits<std::size_t>::max()),
                                                                  name(name) {
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

    // FormatString::Specification implementation
    
    FormatString::Specification::Specification() = default;

    FormatString::Specification::~Specification()= default;

    bool FormatString::Specification::operator==(const FormatString::Specification& other) const {
        return false;
    }
    
    void FormatString::Specification::add_group(const Specification& spec) {
    }
    
    const FormatString::Specification& FormatString::Specification::get_group(std::size_t index) const {
        throw;
    }
    
    bool FormatString::Specification::has_group(std::size_t index) const {
        return false;
    }
    
    void FormatString::Specification::add_specifier(std::size_t group, std::string_view key, std::string_view value) {
    }

    std::string_view FormatString::Specification::get_specifier(std::size_t group, std::string_view key) const {
        return "";
    }

    bool FormatString::Specification::has_specifier(std::size_t group, std::string_view key) const {
        const SpecifierList& specifier_list = get_specifier_mapping(group);
        return specifier_list.find(key) != specifier_list.end();
    }
    
    FormatString::Specification::SpecifierList& FormatString::Specification::get_specifier_mapping(std::size_t group) {
        if (group >= m_groups.size()) {
            // out of bounds
            throw;
        }
        
        if (!std::holds_alternative<SpecifierList>(m_groups[group])) {
            // nested specification may contain more than one group
            throw;
        }
        
        return std::get<SpecifierList>(m_groups[group]);
    }
    
    const FormatString::Specification::SpecifierList& FormatString::Specification::get_specifier_mapping(std::size_t group) const {
        if (group >= m_groups.size()) {
            // out of bounds
            throw;
        }
        
        if (!std::holds_alternative<SpecifierList>(m_groups[group])) {
            // nested specification may contain more than one group
            throw;
        }
        
        return std::get<SpecifierList>(m_groups[group]);
    }
    
    bool FormatString::Specification::empty() const {
        return m_groups.empty();
    }
    
    // FormatString::Placeholder implementation
    
    FormatString::Placeholder::Placeholder(std::size_t identifier_index, const Specification& spec) : identifier_index(identifier_index),
                                                                                                      spec(spec) {
    }
    
    FormatString::Placeholder::~Placeholder() = default;
    
    void FormatString::Placeholder::add_insertion_point(std::size_t position) {
        insertion_points.emplace_back(position);
    }

}