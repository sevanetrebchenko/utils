
#include "utils/string.hpp"
#include "utils/result.hpp"
#include "utils/internal/string.tpp"

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
            }
            
            // TODO: assert
            return "";
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
                    // TODO: assert
                    break;
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
                    // TODO: assert
                    break;
            }
        }
        
        Formatting::Representation to_representation(char representation) {
            switch (representation) {
                case 'd':
                    return Formatting::Representation::Decimal;
                case 'e':
                    return Formatting::Representation::Scientific;
                case '%':
                    return Formatting::Representation::Percentage;
                case 'f':
                    return Formatting::Representation::Fixed;
                case 'b':
                    return Formatting::Representation::Binary;
                case 'o':
                    return Formatting::Representation::Octal;
                case 'x':
                    return Formatting::Representation::Hexadecimal;
                default:
                    // TODO: assert
                    break;
            }
        }
        
        // FormatString implementation
        FormatString::FormatString(std::string_view in) {
            bool processing_placeholder = false;
            std::size_t placeholder_start = in.length();
            
            Identifier::Type format_string_type;
            bool is_format_string_structured = false;
            
            for (std::size_t i = 0u; i < in.length(); ++i) {
                if (processing_placeholder) {
                    if (in[i] == '}') {
                        // Substring to get the placeholder without opening / closing braces.
                        std::string_view placeholder = in.substr(placeholder_start + 1u, i - placeholder_start - 1u);
                        
                        std::size_t split_position = placeholder.find(':');
                        
                        // Parse identifier and handle format string errors.
                        std::string_view identifier = placeholder.substr(0, split_position);
                        Result<Identifier, ErrorCode> parse_identifier_result = parse_identifier(identifier);
                        if (!parse_identifier_result.ok()) {
                            switch (parse_identifier_result.error()) {
                                case ErrorCode::Whitespace: {
                                    static char whitespace[] = { ' ', '\n', '\t', '\v', '\f', '\r' };
                                    std::size_t whitespace_position = identifier.find_first_of(whitespace);
                                    // TODO: assert
                                    throw FormatError("error parsing format string - encountered whitespace character at position {}", whitespace_position);
                                }
                                case ErrorCode::DomainError:
                                    throw FormatError("error parsing format string - positional placeholder value {} at position {} is out of range", identifier, placeholder_start);
                                case ErrorCode::InvalidIdentifier:
                                    throw FormatError("error parsing format string - named placeholder {} at position {} is not a valid identifier", identifier, placeholder_start);
                                default:
                                    // TODO: assert
                                    break;
                            }
                        }
                        
                        // Verify format string placeholder homogeneity - auto-numbered placeholders cannot be mixed with positional / named ones.
                        const Identifier& parsed_identifier = parse_identifier_result.result();
                        if (m_placeholder_identifiers.empty()) {
                            // The type of a format string is determined by the type of the first placeholder.
                            format_string_type = parsed_identifier.type;
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
                        
                        // Placeholder formatting specifiers are optional.
                        if (split_position != std::string::npos) {
                            // Parse top-level formatting specifiers.
                            std::size_t start = split_position + 1u;
                            split_position = placeholder.find(':', start);
                            std::string_view format_specifiers = placeholder.substr(start, split_position - start);

                            Result<Formatting, FormatString::ErrorCode> parse_formatting_result = parse_formatting(format_specifiers);
                            if (!parse_formatting_result.ok()) {
                                switch (parse_formatting_result.error()) {
                                    case FormatString::ErrorCode::InvalidFormatSpecifier:
                                        break;
//                                        throw std::runtime_error(utils::format("error parsing format string - unknown format specifier {} at index {}", format_specifiers[error.position], error.position + placeholder_start + 1u));
                                }
                            }

                            Formatting& formatting = parse_formatting_result.result();
//
//                            while (split_position != std::string::npos) {
//                                start = split_position + 1u;
//                                split_position = placeholder.find(':', start);
//                                format_specifiers = placeholder.substr(start, split_position - start);
//
//                                Result<Formatting, FormatError> result = parse_formatting(format_specifiers);
//                                if (!result.ok()) {
//                                    const FormatError& error = result.error();
//                                    switch (error.type) {
//                                        case FormatError::Type::InvalidFormatSpecifier:
//                                            throw std::runtime_error(utils::format("error parsing format string - unknown format specifier {} at index {}", format_specifiers[error.position], error.position + placeholder_start + 1u));
//                                    }
//                                }
//
//                                formatting.nested_formatting = std::make_shared<Formatting>(result.result());
//                            }
//
//                            register_placeholder(parse_identifier_result.result(), formatting, insertion_point);
                        }
                        else {
                            // Use default formatting.
                            register_placeholder(parse_identifier_result.result(), { }, insertion_point);
                        }
                        
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

#include <iostream>
        
        Result<FormatString::Identifier, FormatString::ErrorCode> FormatString::parse_identifier(std::string_view in) const {
            if (in.empty()) {
                // Detected auto-numbered placeholder - {}.
                return Result<Identifier, ErrorCode>::OK();
            }
            else {
                // A placeholder identifier should not have any whitespace characters.
                for (char i : in) {
                    if (std::isspace(i)) {
                        return Result<Identifier, ErrorCode>::NOT_OK(ErrorCode::Whitespace);
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
                        return Result<Identifier, ErrorCode>::NOT_OK(ErrorCode::DomainError);
                    }
                    
                    return Result<Identifier, ErrorCode>::OK(index);
                }
                // Named placeholders follow the same naming convention as C++ identifiers:
                //  - start with a letter or underscore
                //  - followed by any combination of letters, digits, or underscores (\w)
                else if (std::regex_match(in.begin(), in.end(), std::regex("[a-zA-Z_]\\w*"))) {
                    return Result<Identifier, ErrorCode>::OK(std::string(in));
                }
                else {
                    return Result<Identifier, ErrorCode>::NOT_OK(ErrorCode::InvalidIdentifier);
                }
            }
        }
        
        // Valid format specifiers:
        //   > : right-justify content to available space
        //   < : left-justify content to available space
        //   ^ : center content to available space
        //   d : decimal
        //   e : scientific notation
        //   % : percentage
        //   f : fixed
        //   b : binary
        //   o : octal
        //   x : hexadecimal
        //   - : display minus sign for negative values only
        //     : display minus sign for negative values, insert space before positive values (aligned)
        //   + : display minus sign for negative values, plus sign for positive values
        Result<Formatting, FormatString::ErrorCode> FormatString::parse_formatting(std::string_view in) const {
            // ( [fill] [alignment] ) [sign] [#] [width] [,] [.precision] [representation]
            Formatting formatting { };
            
            // Regex expression for valid format specifiers: ([\s\S]?[<>^])?([+ -])?(\#)?(\d*)?(\,)?(\.\d*)?([de%fbox])?
            // Breakdown:
            // Parse alignment and (optionally) fill character - ([\s\S]?[<>^])?
            // Parse sign - ([+ -])?
            // Parse whether to use base prefix or not - (\#)?
            // Parse minimum output width - (\d*)?
            // Parse whether to use a separator for thousands - (\,)?
            // Parse floating point precision - (\.\d*)?
            // Parse for type representation - ([de%fbox])?
            std::regex pattern = std::regex(R"(([\s\S]?[<>^])?([+ -])?(\#)?(\d*)?(\,)?(\.\d*)?([de%fbox])?)");
            std::match_results<std::string_view::const_iterator> match { };
            
            if (std::regex_match(in.begin(), in.end(), match, pattern)) {
                // Group 0: entire format string (skipped).
                unsigned group = 0u;
                ++group;
                
                // Group 1: fill character, justification
                if (match[group].matched) {
                    std::string submatch = match[group].str();
                    if (submatch.length() == 1) {
                        // Justification is the only required format specifier for group 1 (fill character is optional and defaults to a whitespace, ' ').
                        formatting.justification = to_justification(submatch[0]);
                    }
                    else {
                        formatting.fill = submatch[0];
                        formatting.justification = to_justification(submatch[1]);
                    }
                }
                ++group;
                
                // Group 2: sign
                if (match[group].matched) {
                    formatting.sign = to_sign(match[group].str()[0]);
                }
                ++group;
                
                // Group 3: base prefix
                if (match[group].matched) {
                    formatting.use_base_prefix = true;
                }
                ++group;
                
                // Group 4: minimum output width
                if (match[group].matched) {
                    formatting.width = std::stoul(match[group].str());
                }
                ++group;
                
                // Group 5: thousands separator
                if (match[group].matched) {
                    formatting.use_separator = true;
                }
                ++group;
                
                // Group 6: floating point maximum precision
                if (match[group].matched) {
                    std::uint32_t precision = std::stoul(match[group].str().substr(1)); // Do not include leading '.'
                    if (precision > std::numeric_limits<std::uint8_t>::max()) {
                        // TODO: warning message
                    }
                    formatting.precision = static_cast<std::uint8_t>(precision);
                }
                ++group;
                
                // Group 7: representation
                if (match[group].matched) {
                    formatting.representation = to_representation(match[group].str()[0]);
                }
                ++group;
            }
            else {
            
            }
            
            return Result<Formatting, FormatString::ErrorCode>::OK(formatting);
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
                               use_separator(false),
                               use_base_prefix(false),
                               precision(6u),
                               width(0u),
                               nested(std::shared_ptr<Formatting>(nullptr)) {
    }
    
    Formatting::~Formatting() = default;
    
    bool Formatting::operator==(const Formatting& other) const {
        return *justification == *other.justification &&
               *representation == *other.representation &&
               *sign == *other.sign &&
               *fill == *other.fill &&
               *use_separator == *other.use_separator &&
               *use_base_prefix == *other.use_base_prefix &&
               *precision == *other.precision &&
               *width == *other.width &&
               *nested == *other.nested;
    }
    
    FormatError::~FormatError() {
    }
    
}