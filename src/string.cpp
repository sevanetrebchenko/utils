
#include "utils/string.hpp"
#include "utils/exceptions.hpp"

#include <limits> // std::numeric_limits
#include <charconv> // std::from_chars, std::from_chars_result

namespace utils {

    namespace detail {

        Identifier::Identifier() : type(Type::Auto),
                                   position(std::numeric_limits<std::size_t>::max()),
                                   name() {
        }
        
        Identifier::Identifier(std::size_t position) : type(Type::Position),
                                                       position(position),
                                                       name() {
        }
        
        Identifier::Identifier(std::string_view name) : type(Type::Name),
                                                        position(std::numeric_limits<std::size_t>::max()),
                                                        name(name) {
        }
        
        Identifier::~Identifier() = default;
        
        bool Identifier::operator==(const Identifier& other) const {
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
        
        std::size_t parse_identifier(std::string_view in, Identifier& out) {
            std::size_t offset = 0;

            if (std::isdigit(in[offset])) {
                ++offset;

                // Positional placeholders must only contain numbers
                while (std::isdigit(in[offset])) {
                    ++offset;
                }

                std::size_t position;
                from_string(in.substr(0, offset), position);

                out = Identifier(position);
            }
            else if (std::isalpha(in[offset]) || in[offset] == '_') {
                ++offset;

                // Named placeholders follow the same identifier rules as standard C/C++ identifiers
                while (std::isalnum(in[offset]) || in[offset] == '_') {
                    ++offset;
                }

                out = Identifier(in.substr(0, offset));
            }

            return offset;
        }

        std::size_t parse_specifier(std::string_view in, Specifier& out) {
            std::size_t length = in.length();
            std::size_t i = 0;

            // Specifier names follow the same rules as standard C/C++ identifiers
            while (std::isalpha(in[i]) || (in[i] == '_') || (i && std::isdigit(in[i]))) {
                ++i;
            }

            if (in[i] != '=') {
                return i;
            }

            out.name = in.substr(0, i);

            // Skip separator '='
            ++i;

            if (in[i] != '[') {
                return i;
            }

            // Skip opening brace '['
            std::size_t value_start = ++i;

            while (i < length) {
                if (in[i] == '[') {
                    if (i + 1 == length || in[i + 1] != '[') {
                        // Unterminated / unescaped opening braces are not allowed
                        return i;
                    }

                    // Skip escaped opening brace ']'
                    ++i;
                }
                else if (in[i] == ']') {
                    if (i + 1 == length || in[i + 1] != ']') {
                        break;
                    }

                    // Skip escaped closing brace ']'
                    ++i;
                }

                ++i;
            }

            out.value = in.substr(value_start, i - value_start);
            return i + 1;
        }

        void parse_specifier_value(std::string_view value, std::string& out) {
            if (value.empty()) {
                return;
            }

            std::size_t length = value.length();
            out.reserve(length);

            // Replace escaped brace characters
            for (std::size_t i = 0; i < length; ++i) {
                if (i + 1 < length && ((value[i] == '[' && value[i + 1] == '[') || (value[i] == ']' && value[i + 1] == ']'))) {
                    // Push back only one brace character
                    ++i;
                }
                out.push_back(value[i]);
            }
        }

        std::size_t parse_format_spec(std::string_view in, FormatSpec& out, bool nested) {
            // Note: function assumes input string does not contain a leading formatting group separator ':'
            std::size_t length = in.length();
            std::size_t group = 0;
            std::size_t i = 0;

            while (i < length) {
                char terminator = nested ? '|' : '}';
                if (in[i] == terminator) {
                    // Finished parsing format spec
                    break;
                }

                if (in[i] == ':') {
                    // Skip formatting group separator
                    // {identifier:representation=[...]}
                    //            ^
                    ++i;

                    // Note: empty groups are supported and are treated as empty format specifier lists
                    ++group;
                    continue;
                }

                if (in[i] == '|') {
                    // Skip nested formatting spec separator
                    // {identifier:|representation=[...]:justification=[...]|}
                    //             ^
                    ++i;

                    i += parse_format_spec(in.substr(i), out[group], true);

                    // After parsing a nested formatting spec, the terminator character is expected to be '|'
                    if (in[i] != '|') {
                        return i;
                    }
                }
                else {
                    // Parse format specifiers
                    while (true) {
                        Specifier specifier { };
                        i += parse_specifier(in.substr(i), specifier);

                        // Format specifiers must be unique
                        if (out[group].has_specifier(specifier.name)) {
                            throw;
                        }

                        parse_specifier_value(specifier.value, out[group][specifier.name]);

                        if (in[i] == ',') {
                            // Skip format specifier separator
                            ++i;
                            continue;
                        }
                        else if (in[i] == terminator || in[i] == ':') {
                            // End of format spec || new format group
                            break;
                        }
                        else {
                            // Invalid character
                            return i;
                        }
                    }
                }
            }

            return i;
        }

        char nibble_to_hexadecimal(const char* nibble) {
            if (nibble[0] == '0') {
                // 0---
                if (nibble[1] == '0') {
                    // 00--
                    if (nibble[2] == '0') {
                        // 000-
                        if (nibble[3] == '0') {
                            // 0000 (0)
                            return '0';
                        }
                        else {
                            // 0001 (1)
                            return '1';
                        }
                    }
                    else {
                        // 001-
                        if (nibble[3] == '0') {
                            // 0010 (2)
                            return '2';
                        }
                        else {
                            // 0011 (3)
                            return '3';
                        }
                    }
                }
                else {
                    // 01--
                    if (nibble[2] == '0') {
                        // 010-
                        if (nibble[3] == '0') {
                            // 0100 (4)
                            return '4';
                        }
                        else {
                            // 0101 (5)
                            return '5';
                        }
                    }
                    else {
                        // 011-
                        if (nibble[3] == '0') {
                            // 0110 (6)
                            return '6';
                        }
                        else {
                            // 0111 (7)
                            return '7';
                        }
                    }
                }
            }
            else {
                // 1---
                if (nibble[1] == '0') {
                    // 10--
                    if (nibble[2] == '0') {
                        // 100-
                        if (nibble[3] == '0') {
                            // 1000 (8)
                            return '8';
                        }
                        else {
                            // 1001 (9)
                            return '9';
                        }
                    }
                    else {
                        if (nibble[3] == '0') {
                            // 1010 (10)
                            return 'a';
                        }
                        else {
                            // 1011 (11)
                            return 'b';
                        }
                    }
                }
                else {
                    // 11--
                    if (nibble[2] == '0') {
                        // 110-
                        if (nibble[3] == '0') {
                            // 1100 (12)
                            return 'c';
                        }
                        else {
                            // 1101 (13)
                            return 'd';
                        }
                    }
                    else {
                        // 111-
                        if (nibble[3] == '0') {
                            // 1110 (14)
                            return 'e';
                        }
                        else {
                            // 1111 (15)
                            return 'f';
                        }
                    }
                }
            }
        }

    }

    [[nodiscard]] std::vector<std::string_view> split(std::string_view in, std::string_view delimiter) {
        std::vector<std::string_view> components { };

        std::size_t position;
        do {
            position = in.find(delimiter);
            components.emplace_back(in.substr(0, position));
            in = in.substr(position + delimiter.length());
        }
        while (position != std::string::npos);

        return std::move(components);
    }

    [[nodiscard]] std::string_view trim(std::string_view in) {
        if (in.empty()) {
            return "";
        }

        std::size_t start = 0;
        while (std::isspace(in[start])) {
            ++start;
        }

        std::size_t end = in.length() - 1;
        while (end != start && std::isspace(in[end])) {
            --end;
        }

        return in.substr(start, end + 1);
    }

    template <typename T>
    inline std::size_t fundamental_from_string(std::string_view in, T& out) {
        // Leading whitespace is not ignored
        std::string_view str = trim(in);

        if (str.empty()) {
            // throw FormattedError("");
        }

        // Only a leading '-' is permitted at the beginning
        if (str[0] == '+') {
            str = str.substr(1);
        }

        // Leading base prefixes are not recognized
        int base = 10;
        bool has_base = false;

        if (str.length() > 1) {
            if (str[0] == '0') {
                if (str[1] == 'x' || str[1] == 'X') {
                    // Hexadecimal
                    base = 16;
                    has_base = true;
                }
                else if (str[1] == 'b' || str[1] == 'B') {
                    // Binary
                    base = 2;
                    has_base = true;
                }
            }
        }

        if (has_base) {
            str = str.substr(2);
        }

        const char* start = str.data();
        const char* end = start + str.length();

        std::from_chars_result result { };

        if constexpr (is_integer_type<T>::value) {
            result = std::from_chars(start, end, out, base);
        }
        else {
            // std::format_chars::general supports both scientific and fixed representations
            result = std::from_chars(start, end, out, std::chars_format::general);
        }

        const auto& [ptr, error_code] = result;

        if (error_code == std::errc::invalid_argument) {
            // failed to convert
            throw FormattedError("");
        }

        if (error_code == std::errc::result_out_of_range) {
            out = std::numeric_limits<T>::max();
        }

        // Return the number of characters processed
        return ptr - start;
    }

    std::size_t from_string(std::string_view in, unsigned char& out) {
        return fundamental_from_string(in, out);
    }

    std::size_t from_string(std::string_view in, short& out) {
        return fundamental_from_string(in, out);
    }

    std::size_t from_string(std::string_view in, unsigned short& out) {
        return fundamental_from_string(in, out);
    }

    std::size_t from_string(std::string_view in, int& out) {
        return fundamental_from_string(in, out);
    }

    std::size_t from_string(std::string_view in, unsigned int& out) {
        return fundamental_from_string(in, out);
    }

    std::size_t from_string(std::string_view in, long& out) {
        return fundamental_from_string(in, out);
    }

    std::size_t from_string(std::string_view in, unsigned long& out) {
        return fundamental_from_string(in, out);
    }

    std::size_t from_string(std::string_view in, long long int& out) {
        return fundamental_from_string(in, out);
    }

    std::size_t from_string(std::string_view in, unsigned long long int& out) {
        return fundamental_from_string(in, out);
    }

    std::size_t from_string(std::string_view in, float& out) {
        return fundamental_from_string(in, out);
    }

    std::size_t from_string(std::string_view in, double& out) {
        return fundamental_from_string(in, out);
    }

    std::size_t from_string(std::string_view in, long double& out) {
        return fundamental_from_string(in, out);
    }

    // FormatSpec implementation
    
    FormatSpec::FormatSpec() : m_spec(std::monostate { }),
                               m_type(Type::SpecifierList) {
    }

    FormatSpec::FormatSpec(SpecifierList&& specifiers) : m_spec(std::move(specifiers)),
                                                         m_type(Type::SpecifierList) {
    }
    
    FormatSpec::FormatSpec(const FormatSpec& other) {
        *this = other;
    }
    
    FormatSpec::FormatSpec(FormatSpec&& other) noexcept {
        if (*this != other) {
            m_type = other.m_type;
            m_spec = std::move(other.m_spec);
        }
    }
    
    FormatSpec& FormatSpec::operator=(const FormatSpec& other) {
        if (*this == other) {
            return *this;
        }
        
        if (std::holds_alternative<std::monostate>(other.m_spec)) {
            // Assigning to a FormatSpec that currently has no data
            if (!std::holds_alternative<std::monostate>(m_spec) && m_type == Type::FormattingGroupList) {
                for (FormatSpec* spec : std::get<FormattingGroupList>(m_spec)) {
                    delete spec;
                }
            }
            // Reset to defaults
            m_type = Type::SpecifierList;
            m_spec = std::monostate { }; // std::vector assignment operator automatically cleans up specifier memory when m_spec is of type SpecifierList
        }
        else {
            m_type = other.m_type;
            
            if (other.m_type == Type::SpecifierList) {
                // Vector of specifiers can be copied directly
                m_spec = other.m_spec;
            }
            else {
                FormattingGroupList groups = FormattingGroupList();
                
                // Perform a deep copy of nested formatting groups
                for (const FormatSpec* spec : std::get<FormattingGroupList>(other.m_spec)) {
                    groups.emplace_back(new FormatSpec(*spec));
                }
                
                m_spec = std::move(groups);
            }
        }
        
        return *this;
    }
    
    FormatSpec::~FormatSpec() {
        if (m_type == Type::FormattingGroupList) {
            FormattingGroupList& groups = std::get<FormattingGroupList>(m_spec);
            for (FormatSpec* spec : groups) {
                delete spec;
            }
        }
    }
    
    FormatSpec::Type FormatSpec::type() const {
        return m_type;
    }

    std::size_t FormatSpec::size() const {
        static const auto visitor = []<typename T>(const T& data) -> std::size_t {
            if constexpr (std::is_same<T, std::monostate>::value) {
                return 0;
            }
            else {
                // SpecifierList and FormattingGroupList are both std::vector types
                return data.size();
            }
        };
        return std::visit(visitor, m_spec);
    }

    bool FormatSpec::empty() const {
        static const auto visitor = []<typename T>(const T& data) -> bool {
            if constexpr (std::is_same<T, std::monostate>::value) {
                return true;
            }
            else {
                // SpecifierList and FormattingGroupList are both std::vector types
                return data.empty();
            }
        };
        return std::visit(visitor, m_spec);
    }

    bool FormatSpec::operator!=(const FormatSpec& other) const {
        return !(*this == other);
    }
    
    bool FormatSpec::operator==(const FormatSpec& other) const {
        if (std::holds_alternative<std::monostate>(m_spec)) {
            if (std::holds_alternative<std::monostate>(other.m_spec)) {
                return true;
            }
            else {
                return false;
            }
        }
        else {
            if (std::holds_alternative<std::monostate>(other.m_spec)) {
                return false;
            }
            else {
                if (m_type != other.m_type) {
                    return false;
                }
                
                std::size_t _size = size();
                if (_size != other.size()) {
                    return false;
                }
                
                if (m_type == Type::SpecifierList) {
                    // Compare specifier values
                    const SpecifierList& specifiers = std::get<SpecifierList>(m_spec);
                    const SpecifierList& other_specifiers = std::get<SpecifierList>(other.m_spec);
                    
                    for (std::size_t i = 0; i < _size; ++i) {
                        if (specifiers[i] != other_specifiers[i]) {
                            return false;
                        }
                    }
                }
                else {
                    // Compare nested formatting groups
                    const FormattingGroupList& groups = std::get<FormattingGroupList>(m_spec);
                    const FormattingGroupList& other_groups = std::get<FormattingGroupList>(other.m_spec);
    
                    for (std::size_t i = 0; i < _size; ++i) {
                        // Compare pointers
                        if (!groups[i] && !other_groups[i]) {
                            continue;
                        }
                        if (!groups[i] && other_groups[i] || groups[i] && !other_groups[i]) {
                            return false;
                        }
    
                        // Compare nested format specs
                        if (*(groups[i]) != *(other_groups[i])) {
                            return false;
                        }
                    }
                }
                
                return true;
            }
        }
    }

    void FormatSpec::set_specifier(std::string_view key, std::string value) {
        if (std::holds_alternative<std::monostate>(m_spec)) {
            m_spec = SpecifierList();
            m_type = Type::SpecifierList;
        }
        else if (m_type == Type::FormattingGroupList) {
            // Format specifier-related functions are not available on formatting group lists
            throw FormattedError("bad and/or ambiguous format specification access - specification contains nested formatting group(s) and cannot be accessed by specifier (key: '{}')", key);
        }

        SpecifierList& specifiers = std::get<SpecifierList>(m_spec);
        for (Specifier& specifier : specifiers) {
            if (icasecmp(specifier.name, key)) {
                specifier.value = std::move(value);
                return;
            }
        }

        // Specifier not found, create a new entry
        specifiers.emplace_back(std::string(key), std::move(value));
    }

    std::string& FormatSpec::operator[](std::string_view key) {
        return get_specifier(key);
    }

    std::string_view FormatSpec::operator[](std::string_view key) const {
        return get_specifier(key);
    }

    std::string& FormatSpec::get_specifier(std::string_view key) {
        if (std::holds_alternative<std::monostate>(m_spec)) {
            m_spec = SpecifierList();
            m_type = Type::SpecifierList;
        }
        else if (m_type == Type::FormattingGroupList) {
            // Format specifier-related functions are not available on formatting group lists
            throw FormattedError("bad and/or ambiguous format specification access - specification contains nested formatting group(s) and cannot be accessed by specifier (key: '{}')", key);
        }

        SpecifierList& specifiers = std::get<SpecifierList>(m_spec);
        for (Specifier& specifier : specifiers) {
            if (icasecmp(specifier.name, key)) {
                return specifier.value;
            }
        }

        // Specifier was not found, create a new entry
        return specifiers.emplace_back(std::string(key), "").value;
    }

    std::string_view FormatSpec::get_specifier(std::string_view key) const {
        if (m_type == Type::FormattingGroupList) {
            // Format specifier-related functions are not available on formatting group lists
            throw FormattedError("bad and/or ambiguous format specification access - specification contains nested formatting group(s) and cannot be accessed by specifier (key: '{}')", key);
        }

        for (const Specifier& specifier : std::get<SpecifierList>(m_spec)) {
            if (icasecmp(specifier.name, key)) {
                return specifier.value;
            }
        }

        // Specifier not found
        throw FormattedError("bad format specification access - specifier with name '{}' not found", key);
    }

    FormatSpec& FormatSpec::operator[](std::size_t index) {
        return get_group(index);
    }

    const FormatSpec& FormatSpec::operator[](std::size_t index) const {
        return get_group(index);
    }

    FormatSpec& FormatSpec::get_group(std::size_t index) {
        // Try not to incur extra memory / performance overhead when the formatting specification only contains a specifier list
        // Use a specifier list in line in place of a formatting group list there is only one active group
        if (m_type == Type::SpecifierList) {
            if (index == 0u) {
                // get_group(0) - continue treating this as a specifier list
                return *this;
            }
            else {
                // When an additional group is requested, convert internal structure to a formatting group list
                // This first requires the conversion of this (specifier list) to the first formatting group
                if (empty()) {
                    m_spec = FormattingGroupList();
                }
                else {
                    m_spec = FormattingGroupList { new FormatSpec(std::move(std::get<SpecifierList>(m_spec))) };
                }
                
                m_type = Type::FormattingGroupList;
            }
        }

        FormattingGroupList& groups = std::get<FormattingGroupList>(m_spec);
        while (index >= groups.size()) {
            // Allow a sparse vector of formatting groups to save on memory use
            groups.emplace_back(nullptr);
        }

        if (!groups[index]) {
            groups[index] = new FormatSpec();
        }
        return *groups[index];
    }

    const FormatSpec& FormatSpec::get_group(std::size_t index) const {
        if (m_type == Type::SpecifierList) {
            throw FormattedError("bad format specification access - formatting group {} contains a mapping of specifier name/value pairs and cannot be accessed by index", index);
        }

        const FormattingGroupList& groups = std::get<FormattingGroupList>(m_spec);
        if (index < groups.size()) {
            return *groups[index];
        }

        throw FormattedError("bad format specification access - formatting group {} does not exist (index out of bounds)", index);
    }

    bool FormatSpec::has_group(std::size_t index) const {
        if (std::holds_alternative<std::monostate>(m_spec)) {
            // Uninitialized format spec
            return false;
        }
        else if (m_type == Type::SpecifierList) {
            throw FormattedError("bad format specification access - has_group called on specifier list");
        }

        const FormattingGroupList& groups = std::get<FormattingGroupList>(m_spec);
        return index < groups.size() && groups[index];
    }

    bool FormatSpec::has_specifier(std::string_view key) const {
        if (std::holds_alternative<std::monostate>(m_spec)) {
            // Uninitialized format spec
            return false;
        }
        else if (m_type == Type::FormattingGroupList) {
            throw FormattedError("bad format specification access - has_specifier called on formatting group list");
        }
        
        for (const Specifier& specifier : std::get<SpecifierList>(m_spec)) {
            if (icasecmp(specifier.name, key)) {
                return true;
            }
        }

        return false;
    }
    
    FormatSpec::Specifier::Specifier(std::string name, std::string value) : name(std::move(name)),
                                                                            value(std::move(value)) {
    }

    FormatSpec::Specifier::~Specifier() = default;

    FormatSpec::Specifier& FormatSpec::Specifier::operator=(std::string other) {
        value = std::move(other);
        return *this;
    }

    bool FormatSpec::Specifier::operator==(const FormatSpec::Specifier& other) const {
        return icasecmp(name, other.name) && icasecmp(value, other.value);
    }

    bool FormatSpec::Specifier::operator!=(const FormatSpec::Specifier& other) const {
        return !(*this == other);
    }
    
    FormatString::FormatString(const std::string& format, std::source_location source) : format(format),
                                                                                         source(source) {
    }
    
    FormatString::FormatString(std::string_view format, std::source_location source) : format(format),
                                                                                       source(source) {
    }
    
    FormatString::FormatString(const char* format, std::source_location source) : format(format),
                                                                                  source(source) {
    }
    
    FormatString::~FormatString() = default;
    
    FormatterBase::FormatterBase() : justification(Justification::Left),
                                     width(0),
                                     fill_character(' '),
                                     style(Style::None),
                                     color() {
    }
    
    FormatterBase::~FormatterBase() = default;
    
    void FormatterBase::parse(const FormatSpec& spec) {
        if (spec.has_specifier("justification", "apply_justification", "alignment", "align")) {
            std::string_view value = trim(spec.get_specifier("justification", "apply_justification", "alignment", "align").value);
            if (icasecmp(value, "left")) {
                justification = Justification::Left;
            }
            else if (icasecmp(value, "right")) {
                justification = Justification::Right;
            }
            else if (icasecmp(value, "center")) {
                justification = Justification::Center;
            }
        }

        if (spec.has_specifier("width")) {
            std::string_view value = trim(spec.get_specifier("width"));

            unsigned _width;
            std::size_t num_characters_read = from_string(value, _width);

            if (num_characters_read > 0) {
                width = _width;
            }
        }

        if (spec.has_specifier("fill", "fill_character", "fillcharacter")) {
            std::string_view value = trim(spec.get_specifier("fill", "fill_character", "fillcharacter").value);
            if (!value.empty()) {
                fill_character = value[0];
            }
        }
        
        if (spec.has_specifier("style")) {
            std::string_view value = trim(spec.get_specifier("style"));
            
            if (icasecmp(value, "bold")) {
                style = Style::Bold;
            }
            else if (icasecmp(value, "italic") || icasecmp(value, "italicized")) {
                style = Style::Italicized;
            }
        }
        
        if (spec.has_specifier("color")) {
            std::string_view value = trim(spec.get_specifier("color"));
            
            // Color can either be provided as an RGB triplet, an 8-bit ANSI color code (0-255), or by name (see color.hpp)
            auto iter = colors.find(value);
            if (iter != colors.end()) {
                // Color specified by name
                color = iter->second;
            }
            else {
                std::vector<std::string_view> components = split(value, ",");
                std::size_t size = components.size();
                
                if (!components.empty()) {
                    if (size == 1) {
                        // 8-bit ANSI code
                        std::uint8_t code = from_string(components[0], code);
                        color = ansi_to_rgb(code);
                    }
                    else if (size >= 3) {
                        std::uint8_t r, g, b;
                        from_string(components[0], r);
                        from_string(components[1], g);
                        from_string(components[2], b);
                        color = { r, g, b };
                    }
                }
            }
        }
    }
    
    std::string FormatterBase::format(const std::string& value) const {
        return format(value.c_str(), value.length());
    }
    
    std::string FormatterBase::format(const char* value, std::size_t length) const {
        // Include capacity for ANSI codes
        // ANSI format: [{style};38;2;{red};{green};{blue}m{...}[0m
        // Note: this should only be applied if there are styling options requested
        
        std::string prefix, suffix;
        
        if (style != Style::None || color) {
            prefix = "\x1b[";
            
            if (style == Style::Bold) {
                prefix += "1;";
            }
            else if (style == Style::Italicized) {
                prefix += "3;";
            }
            
            if (color) {
                const Color& c = *color;
                prefix += "38;2;";
                
                char buffer[5] = { 0 }; // Buffer for holding values in the range [0, 255] + terminating semicolon
                char* end = buffer + sizeof(buffer) / sizeof(buffer[0]);
                
                std::to_chars_result result;
                
                // r
                result = std::to_chars(buffer, end, color->r, 10);
                buffer[result.ptr - buffer] = ';';
                
                prefix += buffer;
                
                // g
                result = std::to_chars(buffer, end, color->g, 10);
                buffer[result.ptr - buffer] = ';';
                
                prefix += buffer;
                
                // b
                result = std::to_chars(buffer, end, color->b, 10);
                buffer[result.ptr - buffer] = '\0';
                
                prefix += buffer;
            }
            prefix += 'm';
            
            suffix = "\033[0m";
        }
        
        // Color codes should not contribute to the padding of the formatted string
        std::size_t capacity = std::max(length, width);
        std::string result(capacity, fill_character);
        
        std::size_t write_position;
        if (justification == Justification::Left) {
            write_position = 0;
        }
        else if (justification == Justification::Right) {
            write_position = capacity - length;
        }
        else {
            write_position = (capacity - length) / 2;
        }

        // Insert the prefix, value, and suffix
        result.replace(0, prefix.length(), prefix);
        write_position += prefix.length();
        
        result.replace(write_position, length, value);
        write_position += length;
        
        result.replace(write_position, suffix.length(), suffix);
        // write_position += suffix.length();

        return std::move(result);
    }
    
    Formatter<char>::Formatter() : FormatterBase() {
    }
    
    Formatter<char>::~Formatter() = default;
    
    void Formatter<char>::parse(const utils::FormatSpec& spec) {
        ASSERT(spec.type() == FormatSpec::Type::SpecifierList, "format spec for character types must be a specifier list");
        FormatterBase::parse(spec);
    }
    
    std::string Formatter<char>::format(char c) const {
        return std::move(FormatterBase::format(&c, 1));
    }
    
    Formatter<const char*>::Formatter() : FormatterBase() {
    }
    
    Formatter<const char*>::~Formatter() = default;
    
    void Formatter<const char*>::parse(const utils::FormatSpec& spec) {
        ASSERT(spec.type() == FormatSpec::Type::SpecifierList, "format spec for string types must be a specifier list");
        FormatterBase::parse(spec);
    }
    
    std::string Formatter<const char*>::format(const char* value) const {
        return std::move(format(value, std::strlen(value)));
    }
    
    std::string Formatter<const char*>::format(const char* value, std::size_t length) const {
        return std::move(FormatterBase::format(value, length));
    }
    
    std::string Formatter<std::string_view>::format(std::string_view value) const {
        return Formatter<const char*>::format(value.data(), value.length());
    }
    
    std::string Formatter<std::string>::format(const std::string& value) const {
        return Formatter<const char*>::format(value.c_str(), value.length());
    }
    
    Formatter<void*>::Formatter() : IntegerFormatter<std::uintptr_t>() {
        // Pointers are always formatted using hexadecimal
        representation = Representation::Hexadecimal;
        use_base_prefix = true;
    }
    
    void Formatter<void*>::parse(const FormatSpec& spec) {
        ASSERT(spec.type() == FormatSpec::Type::SpecifierList, "format spec for pointer types must be a specifier list");
        FormatterBase::parse(spec);

        if (spec.has_specifier("use_separator", "useseparator", "use_separator_character", "useseparatorcharacter")) {
            std::string_view value = trim(spec.get_specifier("use_separator", "useseparator", "use_separator_character", "useseparatorcharacter").value);
            if (icasecmp(value, "true") || icasecmp(value, "1")) {
                use_separator_character = true;
            }
            else if (icasecmp(value, "false") || icasecmp(value, "0")) {
                use_separator_character = false;
            }
        }

        if (spec.has_specifier("group_size", "groupsize")) {
            std::string_view value = trim(spec.get_specifier("group_size", "groupsize").value);

            unsigned _group_size;
            std::size_t num_characters_read = from_string(value, _group_size);

            if (num_characters_read > 0) {
                group_size = _group_size;
            }
        }

        if (spec.has_specifier("use_base_prefix", "usebaseprefix")) {
            std::string_view value = trim(spec.get_specifier("use_base_prefix", "usebaseprefix").value);
            if (icasecmp(value, "true") || icasecmp(value, "1")) {
                use_base_prefix = true;
            }
            else if (icasecmp(value, "false") || icasecmp(value, "0")) {
                use_base_prefix = false;
            }
        }
    }
    
    Formatter<void*>::~Formatter() = default;
    
    std::string Formatter<void*>::format(void* value) const {
        if (value) {
            return std::move(IntegerFormatter<std::uintptr_t>::format((std::uintptr_t) value));
        }
        else {
            // Null pointers are printed as 'nullptr' and use a subset of the available formatting specifiers
            return std::move(FormatterBase::format("nullptr"));
        }
    }
    
    Formatter<std::source_location>::Formatter() : m_file_formatter(),
                                                   m_line_formatter() {
    }

    Formatter<std::source_location>::~Formatter() = default;

    void Formatter<std::source_location>::parse(const FormatSpec& spec) {
        if (spec.type() == FormatSpec::Type::SpecifierList) {
            FormatterBase::parse(spec);
        }
        else {
            if (spec.has_group(0)) {
                const FormatSpec& group = spec.get_group(0);
                ASSERT(group.type() == FormatSpec::Type::SpecifierList, "invalid std::source_location format spec - formatting group 0 must be a specifier list");
                FormatterBase::parse(group);
            }
            if (spec.has_group(1)) {
                const FormatSpec& group = spec.get_group(1);
                ASSERT(group.type() == FormatSpec::Type::SpecifierList, "invalid std::source_location format spec - formatting group 1 (file) must be a specifier list");
                m_file_formatter.parse(group);
            }
            if (spec.has_group(2)) {
                const FormatSpec& group = spec.get_group(2);
                ASSERT(group.type() == FormatSpec::Type::SpecifierList, "invalid std::source_location format spec - formatting group 2 (line) must be a specifier list");
                m_line_formatter.parse(group);
            }
        }
    }

    std::string Formatter<std::source_location>::format(const std::source_location& value) const {
        // Format: file:line
        return std::move(FormatterBase::format(m_file_formatter.format(value.file_name()) + ':' + m_line_formatter.format(value.line())));
    }
    
    std::string Formatter<std::filesystem::path>::format(const std::filesystem::path& value) const {
        return std::move(Formatter<std::string>::format(value.string()));
    }
    
    std::string Formatter<std::thread::id>::format(const std::thread::id& value) const {
        std::hash<std::thread::id> hash { };
        return std::move(Formatter<std::size_t>::format(hash(value)));
    }
    
}
