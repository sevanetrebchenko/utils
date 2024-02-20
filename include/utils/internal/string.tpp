
#ifndef UTILS_STRING_TPP
#define UTILS_STRING_TPP

#include "utils/constexpr.hpp"
#include "utils/concepts.hpp"
#include "utils/result.hpp"
#include "utils/tuple.hpp"

#include <functional>
#include <cstdio> // std::snprintf
#include <memory> // std::unique_ptr
#include <stdexcept> // std::runtime_error
#include <tuple> // std::tuple_cat, std::make_tuple
#include <regex> // std::regex_match
#include <iostream>
#include <cassert>
#include <ostream>
#include <sstream>
#include <iomanip>

namespace utils {
    namespace internal {
        
        class FormatString {
            public:
                FormatString(std::string_view in);
                ~FormatString();

                template <typename ...Ts>
                [[nodiscard]] std::string format(const Ts&... args) const;
                
                [[nodiscard]] std::size_t get_total_placeholder_count() const; // Includes duplicates
                [[nodiscard]] std::size_t get_unique_placeholder_count() const;
                [[nodiscard]] std::size_t get_positional_placeholder_count() const;
                [[nodiscard]] std::size_t get_named_placeholder_count() const;
                
            private:
                struct FormattedPlaceholder {
                    FormattedPlaceholder(std::size_t placeholder_index, const Formatting& formatting);
                    ~FormattedPlaceholder();
                    
                    void add_insertion_point(std::size_t position);
                    
                    std::size_t placeholder_index;
                    Formatting formatting;

                    // Positional and named placeholders can appear multiple times in the same format string.
                    // An optimization we can make when formatting placeholders is to format all unique placeholders once and cache them for later. A placeholders
                    // uniqueness is determined by its formatting specifiers - that is, if a placeholder has the same identifier and format specifiers as another,
                    // both placeholders will be formatted in the same way.
                    // We can save on processing power by simply keeping track of the positions in which a given placeholder and formatting specifiers are
                    // used to take advantage of work that was already done and avoid unnecessary duplicate formatting operations.
                    std::vector<std::size_t> insertion_points;
                };
                
                struct Identifier {
                    enum class Type {
                        None = 0,
                        Position,
                        Name
                    };
        
                    Identifier();
                    explicit Identifier(int position);
                    explicit Identifier(std::string name);
                    ~Identifier();
                    
                    [[nodiscard]] bool operator==(const Identifier& other) const;
        
                    Type type;
                    int position; // Allows for ~2 billion unique positions in a single format string.
                    std::string name;
                };

                // Allow external functions to convert
                friend std::string to_string(Identifier::Type);
                
                // Errors returned while parsing a format string.
                enum class ErrorCode {
                    Whitespace = 0,
                    DomainError,
                    InvalidIdentifier,
                    InvalidFormatSpecifier
                };
                
                struct Error {
                    Error(ErrorCode code);
                    Error(ErrorCode code, int position);
                    ~Error();
                    
                    ErrorCode code;
                    int position; // optional
                };
                
                [[nodiscard]] Result<Identifier, Error> parse_identifier(std::string_view in) const;
                [[nodiscard]] Result<Formatting, Error> parse_formatting(std::string_view in) const;
                
                void register_placeholder(const Identifier& identifier, const Formatting& formatting, std::size_t position);
                
                std::string m_format;
                std::vector<Identifier> m_placeholder_identifiers;
                std::vector<FormattedPlaceholder> m_formatted_placeholders;
        };
        
        template <typename T>
        struct is_format_arg : std::false_type { };
        
        template <typename T>
        struct is_format_arg<arg<T>> : std::true_type { };
        
        [[nodiscard]] inline char to_specifier(Formatting::Justification justification) {
            switch (justification) {
                case Formatting::Justification::Right:
                    return '>';
                case Formatting::Justification::Left:
                    return '<';
                case Formatting::Justification::Center:
                    return '^';
            }
            
            throw std::runtime_error("unknown justification");
        }
        
        [[nodiscard]] inline char to_specifier(Formatting::Representation representation) {
            switch (representation) {
                case Formatting::Representation::Decimal:
                    return 'd';
                case Formatting::Representation::Scientific:
                    return 'e';
                case Formatting::Representation::Percentage:
                    return '%';
                case Formatting::Representation::Fixed:
                    return 'f';
                case Formatting::Representation::Binary:
                    return 'b';
                case Formatting::Representation::Octal:
                    return 'o';
                case Formatting::Representation::Hexadecimal:
                    return 'x';
            }
            
            throw std::runtime_error("unknown representation");
        }
        
        [[nodiscard]] inline char to_specifier(Formatting::Sign sign) {
            switch (sign) {
                case Formatting::Sign::NegativeOnly:
                    return '-';
                case Formatting::Sign::Aligned:
                    return ' ';
                case Formatting::Sign::Both:
                    return '+';
            }
            
            throw std::runtime_error("unknown sign");
        }
        
        template <typename T>
        [[nodiscard]] std::string pointer_to_string(T pointer, const Formatting& formatting) {
            using Type = typename std::decay<T>::type;
            static_assert(std::is_pointer<Type>::value || std::is_null_pointer<Type>::value, "non-pointer type provided to pointer_to_string");

            std::stringstream builder { };
            
            // Formatting: sign
            if (formatting.sign.has_custom_value()) {
                // Signs on pointer values are not supported.
                char specifier = to_specifier(*formatting.sign);
                throw FormatError("error formatting format string - invalid specifier {} for pointer type", specifier);
            }
            
            // Formatting: separator
            if (formatting.use_separator.has_custom_value()) {
                throw FormatError("error formatting format string - invalid specifier ',' for pointer type");
            }
            
            // Formatting: data representation
            if (!formatting.representation.has_custom_value() || *formatting.representation == Formatting::Representation::Hexadecimal) {
                // By default, or explicitly, hexadecimal is used for representing pointer addresses.
                builder << std::hex;
            }
            else {
                char specifier = to_specifier(*formatting.representation);
                throw FormatError("error formatting format string - invalid specifier {} for pointer type", specifier);
            }
            
            if (*formatting.use_base_prefix) {
                builder << std::showbase;
            }
            
            builder << (pointer ? (void*) pointer : "nullptr");
            std::string value = std::move(builder.str());
            builder.str(std::string()); // Clear builder internals.
            
            std::int64_t current_width = static_cast<std::int64_t>(value.length());
            unsigned desired_width = *formatting.width;
            std::int64_t left_padding = 0u;
            std::int64_t right_padding = 0u;
            
            // Formatting: justification
            Formatting::Justification justification = *formatting.justification;
            if (justification == Formatting::Justification::Right) {
                if (desired_width > current_width) {
                    left_padding = current_width - desired_width;
                }
            }
            else if (justification == Formatting::Justification::Left) {
                if (desired_width > current_width) {
                    right_padding = current_width - desired_width;
                }
            }
            else {
                if (desired_width > current_width) {
                    std::int64_t extra = current_width - desired_width;
                    std::int64_t half = extra / 2;
                    if (extra % 2 == 0) {
                        left_padding = half;
                        right_padding = half;
                    }
                    else {
                        left_padding = half + 1;
                        right_padding = half;
                    }
                }
            }
            
            char fill_character = *formatting.fill;
            builder << std::setw(left_padding) << std::setfill(fill_character) << "" << value << std::setw(right_padding) << std::setfill(fill_character) << "";
            
            return std::move(builder.str());
        }
        
        template <typename T>
        [[nodiscard]] std::string fundamental_to_string(const T value, const Formatting& formatting) {
            return "";
            
//            using Type = typename std::decay<T>::type;
//            static_assert(std::is_fundamental<Type>::value, "type provided to fundamental_to_string must be built-in");
//
//            std::ostringstream out { };
//
//            // Convert true/false to their alphanumeric format
//            out << std::boolalpha;
//
//            // Formatting: width
//            // Maintains default std::stringstream width (to fit) if not specified.
//            if (formatting.width != -1) {
//                out << std::setw(formatting.width);
//            }
//
//            // Formatting: fill character
//            out << std::setfill(formatting.fill);
//
//            // Formatting: justification
//            switch (formatting.justification) {
//                case PlaceholderFormatting::Justification::Right:
//                    out << std::right;
//                    break;
//                case PlaceholderFormatting::Justification::Left:
//                    out << std::left;
//                    break;
//            }
//
//            // Formatting: representation (only applicable to integral type).
//            if (std::is_integral<Type>::value) {
//                switch (formatting.representation) {
//                    case PlaceholderFormatting::Representation::Decimal:
//                        out << std::setbase(10);
//                        break;
//                    case PlaceholderFormatting::Representation::Binary:
//                        out << std::setbase(2);
//                        break;
//                    case PlaceholderFormatting::Representation::Octal:
//                        out << std::setbase(8);
//                        break;
//                    case PlaceholderFormatting::Representation::Hexadecimal:
//                        out << std::setbase(16);
//                        break;
//                }
//            }
//            else {
//                throw std::runtime_error("");
//            }
//
//            // Formatting: precision (floating point values only).
//            if (std::is_floating_point<Type>::value) {
//                // Maintains default std::stringstream precision (6) if not specified.
//                if (formatting.precision != -1) {
//                    out << std::setprecision(formatting.precision);
//                }
//            }
//            else {
//                throw std::runtime_error("");
//            }
//
//            if constexpr (std::is_null_pointer<Type>::value) {
//                out << "nullptr";
//            }
//            else {
//                out << value;
//            }
//
//            return std::move(out.str());
        }
        
        template <typename T>
        [[nodiscard]] std::string container_to_string(const T& container, const Formatting& formatting) {
            static_assert(is_const_iterable<T>, "container type provided to container_to_string must support const iteration (begin/end).");
            auto current = std::begin(container);
            auto end = std::end(container);
            
            if ((end - current) == 0u) {
                // Special formatting for when a container has no elements.
                return "[]";
            }
            
            std::string result = "[ ";
            
            // If custom formatting is specified, it is applied to container elements.
            result.append(stringify(*current, formatting));
            for (++current; current != end; ++current) {
                result.append(", " + stringify(*current, formatting));
            }
            
            result.append(" ]");

            return std::move(result);
        }
        
        template <typename T>
        [[nodiscard]] std::string tuple_to_string(const T& value, const Formatting& formatting) {
            using Type = typename std::decay<T>::type;
            static_assert(is_pair<Type>::value || is_tuple<Type>::value, "type provided to tuple_to_string must be a tuple type");
            
            std::string result = "{ ";
            
            // Use std::apply + fold expression to iterate over and format the elements of the tuple.
            std::apply([&result, &formatting](const auto&&... args) {
                ((result.append(stringify(args, formatting) + ", ")), ...);
            }, value);
            
            // Overwrite the trailing ", " with " }".
            std::size_t length = result.length();
            result[length - 2u] = ' ';
            result[length - 1u] = '}';
            return result;
        }
        
        
        template <typename T>
        [[nodiscard]] std::string stringify(const T& value, const Formatting& formatting = {}) {
            using Type = typename std::decay<T>::type;
            
            if constexpr (std::is_fundamental<Type>::value && !std::is_null_pointer<Type>::value) {
                // C++ built-ins
                return fundamental_to_string(value, formatting);
            }
            else if constexpr (is_standard_container<Type>::value) {
                // standard C++ container
                return container_to_string(value, formatting);
            }
            else if constexpr (is_pair<Type>::value || is_tuple<Type>::value) {
                // std::pair, std::tuple
                return tuple_to_string(value, formatting);
            }
            else if constexpr (std::is_same<Type, std::string>::value) {
                // std::string
                return value;
            }
            else if constexpr (std::is_pointer<Type>::value || std::is_null_pointer<Type>::value) {
                // pointer types
                return pointer_to_string(value, formatting);
            }
            else if constexpr (is_format_arg<Type>::value) {
                return stringify(value.value, formatting);
            }
            // Utilize user-defined std::string conversion operators for all other custom types.
            // This implementation allows for either a std::string conversion class operator (T::operator std::string(), preferred) or a standalone to_string(const T&) function.
            else if constexpr (is_convertible_to_string<Type>) {
                // Note: can also use std::is_convertible<Type, std::string>::value if the conversion operator is not marked explicit.
                return std::string(value);
            }
            else {
                return to_string(value);
            }
        }
        
        template <typename Tuple, std::size_t N = 0>
        bool is_structured_argument_type(const Tuple& tuple, std::size_t index) {
            if (N == index) {
                using Type = typename std::decay<decltype(std::get<N>(tuple))>::type;
                return is_format_arg<Type>::value;
            }
            
            if constexpr (N + 1 < std::tuple_size<Tuple>::value) {
                return is_structured_argument_type<Tuple, N + 1>(tuple, index);
            }
            
            throw std::out_of_range(format("invalid tuple index {} provided to is_named_argument_type", index));
        }
        
        template <typename ...Ts>
        std::string FormatString::format(const Ts& ...args) const {
            std::string result = m_format;
            
            if (!m_placeholder_identifiers.empty()) {
                auto tuple = std::make_tuple(args...);
                
                if (m_placeholder_identifiers[0].type == Identifier::Type::None) {
                    // For auto-numbered placeholders, there is a 1:1 correlation between a placeholder value and its insertion point.
                    // Hence, the number of arguments provided to format(...) should be at least as many as the number of placeholders.
                    // Note: while it is valid to provide more arguments than necessary, these arguments will be ignored.
                    std::size_t placeholder_count = get_unique_placeholder_count();
                    
                    if (placeholder_count > sizeof...(args)) {
                        // Not enough arguments provided to format(...);
                        throw std::runtime_error(utils::format("error in call to format(...) - expecting {} arguments, but received {}", placeholder_count, sizeof...(args)));
                    }
                    
                    // Format string should only auto-numbered placeholders.
                    // Verify that there are no positional / named argument values in the argument list.
                    for (std::size_t i = 0u; i < placeholder_count; ++i) {
                        if (is_structured_argument_type(tuple, i)) {
                            const std::string& name = runtime_get(tuple, i, [i]<typename T>(const T& value) -> const std::string& {
                                if constexpr (is_format_arg<T>::value) {
                                    return value.name;
                                }
                                // This should never happen.
                                throw std::runtime_error(utils::format("internal runtime_get error - invalid type at tuple index {}", i));
                            });
                            
                            throw std::runtime_error(utils::format("encountered value for named placeholder {} at index {} - structured placeholder values are not allowed in auto-numbered format strings", name, i));
                        }
                    }
                }
                else {
                    // Format string should only contain positional / named placeholders.
                    std::size_t positional_placeholder_count = get_positional_placeholder_count();
                    std::size_t named_placeholder_count = get_named_placeholder_count();
                    
                    if (positional_placeholder_count + named_placeholder_count > sizeof...(args)) {
                        // Not enough arguments provided to format(...);
                        throw std::runtime_error(utils::format("expecting {} arguments, but received {}", positional_placeholder_count + named_placeholder_count, sizeof...(args)));
                    }
                    
                    for (std::size_t i = 0u; i < positional_placeholder_count; ++i) {
                        if (is_structured_argument_type(tuple, i)) {
                            const std::string& name = runtime_get(tuple, i, [i]<typename T>(const T& value) -> const std::string& {
                                if constexpr (is_format_arg<T>::value) {
                                    return value.name;
                                }
                                // This should never happen.
                                throw std::runtime_error(utils::format("internal runtime_get error - invalid type at tuple index {}", i));
                            });

                            // Retrieve the first position at which this placeholder was referenced.
                            std::size_t position = std::string::npos;
                            for (const FormattedPlaceholder& placeholder : m_formatted_placeholders) {
                                if (placeholder.placeholder_index == i && !placeholder.insertion_points.empty()) {
                                    position = placeholder.insertion_points[0];
                                }
                            }
                            
                            if (position != std::string::npos) {
                                throw std::runtime_error(utils::format("expecting value for positional placeholder {} (not referenced), but received value for named placeholder {} - values for all positional placeholders must come before any values for named placeholders", i, name));
                            }
                            else {
                                throw std::runtime_error(utils::format("expecting value for positional placeholder {} (first referenced at index {}), but received value for named placeholder {} - values for all positional placeholders must come before any values for named placeholders", i, position, name));
                            }
                        }
                    }
                    
//                    for (std::size_t i = 0u; i < named_placeholder_count; ++i) {
//                        if (!get(tuple, i + positional_placeholder_count, is_named_argument_type)) {
//                            return Result<std::string>::NOT_OK("expecting named argument type");
//                        }
//                    }

                    // Verify that all named placeholders have a corresponding argument.
//                    for (std::size_t i = 0u; i < get_unique_placeholder_count(); ++i) {
//                        const PlaceholderIdentifier& identifier = m_placeholder_identifiers[i];
//                        if (identifier.type == PlaceholderIdentifier::Type::Name) {
//                            bool found = for_each(tuple, [&identifier]<typename T>(const T& a) -> bool {
//                                if constexpr (is_format_arg<typename std::decay<T>::type>::value) {
//                                    return a.name == identifier.name;
//                                }
//                                else {
//                                    return false;
//                                }
//                            });
//
//                            if (!found) {
//                                return "named placeholder " + identifier.name + " missing argument";
//                            }
//                        }
//                    }
                }
                
                // Format all unique placeholders once to save on computation power.
                // This is only really applicable for positional / named placeholder values, since these can be referenced multiple times
                // in the format string, but this logic is near identical for auto-numbered placeholder values.
                std::size_t unique_placeholder_count = get_unique_placeholder_count();
                
                std::vector<std::string> formatted_placeholders;
                formatted_placeholders.reserve(unique_placeholder_count);

                for (std::size_t i = 0u; i < unique_placeholder_count; ++i) {
                    const FormattedPlaceholder& placeholder = m_formatted_placeholders[i];
                    const Formatting& formatting = placeholder.formatting;
                    
                    formatted_placeholders.emplace_back(runtime_get(tuple, i, [&formatting] <typename T>(const T& value) -> std::string {
                        return stringify(value, formatting);
                    }));
                }
                
                // Values for placeholders are inserted into the resulting string in reverse order of appearance so that inserting a
                // placeholder value does not offset the insertion positions of any placeholders that come before it.

                struct InsertionPoint {
                    std::size_t placeholder_index; // Index of the placeholder to insert.
                    std::size_t position; // Position at which the placeholder should be inserted.
                };
                
                // Comparator to create a min heap based on the insertion point position.
                static auto comparator = [](const InsertionPoint& a, const InsertionPoint& b) -> bool {
                    return a.position > b.position;
                };
                
                std::priority_queue<InsertionPoint, std::vector<InsertionPoint>, decltype(comparator)> insertion_points(comparator);
                for (std::size_t i = 0u; i < unique_placeholder_count; ++i) {
                    const FormattedPlaceholder& placeholder = m_formatted_placeholders[i];
                    
                    for (std::size_t position : placeholder.insertion_points) {
                        insertion_points.emplace(placeholder.placeholder_index, position);
                    }
                }
                
                // Placeholder values are inserted into the string front to back. This allows an easier way of handling insertions for
                // placeholders that are directly adjacent due to peculiarities with the std::string::insert() function inserting starting at
                // the character right before the indicated position. By keeping track of an offset and adjusting the insertion point of
                // subsequent placeholders accordingly, we can insert values for adjacent placeholders without any extra whitespace.
                
                std::size_t offset = 0u;
                
                while (!insertion_points.empty()) {
                    const InsertionPoint& insertion_point = insertion_points.top();
                    const std::string& placeholder_value = formatted_placeholders[insertion_point.placeholder_index];
                    
                    result.insert(insertion_point.position + offset, placeholder_value);
                    offset += placeholder_value.length();
                    
                    insertion_points.pop();
                }
            }
            
            return result;
        }
        
    }
    
    
    template <typename Container>
    [[nodiscard]] std::string join(const Container& container, const std::string& glue) {
        // TODO: support custom containers with iterators
//        static_assert(is_const_iterable<Container>, "container must support std::begin/std::end");
        
        auto iter = std::begin(container);
        std::string result;

//        result.append(internal::stringify(*iter));
//        for (++iter; iter != std::end(container); ++iter) {
//            result.append(glue);
//            result.append(internal::stringify(*iter));
//        }

        return std::move(result);
    }

    template <typename T>
    FormattingSpecifier<T>::FormattingSpecifier(T value) : m_value(std::make_pair(value, false)) {
    }

    template <typename T>
    FormattingSpecifier<T>::~FormattingSpecifier() = default;
    
    template <typename T>
    void FormattingSpecifier<T>::set_value(T value) {
        m_value.first = value;
        m_value.second = true; // Now holds a custom value.
    }
    
    template <typename T>
    bool FormattingSpecifier<T>::has_custom_value() const {
        return m_value.second;
    }
    
    template <typename T>
    T FormattingSpecifier<T>::operator*() const {
        return m_value.first;
    }
    
    template <typename T>
    T FormattingSpecifier<T>::get() const {
        return m_value.first;
    }
    
    template <typename ...Ts>
    FormatError::FormatError(std::string fmt, const Ts& ...args) : std::runtime_error(format(fmt, args...)) {
    }
    
    template <typename T>
    arg<T>::arg(std::string name, const T& value) : name(std::move(name)),
                                                    value(value) {
    }
    
    template <typename T>
    arg<T>::~arg() = default;
    
    template <typename ...Ts>
    std::string format(const std::string& in, const Ts&... args) {
        using namespace internal;
        FormatString format_string = FormatString(in);
        
        if constexpr (sizeof...(args) > 0u) {
            return format_string.format(args...);
        }
        else {
            // TODO: check for placeholders.
            return in;
        }
    }
    
}

#endif // UTILS_STRING_TPP
