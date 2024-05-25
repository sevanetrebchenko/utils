
#include "utils/format.hpp"
#include "utils/result.hpp"
#include "utils/string.hpp"
#include "utils/logging/logging.hpp"

#include <limits> // std::numeric_limits
#include <iostream>

namespace utils {
    
    template <typename T>
    constexpr std::size_t count_digits(T num) {
        return (num < 10) ? 1 : 1 + count_digits(num / 10);
    }
    
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
        
        std::string specifier = std::string(in.substr(0, i));
        
        // Skip separator '='
        ++i;
        
        if (in[i] != '[') {
            return ParseResult<FormatString::Specification::Specifier, FormatString>::NOT_OK(i, "invalid character '{0}' at index {index} - format specifier value must be contained within square braces: [ ... ]", in[i]);
        }
        
        // Skip specifier value opening brace '['
        std::size_t specifier_value_start = ++i;
        std::size_t escaped_brace_count = 0u;
        
        // Determine specifier value length
        while (i < length) {
            if (in[i] == '[') {
                if (i + 1u == length) {
                    // Unterminated specifier value opening brace '['
                    break;
                }
                
                if (in[i + 1u] != '[') {
                    return ParseResult<FormatString::Specification::Specifier, FormatString>::NOT_OK(i, "unescaped '[' at index {index} - opening formatting brace literals must be escaped as '[[' inside specifier values");
                }
                
                // Escaped opening specifier value brace '[['
                ++escaped_brace_count;
                i += 2u;
            }
            else if (in[i] == ']') {
                if (i + 1u < length && in[i + 1u] == ']') {
                    // Escaped closing specifier value brace ']]'
                    ++escaped_brace_count;
                    i += 2u;
                }
                else {
                    // Closing specifier value brace ']'
                    break;
                }
            }
            else {
                ++i;
            }
        }
        
        if (in[i] != ']') {
            return ParseResult<FormatString::Specification::Specifier, FormatString>::NOT_OK(specifier_value_start, "unterminated formatting specifier value at index {index}");
        }
        
        // Build specifier value
        std::string value;
        std::size_t specifier_value_length = i - specifier_value_start - escaped_brace_count;
        value.reserve(specifier_value_length);
        
        std::size_t last_insert_position = specifier_value_start;
        for (std::size_t j = specifier_value_start; j < i; ++j) {
            bool escaped_brace = (in[j] == '[' && in[j + 1u] == '[') ||
                                 (in[j] == ']' && in[j + 1u] == ']');
            
            if (escaped_brace) {
                // Insert everything up until the first escaped brace
                value.append(in, last_insert_position, j - last_insert_position);
                last_insert_position = ++j; // SKip second escaped brace
            }
        }
        
        value.append(in, last_insert_position, i - last_insert_position);
        
        // Skip specifier value closing brace ']'
        ++i;
        
        return ParseResult<FormatString::Specification::Specifier, FormatString>::OK(i, std::move(specifier), std::move(value));
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
    
    FormatString::FormatString(const FormatString& fmt) : m_format(fmt.m_format),
                                                          m_source(fmt.m_source),
                                                          m_identifiers(fmt.m_identifiers),
                                                          m_specifications(fmt.m_specifications),
                                                          m_placeholders(fmt.m_placeholders) {
    }
    
    FormatString::~FormatString() = default;

    std::size_t FormatString::get_placeholder_count() const {
        return m_placeholders.size();
    }

    std::size_t FormatString::get_positional_placeholder_count() const {
        std::size_t highest = 0u;

        // The number of positional placeholders depends on the highest placeholder value encountered in the format string
        for (const Placeholder& placeholder : m_placeholders) {
            const Identifier& identifier = m_identifiers[placeholder.identifier_index];
            if (identifier.type == Identifier::Type::Position) {
                // Positional placeholders indices start at 0
                highest = std::max(identifier.position + 1, highest);
            }
        }

        return highest;
    }

    std::size_t FormatString::get_named_placeholder_count() const {
        std::size_t count = 0u;

        for (const Placeholder& placeholder : m_placeholders) {
            if (m_identifiers[placeholder.identifier_index].type == Identifier::Type::Name) {
                ++count;
            }
        }

        return count;
    }
    
    void FormatString::parse() {
        if (m_format.empty()) {
            return;
        }
        
        std::size_t length = m_format.length();
        std::size_t placeholder_start;
        
        std::size_t i = 0u;
        
        while (i < length) {
            if (m_format[i] == '{') {
                if (i + 1u == length) {
                    throw FormattedError("unterminated '{' at index {}", i);
                }
                else if (m_format[i + 1u] == '{') {
                    // Escaped opening brace '{{', keep only one
                    std::memcpy(&m_format[i], &m_format[i + 1u], m_format.length() - i);
                    
                    m_format.resize(length - 1u);
                    length = m_format.length();
                    
                    ++i;
                }
                else {
                    // Skip placeholder opening brace '{'
                    placeholder_start = i++;

                    Identifier identifier { }; // Auto-numbered by default
                    if (std::isdigit(m_format[i])) {
                        // Positional placeholder
                        ++i;

                        // Positional placeholders must only contain numbers
                        while (std::isdigit(m_format[i])) {
                            ++i;
                        }

                        std::size_t position;
                        from_string(m_format.substr(placeholder_start + 1u, i - (placeholder_start + 1u)), position);

                        identifier = Identifier(position);
                    }
                    // Named placeholders follow the same identifier rules as standard C/C++ identifiers
                    else if (std::isalpha(m_format[i]) || (m_format[i] == '_')) {
                        // Named placeholder
                        ++i;

                        while (std::isalpha(m_format[i]) || std::isdigit(m_format[i]) || (m_format[i] == '_')) {
                            ++i;
                        }

                        identifier = Identifier(std::string(m_format.substr(placeholder_start + 1u, i - (placeholder_start + 1u))));
                    }

                    if (m_format[i] != ':' && m_format[i] != '}') {
                        throw FormattedError("invalid character '{}' at index {} - expecting formatting separator ':' or placeholder terminator '}'", m_format[i], i);
                    }

                    Specification spec { };
                    if (m_format[i] == ':') {
                        // Skip format specification separator ':'
                        ++i;

                        // Parse custom formatting (consumes placeholder closing brace '}')
                        ParseResponse<FormatString> r = parse_specification(m_format.substr(i), spec);
                        if (!r.ok()) {
                            // Throw exception with the error position relative to the start of the string
                            throw FormattedError(r.error(), NamedArgument("index", r.offset() + i));
                        }

                        i += r.offset();
                    }
                    else {
                        // Skip placeholder closing brace '}'
                        ++i;
                    }

                    register_placeholder(identifier, spec, placeholder_start);
                    
                    std::memcpy(&m_format[placeholder_start], &m_format[i], length - i);
                    
                    m_format.resize(length - (i - placeholder_start));
                    length = m_format.length();
                    
                    // Reset parsing position to the start of the placeholder
                    i = placeholder_start;
                }
            }
            else if (m_format[i] == '}') {
                if ((i + 1u != length) && m_format[i + 1u] == '}') {
                    // Escaped closing brace '}}', keep only one
                    std::memcpy(&m_format[i], &m_format[i + 1u], m_format.length() - i);
                    
                    m_format.resize(length - 1u);
                    length = m_format.length();
                    
                    ++i;
                }
                else {
                    throw FormattedError("invalid '}' at index {} - closing brace literals must be escaped as '}}'", i);
                }
            }
            else {
                ++i;
            }
        }
        
//        builder.add_section(last_insert_position, i);
//        m_format = std::move(builder.build(fmt));
//
        std::size_t positional_placeholder_count = get_positional_placeholder_count();
        
        // Issue a warning if not all positional arguments are used
        for (std::size_t position = 0u; position < positional_placeholder_count; ++position) {
            bool found = false;
            for (const Placeholder& placeholder : m_placeholders) {
                const Identifier& identifier = m_identifiers[placeholder.identifier_index];
                if (identifier.type == Identifier::Type::Position && identifier.position == position) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                logging::warning("value for positional placeholder {} is never referenced in the format string", i);
            }
        }
    }
    
    void FormatString::register_placeholder(const Identifier& identifier, const Specification& spec, std::size_t position) {
        if (!m_placeholders.empty()) {
            // Verify format string homogeneity
            // The identifier of the first placeholder determines the type of format string this is
            Identifier::Type type = m_identifiers[m_placeholders[0].identifier_index].type;
            
            if (type == Identifier::Type::Auto) {
                if (identifier.type == Identifier::Type::Position) {
                    throw FormattedError("format string placeholders must be homogeneous - positional placeholder {} at index {} cannot be mixed with auto-numbered placeholders (first encountered at index {})", identifier.position, position, m_placeholders[0].position);
                }
                else if (identifier.type == Identifier::Type::Name) {
                    throw FormattedError("format string placeholders must be homogeneous - named placeholder '{}' at index {} cannot be mixed with auto-numbered placeholders (first encountered at index {})", identifier.name, position, m_placeholders[0].position);
                }
            }
            else {
                // Format string mixes a named / positional placeholder with an auto-numbered placeholder, which is not allowed
                if (identifier.type == Identifier::Type::Auto) {
                    // Format string mixes an auto-numbered placeholder with a named / positional placeholder, which is not allowed
                    throw FormattedError("format string placeholders must be homogeneous - auto-numbered placeholder at index {} cannot be mixed with positional/named placeholders (first encountered at index {})", position, m_placeholders[0].position);
                }
            }
        }
        
        std::size_t num_identifiers = m_identifiers.size();
        std::size_t identifier_index = num_identifiers;
        
        for (std::size_t i = 0u; i < num_identifiers; ++i) {
            if (m_identifiers[i] == identifier) {
                identifier_index = i;
                break;
            }
        }
        
        if (identifier_index == num_identifiers) {
            m_identifiers.emplace_back(identifier);
        }
        
        // Find (or register) new format specification
        std::size_t num_format_specifications = m_specifications.size();
        std::size_t specification_index = num_format_specifications;
        
        for (std::size_t i = 0u; i < num_format_specifications; ++i) {
            if (m_specifications[i] == spec) {
                specification_index = i;
                break;
            }
        }
        
        if (specification_index == num_format_specifications) {
            // Register new identifier
            m_specifications.emplace_back(spec);
        }
    
        // Placeholders are automatically sorted by their position in the format string
        m_placeholders.emplace_back(identifier_index, specification_index, position);
    }
    
    FormatString::operator std::string() const {
        if (m_placeholders.empty()) {
            return m_format;
        }
        
        Formatter<std::size_t> position_formatter { }; // This formatter is used to format positional placeholder positions, default-initialization is good enough
        std::size_t capacity = m_format.length();
        
        for (const Placeholder& placeholder : m_placeholders) {
            const Identifier& identifier = m_identifiers[placeholder.identifier_index];
            switch (identifier.type) {
                case Identifier::Type::Auto:
                    break;
                case Identifier::Type::Position:
                    // Can also use Formatter<T>::reserve for this, but this is evaluated at compile time and is faster
                    capacity += count_digits(identifier.position);
                    break;
                case Identifier::Type::Name:
                    capacity += identifier.name.length();
                    break;
            }
            
            // Add capacity for placeholder braces (one opening brace + one closing brace)
            capacity += 2u;
        }
        
        std::string result = m_format;
        result.reserve(capacity);
        
        std::size_t placeholder_offset = 0u;
        std::size_t write_position;
        std::size_t length;
        
        for (const Placeholder& placeholder : m_placeholders) {
            write_position = placeholder.position + placeholder_offset;
            
            result.insert(write_position++, "{");
            const Identifier& identifier = m_identifiers[placeholder.identifier_index];
            switch (identifier.type) {
                case Identifier::Type::Position: {
                    length = count_digits(placeholder.position);
                    FormattingContext context { length, &result[write_position] };
                    position_formatter.format_to(identifier.position, context);
                    break;
                }
                case Identifier::Type::Name:
                    length = identifier.name.length();
                    result.insert(write_position, identifier.name);
                    break;
                default:
                    // Auto-numbered placeholders are empty
                    length = 0u;
                    break;
            }
            write_position += length;
            result.insert(write_position++, "}");
            
            placeholder_offset += length + 2u;
        }
        
        return std::move(result);
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
        
        for (const Specifier& specifier : std::get<SpecifierList>(m_spec)) {
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
                    if (*(groups[i]) != *(other_groups[i])) {
                        return false;
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