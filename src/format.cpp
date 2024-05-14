
#include "utils/format.hpp"
#include "utils/result.hpp"
#include "utils/string.hpp"
#include "utils/logging/logging.hpp"

#include <limits> // std::numeric_limits

namespace utils {
    
    ParseResult<FormatString::Specification::Specifier, FormatString> parse_specifier(std::string_view in) {
        std::size_t length = in.length();
        std::size_t i = 0u;
        
        // Identifiers can contain letters, digits, or underscores, and must begin with a letter or an underscore
        while (std::isalpha(in[i]) || (in[i] == '_') || (i && std::isdigit(in[i]))) {
            ++i;
        }
        
        if (i == 0u) {
            return ParseResult<FormatString::Specification::Specifier, FormatString>::NOT_OK(i, "empty format specifiers are not allowed");
        }
        
        if (in[i] != '=') {
            return ParseResult<FormatString::Specification::Specifier, FormatString>::NOT_OK(i, "invalid character '{0}' at index {index} - format specifier separator must be '='", in[i]);
        }
        
        std::string_view specifier = in.substr(0, i);
        
        // Skip separator '='
        ++i;
        
        if (in[i] != '[') {
            return ParseResult<FormatString::Specification::Specifier, FormatString>::NOT_OK(i, "invalid character '{0}' at index {index} - format specifier value must be contained within square braces: [ ... ]", in[i]);
        }
        
        // Skip specifier value opening brace '['
        ++i;

        std::size_t specifier_value_start = i;
        std::string value;
        
        // To avoid appending to the value every time,
        std::size_t last_insert_position = specifier_value_start;
        
        while (i < length) {
            if (in[i] == '[') {
                if (i + 1u == length) {
                    break;
                }
                
                if (in[i + 1u] != '[') {
                    return ParseResult<FormatString::Specification::Specifier, FormatString>::NOT_OK(i, "unescaped '[' at index {index} - opening formatting brace literals must be escaped as '[[' inside specifier values");
                }
                
                // Escaped value brace '[[', append once
                value.append(in.substr(last_insert_position, i - last_insert_position));
                last_insert_position = i;
                i += 2u;
            }
            else if (in[i] == ']') {
                if (i + 1u < length && in[i + 1u] == ']') {
                    // Escaped value brace ']]', append once
                    value.append(in.substr(last_insert_position, i - last_insert_position));
                    last_insert_position = i;
                    i += 2u;
                }
                else {
                    break;
                }
            }
            else {
                ++i;
            }
        }
        
        
        
        value.append(in.substr(last_insert_position, i - last_insert_position));
        
        if (in[i] != ']') {
            return ParseResult<FormatString::Specification::Specifier, FormatString>::NOT_OK(specifier_value_start, "unterminated formatting specifier value at index {index}");
        }
        
        // Skip specifier value closing brace ']'
        ++i;
        
        return ParseResult<FormatString::Specification::Specifier, FormatString>::OK(i, std::string(specifier), value);
    }
    
    ParseResponse<FormatString> parse_specification(std::string_view in, FormatString::Specification& spec, bool nested = false) {
        // Note: function assumes input string does not contain a leading formatting group separator ':'
        std::size_t length = in.length();
        std::size_t group = 0u;
        
        char terminator = nested ? '|' : '}';
        
        std::size_t i = 0u;
        
        while (i < length) {
            if (in[i] == terminator) {
                // Skip formatting specification separator '|'
                ++i;
                break;
            }
            
            if (in[i] == ':') {
                // Encountered formatting group separator
                // {identifier:representation=[...]}
                //            ^
                
                // Note: empty groups are supported and are treated as empty format specifier lists
                ++group;
                
                // Skip formatting group separator ':'
                ++i;
                
                continue;
            }
            else if (in[i] == '|') {
                // Encountered nested formatting specification separator
                // {identifier:|representation=[...]:justification=[...]|}
                //             ^
                
                // Skip formatting specification separator '|'
                ++i;
                
                ParseResponse<FormatString> r = parse_specification(in.substr(i), spec[group], true);
                if (!r.ok()) {
                    return ParseResponse<FormatString>::NOT_OK(i, r.error());
                }
                
                i += r.offset();
            }
            else {
                // Parse format specifiers
                while (true) {
                    ParseResult <FormatString::Specification::Specifier, FormatString> r = parse_specifier(in.substr(i));
                    
                    if (!r.ok()) {
                        return ParseResponse<FormatString>::NOT_OK(r.offset() + i, r.error());
                    }
                    
                    const FormatString::Specification::Specifier& specifier = r.result();
                    spec[group][specifier.name] = specifier.value;
                    
                    i += r.offset();
                    
                    if (in[i] != ',') {
                        break;
                    }
                    
                    // Skip format specifier separator ','
                    ++i;
                }
            }
        }
        
        return ParseResponse<FormatString>::OK(i);
    }
    
    // FormatString implementation
    
    template <String T>
    FormatString::FormatString(T fmt, std::source_location source) : m_format(fmt),
                                                                     m_source(source) {
        parse();
    }
    
    FormatString::~FormatString() = default;

    std::size_t FormatString::get_placeholder_count() const {
        return m_placeholders.size();
    }

    std::size_t FormatString::get_positional_placeholder_count() const {
        std::size_t highest = 0u;

        // The number of positional placeholders depends on the highest placeholder value encountered in the format string
        for (const Placeholder& placeholder : m_placeholders) {
            if (placeholder.identifier.type == Identifier::Type::Position) {
                // Positional placeholders indices start at 0
                highest = std::max(placeholder.identifier.position + 1, highest);
            }
        }

        return highest;
    }

    std::size_t FormatString::get_named_placeholder_count() const {
        std::size_t count = 0u;

        for (const Placeholder& placeholder : m_placeholders) {
            if (placeholder.identifier.type == Identifier::Type::Name) {
                ++count;
            }
        }

        return count;
    }

    void FormatString::parse() {
        if (m_format.empty()) {
            return;
        }
        
        std::string_view fmt = m_format;
        
        std::size_t length = fmt.length();
        std::size_t placeholder_start;
        std::size_t placeholder_offset = 0u;
        
        std::size_t i = 0u;
        std::size_t last_insert_position = 0u;
        
        while (i < length) {
            if (fmt[i] == '{') {
                if (i + 1u == length) {
                    throw FormattedError("unterminated '{' at index {}", i);
                }
                else if (fmt[i + 1u] == '{') {
                    // Escaped opening brace '{' should only be added to the resulting string once
                    ++i;
                    m_result.append(fmt.substr(last_insert_position, i - last_insert_position));
                    
                    ++i;
                    last_insert_position = i;
                }
                else {
                    placeholder_start = i;
                    
                    // Skip opening brace '{'
                    ++i;
                    
                    // Auto-numbered placeholder by default, case is not handled below
                    Identifier identifier { };
                    
                    if (std::isdigit(fmt[i])) {
                        // Positional placeholder
                        ++i;
                        
                        // Positional placeholders must only contain numbers
                        while (std::isdigit(fmt[i])) {
                            ++i;
                        }
                        
                        std::size_t position;
                        from_string(fmt.substr(placeholder_start, i - placeholder_start), position);
                        
                        identifier = Identifier(position);
                    }
                    // Named placeholders follow the same identifier rules as standard C/C++ identifiers
                    else if (std::isalpha(fmt[i]) || (fmt[i] == '_')) {
                        // Named placeholder
                        ++i;
                        
                        while (std::isalpha(fmt[i]) || std::isdigit(fmt[i]) || (fmt[i] == '_')) {
                            ++i;
                        }
                        
                        identifier = Identifier(fmt.substr(placeholder_start, i - placeholder_start));
                    }

                    char c = fmt[i];
                    
                    if (c != ':' && c != '}') {
                        throw FormattedError("invalid character '{}' at index {} - expecting formatting separator ':' or placeholder terminator '}'", fmt[i], i);
                    }
                    
                    // Skip format specification separator ':' or closing brace '}'
                    ++i;
                    
                    Specification spec { };
                    if (c == ':') {
                        // Parse custom formatting
                        ParseResponse<FormatString> r = parse_specification(fmt.substr(i), spec);
                        if (!r.ok()) {
                            // Throw exception with the error position relative to the start of the string
                            throw FormattedError(r.error(), NamedArgument("index", r.offset() + i));
                        }
                    }
                    
                    register_placeholder(identifier, spec, placeholder_start - placeholder_offset);
                    placeholder_offset += (i - placeholder_start) + 1;
                    
                    // Append everything up until the start of the placeholder
                    m_result.append(fmt.substr(last_insert_position, placeholder_start - last_insert_position));
                    last_insert_position = i;
                }
            }
            else if (fmt[i] == '}') {
                if ((i + 1u != length) && fmt[i + 1u] == '}') {
                    // Escaped closing brace '}' should only be added to the resulting string once
                    ++i;
                    m_result.append(fmt.substr(last_insert_position, i - last_insert_position));
                    
                    ++i;
                    last_insert_position = i;
                }
                else {
                    throw FormattedError("invalid '}' at index {} - closing brace literals must be escaped as '}}'", i);
                }
            }
            else {
                ++i;
            }
        }

        // Add all remaining characters
        m_result.append(fmt.substr(last_insert_position, i - last_insert_position));
        
        std::size_t num_positional_placeholders = get_positional_placeholder_count();
        
        // Issue a warning if not all positional arguments are used
        std::vector<bool> positional_placeholder_usage;
        positional_placeholder_usage.resize(num_positional_placeholders, false);
        
        for (const InsertionPoint& insertion_point : m_insertion_points) {
            const Identifier& identifier = m_placeholders[insertion_point.placeholder_index].identifier;
            if (identifier.type == Identifier::Type::Position) {
                positional_placeholder_usage[identifier.position] = true;
            }
        }
        
        for (i = 0u; i < num_positional_placeholders; ++i) {
            if (!positional_placeholder_usage[i]) {
                logging::warning("value for positional placeholder {} is never referenced in the format string", i);
            }
        }
    }
    
    void FormatString::register_placeholder(const Identifier& identifier, const Specification& spec, std::size_t position) {
        if (!m_placeholders.empty()) {
            // Verify format string homogeneity
            // The first placeholder determines the type of format string this is
            const Placeholder& placeholder = m_placeholders[0];
            
            if (identifier.type == Identifier::Type::Auto) {
                if (placeholder.identifier.type != Identifier::Type::Auto) {
                    // Format string mixes an auto-numbered placeholder with a named / positional placeholder, which is not allowed
                    throw FormattedError("format string placeholders must be homogeneous - auto-numbered placeholder at index {} cannot be mixed with positional/named placeholders (first encountered at index {})", position, m_insertion_points[0].position);
                }
            }
            else {
                if (placeholder.identifier.type == Identifier::Type::Auto) {
                    // Format string mixes a named / positional placeholder with an auto-numbered placeholder, which is not allowed
                    if (identifier.type == Identifier::Type::Position) {
                        throw FormattedError("format string placeholders must be homogeneous - positional placeholder {} at index {} cannot be mixed with positional/named placeholders (first encountered at index {})", identifier.position, position, m_insertion_points[0].position);
                    }
                    else {
                        throw FormattedError("format string placeholders must be homogeneous - named placeholder '{}' at index {} cannot be mixed with positional/named placeholders (first encountered at index {})", identifier.name, position, m_insertion_points[0].position);
                    }
                }
            }
        }

        std::size_t placeholder_index = m_placeholders.size(); // Invalid index
        if (identifier.type != Identifier::Type::Auto) {
            // A placeholder can be de-duplicated if both the identifier and the formatting specification match
            // Auto-numbered placeholders are never de-duplicated and always contribute as a new placeholder
            for (std::size_t i = 0u; i < m_placeholders.size(); ++i) {
                if (m_placeholders[i].identifier == identifier && m_placeholders[i].spec == spec) {
                    placeholder_index = i;
                    break;
                }
            }
        }
        
        if (placeholder_index == m_placeholders.size()) {
            m_placeholders.emplace_back(identifier, spec);
        }
        
        Placeholder& placeholder = m_placeholders[placeholder_index];

        // This way, insertion points are automatically sorted by insertion position
        m_insertion_points.emplace_back(placeholder_index, position);
    }
    
    bool FormatString::has_placeholder(std::size_t position) const {
        for (const Placeholder& placeholder : m_placeholders) {
            if (placeholder.identifier.type == Identifier::Type::Position && placeholder.identifier.position == position) {
                return true;
            }
        }
        
        return false;
    }
    
    bool FormatString::has_placeholder(std::string_view name) const {
        for (const Placeholder& placeholder : m_placeholders) {
            if (placeholder.identifier.type == Identifier::Type::Name && placeholder.identifier.name == name) {
                return true;
            }
        }
        
        return false;
    }
    
    const FormatString::Placeholder& FormatString::get_placeholder(std::size_t position) {
        for (const Placeholder& placeholder : m_placeholders) {
            if (placeholder.identifier.type == Identifier::Type::Position && placeholder.identifier.position == position) {
                return placeholder;
            }
        }
    
        throw FormattedError("placeholder at position {} does not exist", position);
    }
    
    const FormatString::Placeholder& FormatString::get_placeholder(std::string_view name) {
        for (const Placeholder& placeholder : m_placeholders) {
            if (placeholder.identifier.type == Identifier::Type::Position && placeholder.identifier.name == name) {
                return placeholder;
            }
        }
    
        throw FormattedError("placeholder with name '{}' does not exist", name);
    }
    
    FormatString::operator std::string() const {
        // Any placeholders that were not provided a value are replaced with a simplified version (contains no format specifiers)
        
        // Calculate the length of the resulting string
        std::size_t capacity = m_result.length();
        
        Formatter<std::size_t> position_formatter { };
        
        std::vector<std::size_t> placeholder_lengths;
        placeholder_lengths.resize(m_insertion_points.size(), 0);
        
        for (std::size_t i = 0u; i < m_insertion_points.size(); ++i) {
            const Placeholder& placeholder = m_placeholders[m_insertion_points[i].placeholder_index];
            const Identifier& identifier = placeholder.identifier;
            
            // Each placeholder is surrounded by braces '{...}'
            capacity += 2u;
            
            std::size_t length = 0u;
            switch (identifier.type) {
                case Identifier::Type::Position:
                    length = position_formatter.reserve(identifier.position);
                    break;
                case Identifier::Type::Name:
                    length = identifier.name.length();
                    break;
                case Identifier::Type::Auto:
                default:
                    // Auto-numbered placeholders have no content (length is 0)
                    break;
            }
            
            capacity += length;
            placeholder_lengths[i] = length;
        }
        
        // Allocate resulting capacity up front to prevent multiple (re)allocations while building resulting string
        std::string result;
        result.resize(capacity);
        
        std::size_t last_insert_position = 0u;
        std::size_t num_characters_written;
        
        for (std::size_t i = 0u; i < m_insertion_points.size(); ++i) {
            const InsertionPoint& insertion_point = m_insertion_points[i];
            
            const Placeholder& placeholder = m_placeholders[insertion_point.placeholder_index];
            const Identifier& identifier = placeholder.identifier;
            
            // Add everything up from the format string until the start of this placeholder
            result.replace(last_insert_position, insertion_point.position, std::string_view(m_result));
            num_characters_written = insertion_point.position - last_insert_position;
            last_insert_position = insertion_point.position;
            
            // Placeholder opening brace
            result[last_insert_position] = '{';
            ++num_characters_written;
            
            // Placeholder
            switch (identifier.type) {
                case Identifier::Type::Position: {
                    FormattingContext formatting_context { placeholder_lengths[i], &result[last_insert_position] };
                    position_formatter.format_to(identifier.position, formatting_context);
                    break;
                }
                case Identifier::Type::Name: {
                    result.replace(last_insert_position + num_characters_written, placeholder_lengths[i], identifier.name);
                    break;
                }
                case Identifier::Type::Auto:
                default:
                    // Auto-numbered placeholders have no content (length is 0)
                    break;
            }
            
            num_characters_written += placeholder_lengths[i];
            
            // Placeholder closing brace
            result[last_insert_position] = '}';
            ++num_characters_written;
            
            last_insert_position += num_characters_written;
        }
        
        result.replace(last_insert_position, m_result.length() - last_insert_position, std::string_view(m_result));
        return std::move(result);
    }
    
    std::string_view FormatString::format_string() const {
        return m_format;
    }
    
    std::source_location FormatString::source() const {
        return m_source;
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
    
    FormatString::Specification::Specification() : m_spec(),
                                                   m_type(Type::SpecifierList) {
    }

    FormatString::Specification::~Specification()= default;
    
    const FormatString::Specification& FormatString::Specification::operator[](std::size_t index) const {
        if (m_type == Type::SpecifierList) {
            throw FormattedError("bad format specification access - formatting group {} contains a mapping of specifier name/value pairs and cannot be accessed by index", index);
        }
        
        const FormattingGroupList& groups = std::get<FormattingGroupList>(m_spec);
        if (index >= groups.size()) {
            throw FormattedError("bad format specification access - formatting group {} does not exist (out of bounds)", index);
        }
        
        return *groups[index];
    }
    
    FormatString::Specification& FormatString::Specification::operator[](std::size_t index) {
        if (m_type == Type::SpecifierList) {
            // By default, formatting groups are initialized to specifier lists, which can be repurposed to be formatting groups if empty
            const SpecifierList& specifiers = std::get<SpecifierList>(m_spec);
            if (!specifiers.empty()) {
                throw FormattedError("bad format specification access - formatting group {} contains a mapping of specifier name/value pairs and cannot be accessed by index", index);
            }
    
            m_spec = FormattingGroupList { };
            m_type = Type::FormattingGroupList;
        }
        
        FormattingGroupList& groups = std::get<FormattingGroupList>(m_spec);
        
        while (index >= groups.size()) {
            // Allow a sparse vector of formatting groups to save on memory use
            groups.emplace_back(nullptr);
        }
        
        if (!groups[index]) {
            groups[index] = new Specification();
        }
        return *groups[index];
    }
    
    std::string_view FormatString::Specification::operator[](std::string_view key) const {
        if (m_type == Type::FormattingGroupList) {
            throw FormattedError("bad and/or ambiguous format specification access - specification contains multiple nested formatting groups and cannot be accessed by specifier (key: '{}')", key);
        }
        
        const SpecifierList& specifiers = std::get<SpecifierList>(m_spec);
        for (const Specifier& specifier : specifiers) {
            if (specifier.name == key) {
                return specifier.value;
            }
        }
        
        // Specifier not found
        throw FormattedError("bad format specification access - specifier with name '{}' not found", key);
    }

    std::string& FormatString::Specification::operator[](std::string_view key) {
        if (m_type == Type::FormattingGroupList) {
            // While formatting groups initialized to specifier lists can be converted to formatting group lists, the opposite of this operation is purposefully not supported
            // Initializing a nested formatting specification to a formatting group can only be done through an intentional operation
            throw FormattedError("bad and/or ambiguous format specification access - specification contains multiple nested formatting groups and cannot be accessed by specifier (key: '{}')", key);
        }
        
        SpecifierList& specifiers = std::get<SpecifierList>(m_spec);
        for (Specifier& specifier : specifiers) {
            if (specifier.name == key) {
                return specifier.value;
            }
        }
        
        // Specifier was not found, create a new entry
        return specifiers.emplace_back(std::string(key)).value;
    }
    
    FormatString::Specification::Type FormatString::Specification::type() const {
        return m_type;
    }
    
    std::size_t FormatString::Specification::size() const {
        // All internal types (std::vector) support the size operation
        static const auto visitor = []<typename T>(const T& data) -> std::size_t {
            return data.size();
        };
        return std::visit(visitor, m_spec);
    }
    
    bool FormatString::Specification::empty() const {
        // All internal types (std::vector) support the empty operation
        static const auto visitor = []<typename T>(const T& data) -> bool {
            return data.empty();
        };
        return std::visit(visitor, m_spec);
    }
    
    bool FormatString::Specification::has_group(std::size_t index) const {
        if (std::holds_alternative<SpecifierList>(m_spec)) {
            // TODO: calling has_group on a specifier list makes no sense, throw exception?
            return false;
        }
        
        const FormattingGroupList& groups = std::get<FormattingGroupList>(m_spec);
        return index < groups.size();
    }
    
    bool FormatString::Specification::has_specifier(std::string_view key) const {
        if (std::holds_alternative<FormattingGroupList>(m_spec)) {
            // TODO: calling has_group on a formatting group list makes no sense, throw exception?
            return false;
        }
        
        const SpecifierList& specifiers = std::get<SpecifierList>(m_spec);
        for (const Specifier& specifier : specifiers) {
            if (specifier.name == key) {
                return true;
            }
        }
        
        return false;
    }
    
    bool FormatString::Specification::operator==(const FormatString::Specification& other) const {
        if (m_type != other.m_type) {
            return false;
        }
        
        std::size_t s = size();
        
        if (s != other.size()) {
            return false;
        }
        
        switch (m_type) {
            case Type::FormattingGroupList: {
                const FormattingGroupList& groups = std::get<FormattingGroupList>(m_spec);
                const FormattingGroupList& other_groups = std::get<FormattingGroupList>(other.m_spec);
                
                for (std::size_t i = 0u; i < s; ++i) {
                    if (!groups[i] && !other_groups[i]) {
                        // Both are nullptr (equal)
                        continue;
                    }
                    
                    if (!groups[i] && other_groups[i] || groups[i] && !other_groups[i]) {
                        return false;
                    }
                    
                    // Both are valid pointers
                    if (*(groups[i]) == *(other_groups[i])) {
                        continue;
                    }
                }
                
                break;
            }
            case Type::SpecifierList: {
                const SpecifierList& specifiers = std::get<SpecifierList>(m_spec);
                const SpecifierList& other_specifiers = std::get<SpecifierList>(other.m_spec);
                
                for (std::size_t i = 0u; i < s; ++i) {
                    if (specifiers[i] != other_specifiers[i]) {
                        return false;
                    }
                }
                
                break;
            }
        }
        
        return true;
    }
    
    bool FormatString::Specification::operator!=(const FormatString::Specification& other) const {
        return !(*this == other);
    }
    
    // FormatString::Specification::Specifier implementation
    
    FormatString::Specification::Specifier::Specifier(std::string name, std::string value) : name(std::move(name)),
                                                                                             value(std::move(value)) {
    }
    
    bool FormatString::Specification::Specifier::operator==(const FormatString::Specification::Specifier& other) const {
        // Specifier names are case-insensitive
        return casecmp(name, other.name) && value == other.value;
    }
    
    bool FormatString::Specification::Specifier::operator!=(const FormatString::Specification::Specifier& other) const {
        return !(*this == other);
    }
    
    FormatString::Specification::Specifier::~Specifier() = default;

}