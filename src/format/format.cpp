
#include "utils/format/format.hpp"
#include "utils/logging.hpp"
#include "utils/string.hpp"
#include <cstring> // std::memcpy

namespace utils {
    
    template <typename T>
    constexpr std::size_t count_digits(T num) {
        return (num < 10) ? 1 : 1 + count_digits(num / 10);
    }

    FormatString::FormatString(const FormatString& fmt, std::source_location source_location) : m_format(fmt.m_format),
                                                                                                m_source(source_location),
                                                                                                m_identifiers(fmt.m_identifiers),
                                                                                                m_specifications(fmt.m_specifications),
                                                                                                m_placeholders(fmt.m_placeholders) {
    }
    
    FormatString::~FormatString() = default;
    
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
                    // Can also use Formatter<T>::reserve for this, but this is evaluated at compile time, so it's faster
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
    
    std::string FormatString::string() const {
        return *this;
    }

    std::source_location FormatString::source() const {
        return m_source;
    }
    
    std::size_t FormatString::get_placeholder_count() const {
        return get_positional_placeholder_count() + get_named_placeholder_count();
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
                    // Remove the braces from the format string as the input string is only parsed once
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

        // Issue a warning if not all positional arguments are used
        for (std::size_t position = 0u; position < get_positional_placeholder_count(); ++position) {
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
    
}