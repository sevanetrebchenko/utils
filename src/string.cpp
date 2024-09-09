//
//#include "utils/string.hpp"
//#include "utils/result.hpp"
//#include "utils/assert.hpp"
//#include "utils/detail/string.tpp"
//
//#include <limits> // std::numeric_limits
//#include <charconv> // std::from_chars, std::from_chars_result
//#include <cstring> // std::memcpy, std::strlen
//#include <shared_mutex> // std::shared_mutex, std::shared_lock, std::unique_lock
//
//namespace utils {
//
//    namespace detail {
//
//        std::size_t apply_justification(std::uint8_t justification, char fill_character, std::size_t length, std::string& out) {
//            std::size_t capacity = out.length();
//            std::size_t start = 0u;
//
//            char fill = ' ';
//            if (fill_character) {
//                fill = fill_character;
//            }
//
//            if (length < capacity) {
//                // Formatted string
//                switch (justification) {
//                    case 0:
//                        // Justify left (append fill character from the right)
//                        for (std::size_t i = length; i < capacity; ++i) {
//                            out[i] = fill;
//                        }
//                        break;
//                    case 1:
//                        // Justify right (append fill character from the left)
//                        start = (capacity - 1u) - length;
//                        for (std::size_t i = 0u; i < start; ++i) {
//                            out[i] = fill;
//                        }
//                        break;
//                    case 2:
//                        // Justify center (append fill character on both sides equally)
//                        start = (capacity - length) / 2;
//
//                        // Left side
//                        for (std::size_t i = 0u; i < start; ++i) {
//                            out[i] = fill;
//                        }
//
//                        // Right side (account for additional character in the case that width is odd)
//                        std::size_t last = capacity - 1u;
//                        for (std::size_t i = 0u; i < start + ((capacity - length) % 2); ++i) {
//                            out[last - i] = fill;
//                        }
//                        break;
//                }
//            }
//
//            return start;
//        }
//
//        int round_up_to_multiple(int value, int multiple) {
//            if (multiple == 0) {
//                return value;
//            }
//
//            int remainder = value % multiple;
//            if (remainder == 0) {
//                return value;
//            }
//
//            return value + multiple - remainder;
//        }
//
//        char nibble_to_hexadecimal(const char* nibble) {
//            if (nibble[0] == '0') {
//                // 0---
//                if (nibble[1] == '0') {
//                    // 00--
//                    if (nibble[2] == '0') {
//                        // 000-
//                        if (nibble[3] == '0') {
//                            // 0000 (0)
//                            return '0';
//                        }
//                        else {
//                            // 0001 (1)
//                            return '1';
//                        }
//                    }
//                    else {
//                        // 001-
//                        if (nibble[3] == '0') {
//                            // 0010 (2)
//                            return '2';
//                        }
//                        else {
//                            // 0011 (3)
//                            return '3';
//                        }
//                    }
//                }
//                else {
//                    // 01--
//                    if (nibble[2] == '0') {
//                        // 010-
//                        if (nibble[3] == '0') {
//                            // 0100 (4)
//                            return '4';
//                        }
//                        else {
//                            // 0101 (5)
//                            return '5';
//                        }
//                    }
//                    else {
//                        // 011-
//                        if (nibble[3] == '0') {
//                            // 0110 (6)
//                            return '6';
//                        }
//                        else {
//                            // 0111 (7)
//                            return '7';
//                        }
//                    }
//                }
//            }
//            else {
//                // 1---
//                if (nibble[1] == '0') {
//                    // 10--
//                    if (nibble[2] == '0') {
//                        // 100-
//                        if (nibble[3] == '0') {
//                            // 1000 (8)
//                            return '8';
//                        }
//                        else {
//                            // 1001 (9)
//                            return '9';
//                        }
//                    }
//                    else {
//                        if (nibble[3] == '0') {
//                            // 1010 (10)
//                            return 'A';
//                        }
//                        else {
//                            // 1011 (11)
//                            return 'B';
//                        }
//                    }
//                }
//                else {
//                    // 11--
//                    if (nibble[2] == '0') {
//                        // 110-
//                        if (nibble[3] == '0') {
//                            // 1100 (12)
//                            return 'C';
//                        }
//                        else {
//                            // 1101 (13)
//                            return 'D';
//                        }
//                    }
//                    else {
//                        // 111-
//                        if (nibble[3] == '0') {
//                            // 1110 (14)
//                            return 'E';
//                        }
//                        else {
//                            // 1111 (15)
//                            return 'F';
//                        }
//                    }
//                }
//            }
//        }
//
////        std::unordered_map<std::type_index, const char*> formats { };
////        std::shared_mutex formats_mutex;
////
////        void set_format(std::type_index type, const char* format) {
////            std::unique_lock<std::shared_mutex> lock(formats_mutex);
////            formats[type] = format;
////        }
////
////        void clear_format(std::type_index type) {
////            std::unique_lock<std::shared_mutex> lock(formats_mutex);
////            auto iter = formats.find(type);
////            if (iter != formats.end()) {
////                iter->second = nullptr;
////            }
////        }
////
////        inline const char* get_format(std::type_index type) {
////            std::shared_lock<std::shared_mutex> lock(formats_mutex);
////            auto iter = formats.find(type);
////            if (iter != formats.end()) {
////                return iter->second;
////            }
////            return nullptr;
////        }
//
//    }
//
//    [[nodiscard]] std::vector<std::string> split(std::string_view in, std::string_view delimiter) {
//        std::vector<std::string> components { };
//
//        std::size_t position;
//        do {
//            position = in.find(delimiter);
//            components.emplace_back(in.substr(0, position));
//            in = in.substr(position + delimiter.length());
//        }
//        while (position != std::string::npos);
//
//        return std::move(components);
//    }
//
//    [[nodiscard]] std::string_view trim(std::string_view in) {
//        if (in.empty()) {
//            return "";
//        }
//
//        std::size_t start = 0u;
//        while (std::isspace(in[start])) {
//            ++start;
//        }
//
//        std::size_t end = in.length() - 1u;
//        while (end != start && std::isspace(in[end])) {
//            --end;
//        }
//
//        return in.substr(start, end + 1);
//    }
//
//    template <typename T>
//    inline std::size_t fundamental_from_string(std::string_view in, T& out) {
//        // Leading whitespace is not ignored
//        std::string_view str = trim(in);
//
//        if (str.empty()) {
//            throw FormattedError("");
//        }
//
//        // Only a leading '-' is permitted at the beginning
//        if (str[0] == '+') {
//            str = str.substr(1);
//        }
//
//        // Leading base prefixes are not recognized
//        int base = 10;
//        bool has_base = false;
//
//        if (str.length() > 1) {
//            if (str[0] == '0') {
//                if (str[1] == 'x' || str[1] == 'X') {
//                    // Hexadecimal
//                    base = 16;
//                    has_base = true;
//                }
//                else if (str[1] == 'b' || str[1] == 'B') {
//                    // Binary
//                    base = 2;
//                    has_base = true;
//                }
//            }
//        }
//
//        if (has_base) {
//            str = str.substr(2);
//        }
//
//        const char* start = str.data();
//        const char* end = start + str.length();
//
//        std::from_chars_result result { };
//
//        if constexpr (is_integer_type<T>::value) {
//            result = std::from_chars(start, end, out, base);
//        }
//        else {
//            // std::format_chars::general supports both scientific and fixed representations
//            result = std::from_chars(start, end, out, std::chars_format::general);
//        }
//
//        const auto& [ptr, error_code] = result;
//
//        if (error_code == std::errc::invalid_argument) {
//            // failed to convert
//            logging::fatal("");
//            throw FormattedError("");
//        }
//
//        if (error_code == std::errc::result_out_of_range) {
//            logging::error("");
//            out = std::numeric_limits<T>::max();
//        }
//
//        // Return the number of characters processed
//        return ptr - start;
//    }
//
//    std::size_t from_string(std::string_view in, unsigned char& out) {
//        return fundamental_from_string(in, out);
//    }
//
//    std::size_t from_string(std::string_view in, short& out) {
//        return fundamental_from_string(in, out);
//    }
//
//    std::size_t from_string(std::string_view in, unsigned short& out) {
//        return fundamental_from_string(in, out);
//    }
//
//    std::size_t from_string(std::string_view in, int& out) {
//        return fundamental_from_string(in, out);
//    }
//
//    std::size_t from_string(std::string_view in, unsigned int& out) {
//        return fundamental_from_string(in, out);
//    }
//
//    std::size_t from_string(std::string_view in, long& out) {
//        return fundamental_from_string(in, out);
//    }
//
//    std::size_t from_string(std::string_view in, unsigned long& out) {
//        return fundamental_from_string(in, out);
//    }
//
//    std::size_t from_string(std::string_view in, long long int& out) {
//        return fundamental_from_string(in, out);
//    }
//
//    std::size_t from_string(std::string_view in, unsigned long long int& out) {
//        return fundamental_from_string(in, out);
//    }
//
//    std::size_t from_string(std::string_view in, float& out) {
//        return fundamental_from_string(in, out);
//    }
//
//    std::size_t from_string(std::string_view in, double& out) {
//        return fundamental_from_string(in, out);
//    }
//
//    std::size_t from_string(std::string_view in, long double& out) {
//        return fundamental_from_string(in, out);
//    }
//
//    FormatString::FormatString(const FormatString& fmt, std::source_location source_location) : m_format(fmt.m_format),
//                                                                                                m_source(source_location),
//                                                                                                m_identifiers(fmt.m_identifiers),
//                                                                                                m_specifications(fmt.m_specifications),
//                                                                                                m_placeholders(fmt.m_placeholders) {
//    }
//
//    FormatString::~FormatString() = default;
//
//    FormatString::operator std::string() const {
//        if (m_placeholders.empty()) {
//            return m_format;
//        }
//
//        Formatter<std::size_t> position_formatter { }; // This formatter is used to format positional placeholder positions, default-initialization is good enough
//        std::size_t capacity = m_format.length();
//
//        for (const Placeholder& placeholder : m_placeholders) {
//            const Identifier& identifier = m_identifiers[placeholder.identifier_index];
//            switch (identifier.type) {
//                case Identifier::Type::Auto:
//                    break;
//                case Identifier::Type::Position:
//                    // Can also use Formatter<T>::reserve for this, but this is evaluated at compile time, so it's faster
//                    capacity += detail::count_digits(identifier.position);
//                    break;
//                case Identifier::Type::Name:
//                    capacity += identifier.name.length();
//                    break;
//            }
//
//            // Add capacity for placeholder braces (one opening brace + one closing brace)
//            capacity += 2u;
//        }
//
//        std::string result = m_format;
//        result.reserve(capacity);
//
//        std::size_t placeholder_offset = 0u;
//        std::size_t write_position;
//        std::size_t length;
//
//        for (const Placeholder& placeholder : m_placeholders) {
//            write_position = placeholder.position + placeholder_offset;
//
//            result.insert(write_position++, "{");
//            const Identifier& identifier = m_identifiers[placeholder.identifier_index];
//            switch (identifier.type) {
//                case Identifier::Type::Position: {
//                    length = detail::count_digits(placeholder.position);
//                    FormattingContext context { length, &result[write_position] };
//                    position_formatter.format_to(identifier.position, context);
//                    break;
//                }
//                case Identifier::Type::Name:
//                    length = identifier.name.length();
//                    result.insert(write_position, identifier.name);
//                    break;
//                default:
//                    // Auto-numbered placeholders are empty
//                    length = 0u;
//                    break;
//            }
//            write_position += length;
//            result.insert(write_position++, "}");
//
//            placeholder_offset += length + 2u;
//        }
//
//        return std::move(result);
//    }
//
//    std::string FormatString::string() const {
//        return *this;
//    }
//
//    std::source_location FormatString::source() const {
//        return m_source;
//    }
//
//    std::size_t FormatString::get_placeholder_count() const {
//        return get_positional_placeholder_count() + get_named_placeholder_count();
//    }
//
//    std::size_t FormatString::get_positional_placeholder_count() const {
//        std::size_t highest = 0u;
//
//        // The number of positional placeholders depends on the highest placeholder value encountered in the format string
//        for (const Placeholder& placeholder : m_placeholders) {
//            const Identifier& identifier = m_identifiers[placeholder.identifier_index];
//            if (identifier.type == Identifier::Type::Position) {
//                // Positional placeholders indices start at 0
//                highest = std::max(identifier.position + 1, highest);
//            }
//        }
//
//        return highest;
//    }
//
//    std::size_t FormatString::get_named_placeholder_count() const {
//        std::size_t count = 0u;
//
//        for (const Placeholder& placeholder : m_placeholders) {
//            if (m_identifiers[placeholder.identifier_index].type == Identifier::Type::Name) {
//                ++count;
//            }
//        }
//
//        return count;
//    }
//
//    void FormatString::parse() {
//        if (m_format.empty()) {
//            return;
//        }
//
//        std::size_t length = m_format.length();
//        std::size_t placeholder_start;
//
//        std::size_t i = 0u;
//
//        while (i < length) {
//            if (m_format[i] == '{') {
//                if (i + 1u == length) {
//                    throw FormattedError("unterminated '{' at index {}", i);
//                }
//                else if (m_format[i + 1u] == '{') {
//                    // Escaped opening brace '{{', keep only one
//                    // Remove the braces from the format string as the input string is only parsed once
//                    std::memcpy(&m_format[i], &m_format[i + 1u], m_format.length() - i);
//
//                    m_format.resize(length - 1u);
//                    length = m_format.length();
//
//                    ++i;
//                }
//                else {
//                    // Skip placeholder opening brace '{'
//                    placeholder_start = i++;
//
//                    Identifier identifier { }; // Auto-numbered by default
//                    if (std::isdigit(m_format[i])) {
//                        // Positional placeholder
//                        ++i;
//
//                        // Positional placeholders must only contain numbers
//                        while (std::isdigit(m_format[i])) {
//                            ++i;
//                        }
//
//                        std::size_t position;
//                        from_string(m_format.substr(placeholder_start + 1u, i - (placeholder_start + 1u)), position);
//
//                        identifier = Identifier(position);
//                    }
//                    // Named placeholders follow the same identifier rules as standard C/C++ identifiers
//                    else if (std::isalpha(m_format[i]) || (m_format[i] == '_')) {
//                        // Named placeholder
//                        ++i;
//
//                        while (std::isalpha(m_format[i]) || std::isdigit(m_format[i]) || (m_format[i] == '_')) {
//                            ++i;
//                        }
//
//                        identifier = Identifier(std::string(m_format.substr(placeholder_start + 1u, i - (placeholder_start + 1u))));
//                    }
//
//                    if (m_format[i] != ':' && m_format[i] != '}') {
//                        throw FormattedError("invalid character '{}' at index {} - expecting formatting separator ':' or placeholder terminator '}'", m_format[i], i);
//                    }
//
//                    Specification spec { };
//                    if (m_format[i] == ':') {
//                        // Skip format specification separator ':'
//                        ++i;
//
//                        // Parse custom formatting (consumes placeholder closing brace '}')
//                        ParseResponse<FormatString> r = parse_specification(m_format.substr(i), spec);
//                        if (!r.ok()) {
//                            // Throw exception with the error position relative to the start of the string
//                            throw FormattedError(r.error(), NamedArgument("index", r.offset() + i));
//                        }
//
//                        i += r.offset();
//                    }
//                    else {
//                        // Skip placeholder closing brace '}'
//                        ++i;
//                    }
//
//                    register_placeholder(identifier, spec, placeholder_start);
//
//                    std::memcpy(&m_format[placeholder_start], &m_format[i], length - i);
//
//                    m_format.resize(length - (i - placeholder_start));
//                    length = m_format.length();
//
//                    // Reset parsing position to the start of the placeholder
//                    i = placeholder_start;
//                }
//            }
//            else if (m_format[i] == '}') {
//                if ((i + 1u != length) && m_format[i + 1u] == '}') {
//                    // Escaped closing brace '}}', keep only one
//                    std::memcpy(&m_format[i], &m_format[i + 1u], m_format.length() - i);
//
//                    m_format.resize(length - 1u);
//                    length = m_format.length();
//
//                    ++i;
//                }
//                else {
//                    throw FormattedError("invalid '}' at index {} - closing brace literals must be escaped as '}}'", i);
//                }
//            }
//            else {
//                ++i;
//            }
//        }
//
//        // Issue a warning if not all positional arguments are used
//        for (std::size_t position = 0u; position < get_positional_placeholder_count(); ++position) {
//            bool found = false;
//            for (const Placeholder& placeholder : m_placeholders) {
//                const Identifier& identifier = m_identifiers[placeholder.identifier_index];
//                if (identifier.type == Identifier::Type::Position && identifier.position == position) {
//                    found = true;
//                    break;
//                }
//            }
//            if (!found) {
//                logging::warning("value for positional placeholder {} is never referenced in the format string", i);
//            }
//        }
//    }
//
//
//
//    ParseResponse<FormatString> FormatString::parse_specification(std::string_view in, FormatString::Specification& spec, bool nested) {
//        // Note: function assumes input string does not contain a leading formatting group separator ':'
//        std::size_t length = in.length();
//        std::size_t group = 0u;
//
//        char terminator = nested ? '|' : '}';
//
//        std::size_t i = 0u;
//
//        while (i < length) {
//            if (in[i] == terminator) {
//                break;
//            }
//
//            if (in[i] == ':') {
//                // Encountered formatting group separator
//                // {identifier:representation=[...]}
//                //            ^
//
//                // Note: empty groups are supported and are treated as empty format specifier lists
//                ++group;
//
//                // Skip formatting group separator ':'
//                ++i;
//                continue;
//            }
//            else if (in[i] == '|') {
//                // Encountered nested formatting specification separator
//                // {identifier:|representation=[...]:justification=[...]|}
//                //             ^
//
//                // Skip formatting specification separator '|'
//                ++i;
//
//                ParseResponse<FormatString> r = parse_specification(in.substr(i), spec[group], true);
//                if (!r.ok()) {
//                    return ParseResponse<FormatString>::NOT_OK(i, r.error());
//                }
//
//                i += r.offset();
//            }
//            else {
//                // Parse format specifiers
//                while (true) {
//                    ParseResult <FormatString::Specification::Specifier, FormatString> r = parse_specifier(in.substr(i));
//
//                    if (!r.ok()) {
//                        return ParseResponse<FormatString>::NOT_OK(r.offset() + i, r.error());
//                    }
//
//                    FormatString::Specification::Specifier& specifier = r.result();
//                    spec[group][specifier.name] = std::move(specifier.value);
//
//                    i += r.offset();
//
//                    if (in[i] != ',') {
//                        break;
//                    }
//
//                    // Skip format specifier separator ','
//                    ++i;
//                }
//            }
//        }
//
//        return ParseResponse<FormatString>::OK(i);
//    }
//
//    void FormatString::register_placeholder(const Identifier& identifier, const Specification& spec, std::size_t position) {
//        if (!m_placeholders.empty()) {
//            // Verify format string homogeneity
//            // The identifier of the first placeholder determines the type of format string this is
//            Identifier::Type type = m_identifiers[m_placeholders[0].identifier_index].type;
//
//            if (type == Identifier::Type::Auto) {
//                if (identifier.type == Identifier::Type::Position) {
//                    throw FormattedError("format string placeholders must be homogeneous - positional placeholder {} at index {} cannot be mixed with auto-numbered placeholders (first encountered at index {})", identifier.position, position, m_placeholders[0].position);
//                }
//                else if (identifier.type == Identifier::Type::Name) {
//                    throw FormattedError("format string placeholders must be homogeneous - named placeholder '{}' at index {} cannot be mixed with auto-numbered placeholders (first encountered at index {})", identifier.name, position, m_placeholders[0].position);
//                }
//            }
//            else {
//                // Format string mixes a named / positional placeholder with an auto-numbered placeholder, which is not allowed
//                if (identifier.type == Identifier::Type::Auto) {
//                    // Format string mixes an auto-numbered placeholder with a named / positional placeholder, which is not allowed
//                    throw FormattedError("format string placeholders must be homogeneous - auto-numbered placeholder at index {} cannot be mixed with positional/named placeholders (first encountered at index {})", position, m_placeholders[0].position);
//                }
//            }
//        }
//
//        std::size_t num_identifiers = m_identifiers.size();
//        std::size_t identifier_index = num_identifiers;
//
//        for (std::size_t i = 0u; i < num_identifiers; ++i) {
//            if (m_identifiers[i] == identifier) {
//                identifier_index = i;
//                break;
//            }
//        }
//
//        if (identifier_index == num_identifiers) {
//            m_identifiers.emplace_back(identifier);
//        }
//
//        // Find (or register) new format specification
//        std::size_t num_format_specifications = m_specifications.size();
//        std::size_t specification_index = num_format_specifications;
//
//        for (std::size_t i = 0u; i < num_format_specifications; ++i) {
//            if (m_specifications[i] == spec) {
//                specification_index = i;
//                break;
//            }
//        }
//
//        if (specification_index == num_format_specifications) {
//            // Register new identifier
//            m_specifications.emplace_back(spec);
//        }
//
//        // Placeholders are automatically sorted by their position in the format string
//        m_placeholders.emplace_back(identifier_index, specification_index, position, false);
//    }
//
//    FormatString::Identifier::Identifier() : type(Type::Auto),
//                                             position(std::numeric_limits<std::size_t>::max()),
//                                             name() {
//    }
//
//    FormatString::Identifier::Identifier(std::size_t position) : type(Type::Position),
//                                                                 position(position),
//                                                                 name() {
//    }
//
//    FormatString::Identifier::Identifier(std::string name) : type(Type::Name),
//                                                             position(std::numeric_limits<std::size_t>::max()),
//                                                             name(std::move(name)) {
//    }
//
//    FormatString::Identifier::~Identifier() = default;
//
//    bool FormatString::Identifier::operator==(const Identifier& other) const {
//        bool matching_types = type == other.type;
//
//        if (type == Type::Auto) {
//            return matching_types;
//        }
//        else if (type == Type::Position) {
//            return matching_types && position == other.position;
//        }
//        else {
//            return matching_types && name == other.name;
//        }
//    }
//
//    FormatString::Specification::Specification() : m_spec(),
//                                                   m_type(Type::SpecifierList) {
//    }
//
//    FormatString::Specification::Specification(SpecifierList&& specifiers) : m_spec(std::move(specifiers)),
//                                                                             m_type(Type::SpecifierList) {
//    }
//
//    FormatString::Specification::~Specification()= default;
//
//    bool FormatString::Specification::operator==(const FormatString::Specification& other) const {
//        if (m_type != other.m_type) {
//            return false;
//        }
//
//        std::size_t s = size();
//
//        if (s != other.size()) {
//            return false;
//        }
//
//        switch (m_type) {
//            case Type::FormattingGroupList: {
//                const FormattingGroupList& groups = std::get<FormattingGroupList>(m_spec);
//                const FormattingGroupList& other_groups = std::get<FormattingGroupList>(other.m_spec);
//
//                for (std::size_t i = 0u; i < s; ++i) {
//                    if (!groups[i] && !other_groups[i]) {
//                        // Both are nullptr (equal)
//                        continue;
//                    }
//
//                    if (!groups[i] && other_groups[i] || groups[i] && !other_groups[i]) {
//                        return false;
//                    }
//
//                    // Both are valid pointers
//                    if (*(groups[i]) != *(other_groups[i])) {
//                        return false;
//                    }
//                }
//
//                break;
//            }
//            case Type::SpecifierList: {
//                const SpecifierList& specifiers = std::get<SpecifierList>(m_spec);
//                const SpecifierList& other_specifiers = std::get<SpecifierList>(other.m_spec);
//
//                for (std::size_t i = 0u; i < s; ++i) {
//                    if (specifiers[i] != other_specifiers[i]) {
//                        return false;
//                    }
//                }
//
//                break;
//            }
//        }
//
//        return true;
//    }
//
//    bool FormatString::Specification::operator!=(const FormatString::Specification& other) const {
//        return !(*this == other);
//    }
//
//    FormatString::Specification::Type FormatString::Specification::type() const {
//        return m_type;
//    }
//
//    std::size_t FormatString::Specification::size() const {
//        // All internal types (std::vector) support the size operation
//        static const auto visitor = []<typename T>(const T& data) -> std::size_t {
//            return data.size();
//        };
//        return std::visit(visitor, m_spec);
//    }
//
//    bool FormatString::Specification::empty() const {
//        // All internal types (std::vector) support the empty operation
//        static const auto visitor = []<typename T>(const T& data) -> bool {
//            return data.empty();
//        };
//        return std::visit(visitor, m_spec);
//    }
//
//    void FormatString::Specification::set_specifier(std::string_view key, std::string value) {
//        if (m_type == Type::FormattingGroupList) {
//            throw FormattedError("bad and/or ambiguous format specification access - specification contains nested formatting group(s) and cannot be accessed by specifier (key: '{}')", key);
//        }
//
//        SpecifierList& specifiers = std::get<SpecifierList>(m_spec);
//        for (Specifier& specifier : specifiers) {
//            if (icasecmp(specifier.name, key)) {
//                specifier.value = std::move(value);
//                return;
//            }
//        }
//
//        // Specifier not found, create a new entry
//        specifiers.emplace_back(std::string(key), std::move(value));
//    }
//
//    FormatString::Specification::Specifier& FormatString::Specification::operator[](std::string_view key) {
//        return get_specifier(key);
//    }
//
//    const FormatString::Specification::Specifier& FormatString::Specification::operator[](std::string_view key) const {
//        return get_specifier(key);
//    }
//
//    FormatString::Specification::Specifier& FormatString::Specification::get_specifier(std::string_view key) {
//        if (m_type == Type::FormattingGroupList) {
//            // While formatting groups initialized to specifier lists can be converted to formatting group lists, the opposite of this operation is purposefully not supported
//            // Initializing a nested formatting specification to a formatting group can only be done through an intentional operation
//            throw FormattedError("bad and/or ambiguous format specification access - specification contains nested formatting group(s) and cannot be accessed by specifier (key: '{}')", key);
//        }
//
//        SpecifierList& specifiers = std::get<SpecifierList>(m_spec);
//        for (Specifier& specifier : specifiers) {
//            if (icasecmp(specifier.name, key)) {
//                return specifier;
//            }
//        }
//
//        // Specifier was not found, create a new entry
//        return specifiers.emplace_back(std::string(key), "");
//    }
//
//    const FormatString::Specification::Specifier& FormatString::Specification::get_specifier(std::string_view key) const {
//        if (m_type == Type::FormattingGroupList) {
//            throw FormattedError("bad and/or ambiguous format specification access - specification contains nested formatting group(s) and cannot be accessed by specifier (key: '{}')", key);
//        }
//
//        for (const Specifier& specifier : std::get<SpecifierList>(m_spec)) {
//            if (icasecmp(specifier.name, key)) {
//                return specifier;
//            }
//        }
//
//        // Specifier not found
//        throw FormattedError("bad format specification access - specifier with name '{}' not found", key);
//    }
//
//    FormatString::Specification& FormatString::Specification::operator[](std::size_t index) {
//        return get_formatting_group(index);
//    }
//
//    const FormatString::Specification& FormatString::Specification::operator[](std::size_t index) const {
//        return get_formatting_group(index);
//    }
//
//    FormatString::Specification& FormatString::Specification::get_formatting_group(std::size_t index) {
//        // Try not to incur extra memory / performance overhead when the formatting specification only contains a specifier list
//        // Use a specifier list in line in place of a formatting group list there is only one active group
//        if (m_type == Type::SpecifierList) {
//            if (index == 0u) {
//                // Continue treating this as a specifier list and not a formatting group list
//                return *this;
//            }
//            else {
//                // When an additional group is requested, convert internal structure to a formatting group list
//                // This first requires the conversion of this (specifier list) to the first formatting group
//                m_spec = FormattingGroupList { new Specification(std::move(std::get<SpecifierList>(m_spec))) };
//                m_type = Type::FormattingGroupList;
//            }
//        }
//
//        FormattingGroupList& groups = std::get<FormattingGroupList>(m_spec);
//        while (index >= groups.size()) {
//            // Allow a sparse vector of formatting groups to save on memory use
//            groups.emplace_back(nullptr);
//        }
//
//        if (!groups[index]) {
//            groups[index] = new Specification();
//        }
//        return *groups[index];
//    }
//
//    const FormatString::Specification& FormatString::Specification::get_formatting_group(std::size_t index) const {
//        if (m_type == Type::SpecifierList) {
//            throw FormattedError("bad format specification access - formatting group {} contains a mapping of specifier name/value pairs and cannot be accessed by index", index);
//        }
//
//        const FormattingGroupList& groups = std::get<FormattingGroupList>(m_spec);
//        if (index < groups.size()) {
//            return *groups[index];
//        }
//
//        throw FormattedError("bad format specification access - formatting group {} does not exist (out of bounds)", index);
//    }
//
//    bool FormatString::Specification::has_group(std::size_t index) const {
//        if (std::holds_alternative<SpecifierList>(m_spec)) {
//            // TODO: calling has_group on a specifier list makes no sense, throw exception?
//            return false;
//        }
//
//        return index < std::get<FormattingGroupList>(m_spec).size();
//    }
//
//    bool FormatString::Specification::has_specifier(std::string_view key) const {
//        if (std::holds_alternative<FormattingGroupList>(m_spec)) {
//            // TODO: calling has_group on a formatting group list makes no sense, throw exception?
//            return false;
//        }
//
//        for (const Specifier& specifier : std::get<SpecifierList>(m_spec)) {
//            if (icasecmp(specifier.name, key)) {
//                return true;
//            }
//        }
//
//        return false;
//    }
//
//    FormatString::Specification::Specifier::Specifier(std::string name, std::string value) : name(std::move(name)),
//                                                                                             value(std::move(value)) {
//    }
//
//    FormatString::Specification::Specifier::~Specifier() = default;
//
//    FormatString::Specification::Specifier& FormatString::Specification::Specifier::operator=(std::string other) {
//        value = std::move(other);
//        return *this;
//    }
//
//    bool FormatString::Specification::Specifier::operator==(const FormatString::Specification::Specifier& other) const {
//        return icasecmp(name, other.name) && icasecmp(value, other.value);
//    }
//
//    bool FormatString::Specification::Specifier::operator!=(const FormatString::Specification::Specifier& other) const {
//        return !(*this == other);
//    }
//
//    std::ostream& operator<<(std::ostream& stream, const FormatString& fmt) {
//        stream << fmt.string();
//        return stream;
//    }
//
//    FormattingContext::FormattingContext(std::size_t length, char* src) : m_buffer(nullptr),
//                                                                          m_owner(src == nullptr),
//                                                                          m_length(length) {
//        if (!length) {
//            throw std::runtime_error("buffer of size 0 is invalid");
//        }
//
//        m_buffer = src ? src : new char[length];
//    }
//
//    FormattingContext::~FormattingContext() {
//        if (m_owner) {
//            delete[] m_buffer;
//        }
//    }
//
//    char& FormattingContext::operator[](std::size_t index) {
//        if (index >= m_length) {
//            throw std::out_of_range(utils::format("FormattingContext::operator[]: index {} exceeds the length of the underlying buffer ({})", index, m_length));
//        }
//        return m_buffer[index];
//    }
//
//    char& FormattingContext::at(std::size_t index) {
//        if (index >= m_length) {
//            throw std::out_of_range(utils::format("FormattingContext::at: index {} exceeds the the length of the underlying buffer ({})", index, m_length));
//        }
//        return m_buffer[index];
//    }
//
//    void FormattingContext::insert(std::size_t offset, const char* src, std::size_t length) {
//        if (!length) {
//            length = strlen(src);
//        }
//
//        if (offset + length > m_length) {
//            throw std::out_of_range(utils::format("FormattingContext::insert: inserting {} character(s) at offset {} exceeds the length of the underlying buffer ({})", length, offset, m_length));
//        }
//
//        std::memcpy(m_buffer + offset, src, length);
//    }
//
//    void FormattingContext::insert(std::size_t offset, char c, std::size_t count) {
//        if (offset + count > m_length) {
//            throw std::out_of_range(utils::format("FormattingContext::insert: inserting {} character(s) at offset {} exceeds the length of the underlying buffer ({})", count, offset, m_length));
//        }
//        for (std::size_t i = 0u; i < count; ++i) {
//            m_buffer[offset + i] = c;
//        }
//    }
//
//    FormattingContext FormattingContext::slice(std::size_t offset, std::size_t length) {
//        bool valid = true;
//
//        if (length == std::string::npos) {
//            if (offset >= m_length) {
//                valid = false;
//            }
//            else {
//                // Read all characters up until the end of the buffer
//                length = m_length - offset;
//            }
//        }
//        else if (offset + length >= m_length) {
//            valid = false;
//        }
//
//        if (!valid) {
//            throw std::out_of_range(utils::format("FormattingContext::slice: a subcontext of length {} at offset {} exceeds the length of the underlying buffer ({})", length, offset, m_length));
//        }
//
//        // Specifying std::string::npos returns a slice until the end of the buffer
//        return { length == std::string::npos ? m_length - offset : length, &m_buffer[offset] };
//    }
//
//    std::string FormattingContext::string() const {
//        return std::move(std::string(m_buffer, m_length));
//    }
//
//    const char* FormattingContext::data() const {
//        return m_buffer;
//    }
//
//    std::size_t FormattingContext::length() const {
//        return m_length;
//    }
//
//    std::size_t FormattingContext::size() const {
//        return m_length;
//    }
//
////    Formatter<std::source_location>::Formatter() : m_line_formatter(),
////                                                   m_filename_formatter() {
////    }
////
////    Formatter<std::source_location>::~Formatter() = default;
////
////    void Formatter<std::source_location>::parse(const FormatString::Specification& spec) {
////        switch (spec.type()) {
////            case FormatString::Specification::Type::FormattingGroupList:
////                if (spec.has_group(0)) {
////                    m_line_formatter.parse(spec.get_formatting_group(0));
////                }
////                if (spec.has_group(1)) {
////                    m_filename_formatter.parse(spec.get_formatting_group(1));
////                }
////                break;
////            case FormatString::Specification::Type::SpecifierList:
////                logging::warning(""); // TODO:
////                break;
////        }
////    }
////
////    std::string Formatter<std::source_location>::format(const std::source_location& value) const {
////        unsigned line = value.line();
////        std::size_t formatted_line_number_length = m_line_formatter.reserve(line);
////
////        const char* filename = value.file_name();
////        std::size_t formatted_filename_length = m_filename_formatter.reserve(filename);
////
////        // Account for joining ':' in format string 'filename:line'
////        std::size_t capacity = formatted_line_number_length + 1u + formatted_filename_length;
////
////        std::string result;
////        result.resize(capacity);
////
////        FormattingContext context { capacity, result.data() };
////        m_filename_formatter.format_to(filename, context.slice(0, formatted_filename_length));
////        context[formatted_filename_length] = ':';
////        m_line_formatter.format_to(line, context.slice(formatted_filename_length + 1));
////
////        return std::move(result);
////    }
////
////    std::size_t Formatter<std::source_location>::reserve(const std::source_location& value) const {
////        return m_line_formatter.reserve(value.line()) + 1u + m_filename_formatter.reserve(value.file_name());
////    }
////
////    void Formatter<std::source_location>::format_to(const std::source_location& value, FormattingContext context) const {
////        unsigned line = value.line();
////        std::size_t formatted_line_number_length = m_line_formatter.reserve(line);
////
////        const char* filename = value.file_name();
////        std::size_t formatted_filename_length = m_filename_formatter.reserve(filename);
////
////        m_filename_formatter.format_to(filename, context.slice(0, formatted_filename_length));
////        context[formatted_filename_length] = ':';
////        m_line_formatter.format_to(line, context.slice(formatted_filename_length + 1));
////    }
//
//}
//

#include "utils/string.hpp"
#include "utils/exceptions.hpp"
#include "utils/logging.hpp"

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
            std::size_t offset = 0u;

            if (std::isdigit(in[offset])) {
                ++offset;

                // Positional placeholders must only contain numbers
                while (std::isdigit(in[offset])) {
                    ++offset;
                }

                std::size_t position;
                from_string(in.substr(0u, offset), position);

                out = Identifier(position);
            }
            else if (std::isalpha(in[offset]) || in[offset] == '_') {
                ++offset;

                // Named placeholders follow the same identifier rules as standard C/C++ identifiers
                while (std::isalnum(in[offset]) || in[offset] == '_') {
                    ++offset;
                }

                out = Identifier(in.substr(0u, offset));
            }

            return offset;
        }

        std::size_t parse_specifier(std::string_view in, Specifier& out) {
            std::size_t length = in.length();
            std::size_t i = 0u;

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
                    if (i + 1 == length || in[i + 1u] != '[') {
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
            for (std::size_t i = 0u; i < length; ++i) {
                if (i + 1 < length && ((value[i] == '[' && value[i + 1u] == '[') || (value[i] == ']' && value[i + 1u] == ']'))) {
                    // Push back only one brace character
                    ++i;
                }
                out.push_back(value[i]);
            }
        }

        std::size_t parse_format_spec(std::string_view in, FormatSpec& out, bool nested) {
            // Note: function assumes input string does not contain a leading formatting group separator ':'
            std::size_t length = in.length();
            std::size_t group = 0u;
            std::size_t i = 0u;

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
                        else if (in[i] == terminator) {
                            // End of format spec
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
                            return 'A';
                        }
                        else {
                            // 1011 (11)
                            return 'B';
                        }
                    }
                }
                else {
                    // 11--
                    if (nibble[2] == '0') {
                        // 110-
                        if (nibble[3] == '0') {
                            // 1100 (12)
                            return 'C';
                        }
                        else {
                            // 1101 (13)
                            return 'D';
                        }
                    }
                    else {
                        // 111-
                        if (nibble[3] == '0') {
                            // 1110 (14)
                            return 'E';
                        }
                        else {
                            // 1111 (15)
                            return 'F';
                        }
                    }
                }
            }
        }

    }

    [[nodiscard]] std::vector<std::string> split(std::string_view in, std::string_view delimiter) {
        std::vector<std::string> components { };

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

        std::size_t start = 0u;
        while (std::isspace(in[start])) {
            ++start;
        }

        std::size_t end = in.length() - 1u;
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
            logging::fatal("");
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
    
    FormatSpec::FormatSpec() : m_spec(SpecifierList()),
                               m_type(Type::SpecifierList) {
    }

    FormatSpec::FormatSpec(SpecifierList&& specifiers) : m_spec(std::move(specifiers)),
                                                         m_type(Type::SpecifierList) {
    }

    FormatSpec::~FormatSpec() {
        if (m_type == Type::FormattingGroupList) {
            FormattingGroupList& groups = std::get<FormattingGroupList>(m_spec);
            for (FormatSpec* spec : groups) {
                delete spec;
            }
        }
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
        if (*this != other) {
            m_type = other.m_type;
            if (other.m_type == Type::SpecifierList) {
                // Vector of specifiers can be copied directly
                m_spec = other.m_spec;
            }
            else {
                FormattingGroupList groups = FormattingGroupList();
                for (const FormatSpec* spec : std::get<FormattingGroupList>(other.m_spec)) {
                    groups.emplace_back(new FormatSpec(*spec));
                }
                m_spec = std::move(groups);
            }
        }
        
        return *this;
    }
    
    FormatSpec::Type FormatSpec::type() const {
        return m_type;
    }

    std::size_t FormatSpec::size() const {
        // All internal types (std::vector) support the size operation
        static const auto visitor = []<typename T>(const T& data) -> std::size_t {
            return data.size();
        };
        return std::visit(visitor, m_spec);
    }

    bool FormatSpec::empty() const {
        // All internal types (std::vector) support the empty operation
        static const auto visitor = []<typename T>(const T& data) -> bool {
            return data.empty();
        };
        return std::visit(visitor, m_spec);
    }

    bool FormatSpec::operator!=(const FormatSpec& other) const {
        return !(*this == other);
    }
    
    bool FormatSpec::operator==(const FormatSpec& other) const {
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

    void FormatSpec::set_specifier(std::string_view key, std::string value) {
        if (m_type == Type::FormattingGroupList) {
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
        if (m_type == Type::FormattingGroupList) {
            // While formatting groups initialized to specifier lists can be converted to formatting group lists, the opposite of this operation is purposefully not supported
            // Initializing a nested formatting specification to a formatting group can only be done through an intentional operation
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
                // Continue treating this as a specifier list and not a formatting group list
                return *this;
            }
            else {
                // When an additional group is requested, convert internal structure to a formatting group list
                // This first requires the conversion of this (specifier list) to the first formatting group
                m_spec = FormattingGroupList { new FormatSpec(std::move(std::get<SpecifierList>(m_spec))) };
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
        if (std::holds_alternative<SpecifierList>(m_spec)) {
            // TODO: calling has_group on a specifier list makes no sense, throw exception?
            return false;
        }

        return index < std::get<FormattingGroupList>(m_spec).size();
    }

    bool FormatSpec::has_specifier(std::string_view key) const {
        if (std::holds_alternative<FormattingGroupList>(m_spec)) {
            // TODO: calling has_group on a formatting group list makes no sense, throw exception?
            return false;
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
    
    FormatString::FormatString(const char* format, std::source_location source) : format(format),
                                                                                  source(source) {
    }

    FormatString::FormatString(std::string_view format, std::source_location source) : format(format),
                                                                                       source(source) {
    }
    
    FormatString::~FormatString() = default;
    
    Formatter<char>::Formatter() : justification(Justification::Left),
                                   width(0u),
                                   fill_character(' ') {
    }
    
    Formatter<char>::~Formatter() {
    }
    
    void Formatter<char>::parse(const utils::FormatSpec& spec) {
        if (spec.type() == FormatSpec::Type::FormattingGroupList) {
            throw std::runtime_error("format spec must be a list of specifiers");
        }

        if (spec.has_specifier("justification", "justify", "alignment", "align")) {
            std::string_view value = trim(spec.get_specifier("justification", "justify", "alignment", "align"));
            if (icasecmp(value, "left")) {
                justification = Justification::Left;
            }
            else if (icasecmp(value, "right")) {
                justification = Justification::Right;
            }
            else if (icasecmp(value, "center")) {
                justification = Justification::Center;
            }
            else {
                logging::warning("ignoring unknown justification specifier value: '{}' - expecting one of: left, right, or center (case-insensitive)", value);
            }
        }

        if (spec.has_specifier("width")) {
            std::string_view value = trim(spec.get_specifier("width"));

            unsigned _width;
            std::size_t num_characters_read = from_string(value, _width);

            if (num_characters_read < value.length()) {
                logging::warning("ignoring invalid width specifier value: '{}' - specifier value must be an integer", value);
            }
            else {
                width = _width;
            }
        }

        if (spec.has_specifier("fill", "fill_character", "fillcharacter")) {
            std::string_view value = trim(spec.get_specifier("fill", "fill_character", "fillcharacter"));
            if (value.length() > 1u) {
                logging::warning("ignoring invalid fill character specifier value: '{}' - specifier value must be a single character", value);
            }
            else {
                fill_character = value[0];
            }
        }
    }
    
    std::string Formatter<char>::format(char c) const {
        std::size_t capacity = std::max(1ull, width);
        std::string result(capacity, fill_character);

        switch (justification) {
            case Justification::Left:
                result[0] = c;
                break;
            case Justification::Right:
                result[capacity - 1] = c;
                break;
            case Justification::Center:
                result[(capacity - 1) / 2] = c;
                break;
        }
        
        return std::move(result);
    }
    
    Formatter<const char*>::Formatter() : justification(Justification::Left),
                                          width(0u),
                                          fill_character(' ') {
    }
    
    Formatter<const char*>::~Formatter() {
    }
    
    void Formatter<const char*>::parse(const utils::FormatSpec& spec) {
        if (spec.type() == FormatSpec::Type::FormattingGroupList) {
            throw std::runtime_error("format spec must be a list of specifiers");
        }

        if (spec.has_specifier("justification", "justify", "alignment", "align")) {
            std::string_view value = trim(spec.get_specifier("justification", "justify", "alignment", "align"));
            if (icasecmp(value, "left")) {
                justification = Justification::Left;
            }
            else if (icasecmp(value, "right")) {
                justification = Justification::Right;
            }
            else if (icasecmp(value, "center")) {
                justification = Justification::Center;
            }
            else {
                logging::warning("ignoring unknown justification specifier value: '{}' - expecting one of: left, right, or center (case-insensitive)", value);
            }
        }

        if (spec.has_specifier("width")) {
            std::string_view value = trim(spec.get_specifier("width"));

            unsigned _width;
            std::size_t num_characters_read = from_string(value, _width);

            if (num_characters_read < value.length()) {
                logging::warning("ignoring invalid width specifier value: '{}' - specifier value must be an integer", value);
            }
            else {
                width = _width;
            }
        }

        if (spec.has_specifier("fill", "fill_character", "fillcharacter")) {
            std::string_view value = trim(spec.get_specifier("fill", "fill_character", "fillcharacter"));
            if (value.length() > 1u) {
                logging::warning("ignoring invalid fill character specifier value: '{}' - specifier value must be a single character", value);
            }
            else {
                fill_character = value[0];
            }
        }
    }
    
    std::string Formatter<const char*>::format(const char* value) const {
        return std::move(format(value, std::strlen(value)));
    }
    
    std::string Formatter<const char*>::format(const char* value, std::size_t length) const {
        std::size_t capacity = std::max(length, (std::size_t) width);
        std::string result(capacity, fill_character);

        std::size_t write_position;
        switch (justification) {
            case Justification::Left:
                write_position = 0u;
                break;
            case Justification::Right:
                write_position = capacity - length;
                break;
            case Justification::Center:
                write_position = (capacity - length) / 2;
                break;
        }
        
        result.replace(write_position, length, value, 0, length);
        return std::move(result);
    }
    
    std::string Formatter<std::string_view>::format(std::string_view value) const {
        return Formatter<const char*>::format(value.data(), value.length());
    }
    
    std::string Formatter<std::string>::format(const std::string& value) const {
        return Formatter<const char*>::format(value.c_str(), value.length());
    }
    
    Formatter<std::source_location>::Formatter() : m_file_formatter(),
                                                   m_line_formatter() {
    }

    Formatter<std::source_location>::~Formatter() = default;

    void Formatter<std::source_location>::parse(const utils::FormatSpec& spec) {
    }

    std::string Formatter<std::source_location>::format(const std::source_location& value) const {
        std::string result;
        
        result += m_file_formatter.format(value.file_name()) + ":" + m_line_formatter.format(value.line());
        
        return std::move(result);
    }
    
}
