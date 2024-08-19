
#include "utils/format.hpp"
#include "utils/result.hpp"
#include "utils/string.hpp"
#include "utils/logging/logging.hpp"

#include <limits> // std::numeric_limits

namespace utils {
    
    namespace detail {
        
        int round_up_to_multiple(int value, int multiple) {
            if (multiple == 0) {
                return value;
            }
    
            int remainder = value % multiple;
            if (remainder == 0) {
                return value;
            }
    
            return value + multiple - remainder;
        }
        
        template <typename T>
        constexpr std::size_t count_digits(T num) {
            return (num < 10) ? 1 : 1 + count_digits(num / 10);
        }
        
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
    
    FormatString::FormatString(const FormatString& fmt) : m_format(fmt.m_format),
                                                          m_source(fmt.m_source),
                                                          m_identifiers(fmt.m_identifiers),
                                                          m_specifications(fmt.m_specifications),
                                                          m_placeholders(fmt.m_placeholders) {
    }
    
    FormatString::~FormatString() = default;


    

    
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
        m_placeholders.emplace_back(identifier_index, specification_index, position, false);
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

    FormatString::Specification::Specification(SpecifierList&& specifiers) : m_spec(std::move(specifiers)),
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
        // Try not to incur extra memory / performance overhead when the formatting specification only contains a specifier list
        // Use a specifier list in line in place of a formatting group list there is only one active group
        if (m_type == Type::SpecifierList) {
            if (index == 0u) {
                // Continue treating this as a specifier list and not a formatting group list
                return *this;
            }
            else {
                // When an additional group is requested, convert internal structure to a formatting group list
                // This first requires the conversion of this (specifier list) to the first formatting group
                m_spec = FormattingGroupList { new Specification(std::move(std::get<SpecifierList>(m_spec))) };
                m_type = Type::FormattingGroupList;
            }
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
            if (casecmp(specifier.name, key)) {
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