
#include "utils/string.hpp"
#include "utils/result.hpp"
#include "utils/assert.hpp"

#include <limits>
#include <charconv>

namespace utils {
    namespace internal {
        
        [[nodiscard]] std::string to_string(FormatString::Identifier::Type type) {
            switch (type) {
                case FormatString::Identifier::Type::None:
                    return "auto-numbered";
                case FormatString::Identifier::Type::Position:
                    return "positional";
                case FormatString::Identifier::Type::Name:
                    return "named";
                default:
                    ASSERT(false, "unknown identifier type ({})", static_cast<std::underlying_type<FormatString::Identifier::Type>::type>(type));
                    return ""; // Silence compiler warnings.
            }
        }
        
        char to_specifier(Formatting::Justification justification) {
            switch (justification) {
                case Formatting::Justification::Right:
                    return '>';
                case Formatting::Justification::Left:
                    return '<';
                case Formatting::Justification::Center:
                    return '^';
                default:
                    ASSERT(false, "unknown justification value ({})", static_cast<std::underlying_type<Formatting::Justification>::type>(justification));
                    return '\0'; // Silence compiler warnings.
            }
        }
        
        char to_specifier(Formatting::Representation representation) {
            switch (representation) {
                case Formatting::Representation::Decimal:
                    return 'd';
                case Formatting::Representation::Scientific:
                    return 'e';
                case Formatting::Representation::Fixed:
                    return 'f';
                case Formatting::Representation::Binary:
                    return 'b';
                case Formatting::Representation::Hexadecimal:
                    return 'x';
                default:
                    ASSERT(false, "unknown representation value ({})", static_cast<std::underlying_type<Formatting::Representation>::type>(representation));
                    return '\0'; // Silence compiler warnings.
            }
        }
        
        std::string to_string(Formatting::Representation representation) {
            switch (representation) {
                case Formatting::Representation::Decimal:
                    return "decimal";
                case Formatting::Representation::Binary:
                    return "binary";
                case Formatting::Representation::Hexadecimal:
                    return "hexadecimal";
                case Formatting::Representation::Scientific:
                    return "scientific";
                case Formatting::Representation::Fixed:
                    return "fixed";
            }
        }
        
        Formatting::Justification to_justification(char justification) {
            switch (justification) {
                case '<':
                    return Formatting::Justification::Left;
                case '>':
                    return Formatting::Justification::Right;
                case '^':
                    return Formatting::Justification::Center;
                default:
                    ASSERT(false, "unknown justification value ({})", justification);
                    return Formatting::Justification::Right;
            }
        }
        
        Formatting::Sign to_sign(char sign) {
            switch (sign) {
                case '-':
                    return Formatting::Sign::NegativeOnly;
                case ' ':
                    return Formatting::Sign::Aligned;
                case '+':
                    return Formatting::Sign::Both;
                default:
                    ASSERT(false, "unknown sign value ({})", sign);
            }
        }
        
        Formatting::Representation to_representation(char representation) {
            switch (representation) {
                case 'd':
                    return Formatting::Representation::Decimal;
                case 'e':
                    return Formatting::Representation::Scientific;
                case 'f':
                    return Formatting::Representation::Fixed;
                case 'b':
                    return Formatting::Representation::Binary;
                case 'x':
                    return Formatting::Representation::Hexadecimal;
                default:
                    ASSERT(false, "unknown representation value ({})", representation);
            }
        }
        
        // FormatString implementation
        FormatString::FormatString(std::string_view in) {
            bool processing_placeholder = false;
            std::size_t placeholder_start = in.length();
            bool is_format_string_structured = false;
            
            for (std::size_t i = 0u; i < in.length(); ++i) {
                if (processing_placeholder) {
                    if (in[i] == '}') {
                        // Substring to get the placeholder without opening / closing braces.
                        std::string_view placeholder = in.substr(placeholder_start + 1u, i - placeholder_start - 1u);
                        
                        std::size_t offset = 0u;
                        std::size_t split_position = placeholder.find(':');
                        
                        // Parse identifier and handle format string errors.
                        std::string_view identifier = placeholder.substr(offset, split_position);
                        Result<Identifier, Error> parse_identifier_result = parse_identifier(identifier);
                        if (!parse_identifier_result.ok()) {
                            const Error& error = parse_identifier_result.error();
                            switch (error.code) {
                                case ErrorCode::Whitespace:
                                    throw FormatError("error parsing format string - encountered whitespace character at position {}", error.position);
                                case ErrorCode::DomainError:
                                    throw FormatError("error parsing format string - positional placeholder value {} at position {} is out of range", identifier, placeholder_start);
                                case ErrorCode::InvalidIdentifier:
                                    throw FormatError("error parsing format string - named placeholder {} at position {} is not a valid identifier", identifier, placeholder_start);
                                default:
                                    ASSERT(false, "unknown error code returned from parse_identifier ({}) while parsing identifier '{}'", static_cast<std::underlying_type<ErrorCode>::type>(error.code), identifier);
                            }
                        }
                        
                        // Verify format string placeholder homogeneity - auto-numbered placeholders cannot be mixed with positional / named ones.
                        const Identifier& parsed_identifier = parse_identifier_result.result();
                        if (m_placeholder_identifiers.empty()) {
                            // The format string type is determined by the type of the first placeholder.
                            is_format_string_structured = parsed_identifier.type != Identifier::Type::None;
                        }
                        else {
                            // A format string can either be composed of only unstructured (auto-numbered) placeholders or only structured (positional / named) placeholders.
                            // Mixing the two is not valid.
                            bool homogeneous = !is_format_string_structured && parsed_identifier.type == Identifier::Type::None ||
                                               is_format_string_structured && parsed_identifier.type == Identifier::Type::Position || parsed_identifier.type == Identifier::Type::Name;
                            
                            if (!homogeneous) {
                                throw FormatError("error parsing format string - format string placeholders must be homogeneous ({} format string has {} placeholder at position {})", (is_format_string_structured ? "structured" : "unstructured"), parsed_identifier.type, placeholder_start);
                            }
                        }
                        
                        std::size_t insertion_point = m_format.length();
                        
                        // Format specifier strings can be nested, separated by ':'. This is particularly useful for formatting containers of objects, where it is desirable
                        // to specify separate format specifiers for the container and individual container items. Format specifiers are always applied in a top-down fashion.
                        
                        Formatting formatting { };
                        std::vector<Formatting*> nested_formatting { };
                        bool first = true;
                        
                        // If there are no format specifier strings, formatting remains default-initialized.
                        while (split_position != std::string::npos) {
                            std::size_t previous_split_position = split_position;
                            
                            // Do not include ':' separator in further searches / substrings.
                            offset = split_position + 1u;
                            split_position = placeholder.find(':', offset);
                            std::string_view format_specifiers = placeholder.substr(offset, split_position - offset);
                            
                            Result<Formatting, Error> parse_formatting_result = parse_formatting(format_specifiers);
                            if (!parse_formatting_result.ok()) {
                                const Error& error = parse_formatting_result.error();
                                switch (error.code) {
                                    case FormatString::ErrorCode::InvalidFormatSpecifier:
                                        throw FormatError("error parsing format string - unknown format specifier {} at position {}", format_specifiers[error.position], error.position + placeholder_start + 1u);
                                    case FormatString::ErrorCode::EmptyFormatSpecifierString:
                                        throw FormatError("error parsing format string - empty format specifier string at position {}", previous_split_position);
                                    default:
                                        ASSERT(false, "unknown error code returned from parse_formatting ({}) while parsing format specifier string '{}'", static_cast<std::underlying_type<ErrorCode>::type>(error.code), format_specifiers);
                                }
                            }
                            
                            if (first) {
                                formatting = parse_formatting_result.result();
                                first = false;
                            }
                            else {
                                nested_formatting.emplace_back(new Formatting(parse_formatting_result.result()));
                            }
                        }
                        
                        // Configure the formatting parent chain so that nested Formatting objects can be accessed properly.
                        for (int j = static_cast<int>(nested_formatting.size()) - 2; j >= 0; --j) {
                            nested_formatting[j]->set_nested_formatting(nested_formatting[j + 1]);
                        }

                        if (!nested_formatting.empty()) {
                            // Connect nested chain to the top-level Formatting object.
                            formatting.set_nested_formatting(nested_formatting[0]);
                        }
                        
                        register_placeholder(parse_identifier_result.result(), formatting, insertion_point);
                        processing_placeholder = false;
                    }
                }
                else {
                    bool is_last_character = i == (in.length() - 1u);
                    bool is_escape_sequence = (in[i] == '{' && !is_last_character && in[i + 1u] == '{') ||
                                              (in[i] == '}' && !is_last_character && in[i + 1u] == '}');
                    
                    if (is_escape_sequence) {
                        // Escaped brace character.
                        // Resulting string will only have one brace (instead of the two in the format string).
                        m_format += in[i];
                        
                        // Skip over the second brace character.
                        i += 1u;
                    }
                    else if (in[i] == '{') {
                        // Start of placeholder.
                        processing_placeholder = true;
                        placeholder_start = i;
                        
                    }
                    else if (in[i] == '}') {
                        // This code path would never be hit if the '}' character belonged to a previously-opened placeholder scope, as this path is processed above.
                        // Therefore, this is an unescaped '}' character that does NOT belong to a scope, which is not a valid format string.
                        throw std::runtime_error("");
                    }
                    else {
                        // Non-special characters are left unmodified.
                        m_format += in[i];
                    }
                }
            }
            
            if (processing_placeholder) {
                throw FormatError("error parsing format string - unterminated placeholder at position {}", placeholder_start);
            }
        }
        
        FormatString::~FormatString() = default;
        
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
            int highest_position = 0u;
            
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

        Result<FormatString::Identifier, FormatString::Error> FormatString::parse_identifier(std::string_view in) const {
            if (in.empty()) {
                // Detected auto-numbered placeholder - {}.
                return Result<Identifier, Error>::OK();
            }
            else {
                // A placeholder identifier should not have any whitespace characters.
                for (std::size_t i = 0u; i < in.length(); ++i) {
                    if (std::isspace(in[i])) {
                        return Result<Identifier, Error>::NOT_OK(ErrorCode::Whitespace, static_cast<int>(i));
                    }
                }
                
                // Note: regex_match checks the entire input string. This behaviour can be simulated by using
                // regex search and anchoring the input string by using '^' (start) and '$' (end), but has been
                // omitted for simplicity.
                
                // Determine if placeholder is positional or named.
                
                // Positional placeholders can only be positive integers.
                if (std::regex_match(in.begin(), in.end(), std::regex("[0-9]+"))) {
                    int index;
                    const char* start = in.data();
                    const char* end = start + in.length();
                    std::from_chars_result conversion_result = std::from_chars(start, end, index, 10);
                    
                    // Regex check asserts that std::from_chars(...) will only return std::errc::result_out_of_range if the position value exceeds that of an integer.
                    // Note that this value is ~2.14 billion and should be considered a hard limit on the number of positional arguments for a single format string.
                    if (conversion_result.ec == std::errc::result_out_of_range) {
                        return Result<Identifier, Error>::NOT_OK(ErrorCode::DomainError);
                    }
                    
                    return Result<Identifier, Error>::OK(index);
                }
                // Named placeholders follow the same naming convention as C++ identifiers:
                //  - start with a letter or underscore
                //  - followed by any combination of letters, digits, or underscores (\w)
                else if (std::regex_match(in.begin(), in.end(), std::regex("[a-zA-Z_]\\w*"))) {
                    return Result<Identifier, Error>::OK(std::string(in));
                }
                else {
                    return Result<Identifier, Error>::NOT_OK(ErrorCode::InvalidIdentifier);
                }
            }
        }
        
        Result<Formatting, FormatString::Error> FormatString::parse_formatting(std::string_view in) const {
            if (in.empty()) {
                return Result<Formatting, FormatString::Error>::NOT_OK(ErrorCode::EmptyFormatSpecifierString);
            }
            
            Formatting formatting { };
            
            // Pattern for specifying custom formatting: ([fill][alignment]) [sign][#][minimum width][,][.precision][representation]
            // Regex expression: ([\s\S]?[<>^])?([+ -])?(\#)?(\d+)?(\,)?(\.\d*)?([de%fbox])?
            //   - alignment and (optionally) fill character - ([\s\S]?[<>^])?
            //   - sign - ([+ -])?
            //   - use type base prefix - (\#)?
            //   - minimum output width - (\d+)?
            //   - separate thousands+ with a comma - (\,)?
            //   - floating point precision - (\.\d*)?
            //   - type representation - ([de%fbox])?
            // Note: custom format specifiers are entirely optional and the input string may contain all or none.
            std::match_results<std::string_view::const_iterator> match { };
            if (std::regex_match(in.begin(), in.end(), match, std::regex("([\\s\\S]?[<>^])?([+ -])?(\\#)?(\\d+)?(\\,)?(\\.\\d*)?([de%fbox])?"))) {
                // Group 0: entire format string (skipped).
                unsigned group = 0u;
                ++group;
                
                // Group 1: fill character, justification
                if (match[group].matched) {
                    std::string submatch = match[group].str();
                    if (submatch.length() == 1) {
                        // Justification is the only required format specifier for group 1 (fill character is optional and defaults to a whitespace, ' ').
                        formatting.justification.set(to_justification(submatch[0]));
                    }
                    else {
                        formatting.fill = submatch[0];
                        formatting.justification.set(to_justification(submatch[1]));
                    }
                }
                ++group;
                
                // Group 2: sign
                if (match[group].matched) {
                    formatting.sign.set(to_sign(match[group].str()[0]));
                }
                ++group;
                
                // Group 3: base prefix
                if (match[group].matched) {
                    formatting.wildcard.set(true);
                }
                ++group;
                
                // Group 4: minimum output width
                if (match[group].matched) {
                    formatting.width.set(std::stoul(match[group].str()));
                }
                ++group;
                
                // Group 5: thousands separator
                if (match[group].matched) {
                    formatting.separator.set(match[group].str()[0]);
                }
                ++group;
                
                // Group 6: floating point maximum precision
                if (match[group].matched) {
                    std::uint32_t precision = std::stoul(match[group].str().substr(1)); // Do not include leading '.'
                    if (precision > std::numeric_limits<std::uint8_t>::max()) {
                        // TODO: warning message
                    }
                    formatting.precision.set(static_cast<std::uint8_t>(precision));
                }
                ++group;
                
                // Group 7: representation
                if (match[group].matched) {
                    formatting.representation.set(to_representation(match[group].str()[0]));
                }
                ++group;
            }
            else {
                // By breaking down the regex used in the check above into its logical components and keeping track of the position and length of matches,
                // we can determine and report back the position at which the regex first does not match the input string. Note that all the formatting
                // specifiers are entirely optional. A successful match moves the starting position by the length of the matched substring - this greedy
                // approach ensures that  characters are matched only once and at the earliest possible point. Upon failure to match, either due to
                // encountering an unsupported or out of place format specifier, the algorithm simply moves on and attempts to match against the next
                // regex at the same position. For improperly formed input strings, this position will be somewhere in the middle - this indicates the
                // first position at which the input diverged from the expected formatting specification. For properly formed input strings, this position
                // will be at the end (all characters matched). Note that this approach can also be used for determining valid input strings, but is less
                // performant as it evaluates multiple regex instead of the single one used above. This hit to performance is ignored is this code branch,
                // as specifying invalid or out of place format specifiers ultimately results in a runtime exception being thrown anyway.
                
                // (see breakdown of regex logical component ordering above):
                static const char* patterns[7] = { "^([\\s\\S]?[<>^])?", "^([+ -])?", "^(\\#)?", "^(\\d+)?", "^(\\,)?", "^(\\.\\d*)?", "^([de%fbox])?" };
                
                // Valid custom format specifier strings have an upper bound on their length.
                int position = 0;
                std::string_view::const_iterator start = in.begin();
                std::string_view::const_iterator end = in.end();
                
                for (const char* pattern : patterns) {
                    if (std::regex_search(start + position, end, match, std::regex(pattern))) {
                        // The characters matched represent a valid format specifier and should be ignored when determining the position of the first invalid one.
                        position += static_cast<int>(match.length());
                    }
                }
                
                return Result<Formatting, Error>::NOT_OK(ErrorCode::InvalidFormatSpecifier, position);
            }
            
            return Result<Formatting, Error>::OK(formatting);
        }
        
        void FormatString::register_placeholder(const Identifier& identifier, const Formatting& formatting, std::size_t position) {
            std::size_t placeholder_index = m_placeholder_identifiers.size(); // invalid

            if (identifier.type != Identifier::Type::None) {
                // Auto-numbered placeholders should not be de-duped and always count as a new placeholder.
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
        
        // Identifier implementation.
        FormatString::Identifier::Identifier() : type(Type::None),
                                                 position(-1),
                                                 name() {
        }

        FormatString::Identifier::Identifier(int position) : type(Type::Position),
                                                             position(position),
                                                             name() {
        }
        
        FormatString::Identifier::Identifier(std::string name) : type(Type::Name),
                                                                 position(-1),
                                                                 name(std::move(name)) {
        }
        
        FormatString::Identifier::~Identifier() = default;
        
        bool FormatString::Identifier::operator==(const Identifier& other) const {
            return type == other.type && position == other.position && name == other.name;
        }
        
        
        
        
        


        
        FormatString::FormattedPlaceholder::FormattedPlaceholder(std::size_t placeholder_index, const Formatting& formatting) : placeholder_index(placeholder_index),
                                                                                                                                formatting(formatting) {
        }
        
        FormatString::FormattedPlaceholder::~FormattedPlaceholder() = default;
        
        void FormatString::FormattedPlaceholder::add_insertion_point(std::size_t position) {
            insertion_points.emplace_back(position);
        }
        
        FormatString::Error::Error(ErrorCode code, int position) : code(code),
                                                                   position(position) {
        }
        
        FormatString::Error::~Error() = default;
        
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
    
    Formatting::Formatting() : justification(Justification::Right),
                               representation(Representation::Decimal),
                               sign(Sign::NegativeOnly),
                               fill(' '),
                               separator(' '),
                               wildcard(false),
                               precision(6u),
                               width(0u),
                               m_nested(nullptr) {
    }
    
    Formatting::Formatting(const Formatting& other) : justification(other.justification),
                                                      representation(other.representation),
                                                      sign(other.sign),
                                                      fill(other.fill),
                                                      separator(other.separator),
                                                      wildcard(other.wildcard),
                                                      precision(other.precision),
                                                      width(other.width),
                                                      m_nested(nullptr) {
        if (other.m_nested) {
            // Perform deep copy.
            m_nested = new Formatting(*other.m_nested);
        }
    }
    
    Formatting::~Formatting() {
        delete m_nested;
    }
    
    bool Formatting::operator==(const Formatting& other) const {
        bool formatting_equal = *justification == *other.justification &&
                                *representation == *other.representation &&
                                *sign == *other.sign &&
                                *fill == *other.fill &&
                                *separator == *other.separator &&
                                *wildcard == *other.wildcard &&
                                *precision == *other.precision &&
                                *width == *other.width;
        
        // Recursively compare nested formatting, assuming both pointers are valid.
        bool nested_formatting_equal = m_nested && other.m_nested && (*m_nested == *other.m_nested);
        
        return formatting_equal && nested_formatting_equal;
    }
    

    
    Formatting& Formatting::operator=(const Formatting& other) {
        if (this == &other) {
            return *this;
        }

        justification = other.justification;
        representation = other.representation;
        sign = other.sign;
        fill = other.fill;
        separator = other.separator;
        wildcard = other.wildcard;
        precision = other.precision;
        width = other.width;
        
        if (other.m_nested) {
            // Perform deep copy.
            m_nested = new Formatting(*other.m_nested);
        }
        
        return *this;
    }
    
    void Formatting::set_nested_formatting(Formatting* nested) {
        m_nested = nested;
    }
    
    Formatting Formatting::nested() const {
        if (m_nested) {
            return *m_nested;
        }
        
        // No nested formatting provided, use defaults.
        return { };
    }
    
    FormatError::~FormatError() = default;
    
}