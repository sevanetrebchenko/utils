
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

namespace utils {
    namespace internal {
        
        class PlaceholderIdentifier {
            public:
                enum class Type {
                    None = 0,
                    Position,
                    Name
                };

                explicit PlaceholderIdentifier(std::string_view identifier);
                static Result<PlaceholderIdentifier> parse(std::string_view identifier) noexcept;
                
                PlaceholderIdentifier();
                ~PlaceholderIdentifier();
                
                [[nodiscard]] bool operator==(const PlaceholderIdentifier& other) const;

                Type type;
                std::size_t position; // Allows for ~2 billion unique positions in a single format string.
                std::string name;
                
            private:
                explicit PlaceholderIdentifier(int position);
                explicit PlaceholderIdentifier(std::string name);
        };
        
        struct PlaceholderFormatting {
            enum Justification {
                Right = 0,
                Left,
                Center
            };
            
            enum Representation {
                Decimal = 0,
                Binary,
                Unicode,
                Octal,
                Hexadecimal
            };
            
            enum Sign {
                NegativeOnly = 0,
                Aligned,
                Both
            };
            
            explicit PlaceholderFormatting(std::string_view specifiers);
            static Result<PlaceholderFormatting> parse(std::string_view specifiers) noexcept;
            
            PlaceholderFormatting();
            ~PlaceholderFormatting();
            
            [[nodiscard]] bool operator==(const PlaceholderFormatting& other) const;
        
            Justification justification;
            Representation representation;
            Sign sign;
            char fill;
            char separator;
            unsigned width;
            unsigned precision;
        };
        
        class FormatString {
            public:
                explicit FormatString(std::string_view format_string);
                static Result<FormatString> parse(std::string_view format_string);
            
                FormatString();
                ~FormatString();

                template <typename ...Ts>
                [[nodiscard]] Result<std::string> format(const Ts&... args) const;
                
                [[nodiscard]] std::size_t get_total_placeholder_count() const; // Includes duplicates
                [[nodiscard]] std::size_t get_unique_placeholder_count() const;
                [[nodiscard]] std::size_t get_positional_placeholder_count() const;
                [[nodiscard]] std::size_t get_named_placeholder_count() const;
                
            private:
                struct FormattedPlaceholder {
                    FormattedPlaceholder(std::size_t placeholder_index, const PlaceholderFormatting& formatting);
                    ~FormattedPlaceholder();
                    
                    void add_insertion_point(std::size_t position);
                    
                    std::size_t placeholder_index;
                    PlaceholderFormatting formatting;

                    // Positional and named placeholders can appear multiple times in the same format string.
                    // An optimization we can make when formatting placeholders is to format all unique placeholders once and cache them for later. A placeholders
                    // uniqueness is determined by its formatting specifiers - that is, if a placeholder has the same identifier and format specifiers as another,
                    // both placeholders will be formatted in the same way.
                    // We can save on processing power by simply keeping track of the positions in which a given placeholder and formatting specifiers are
                    // used to take advantage of work that was already done and avoid unnecessary duplicate formatting operations.
                    std::vector<std::size_t> insertion_points;
                };

                void register_placeholder(const PlaceholderIdentifier& identifier, const PlaceholderFormatting& formatting, std::size_t position);
                
                // Verifies that placeholders are of the same type.
                // A format string can either contain all auto-numbered placeholders or a mix of positional and named placeholders.
                // Auto-numbered placeholders cannot be mixed in with placeholders of the other two types.
                [[nodiscard]] bool verify_placeholder_homogeneity() const;
                
                std::string m_format;
                std::vector<PlaceholderIdentifier> m_placeholder_identifiers;
                std::vector<FormattedPlaceholder> m_formatted_placeholders;
        };
        
        template <typename T>
        struct is_format_arg : std::false_type { };
        
        template <typename T>
        struct is_format_arg<arg<T>> : std::true_type { };
        
        template <typename T>
        [[nodiscard]] std::string pointer_to_string(T pointer) {
            using Type = typename std::remove_reference<typename std::remove_cv<T>::type>::type;
            static_assert(std::is_pointer<Type>::value || std::is_null_pointer<Type>::value, "non-pointer type provided to pointer_to_string");
            
            static char buffer[2u * sizeof(void*) + 3u] { '\0' }; // Enough space to store a pointer address + an optional '0x' prefix (2 bytes) + null terminator (1 byte).
            static std::size_t buffer_size = sizeof(buffer) / sizeof(buffer[0]);
            
            int num_characters = std::snprintf(buffer, buffer_size, "%p", (void*)(pointer));
            if (num_characters < 0) {
                throw std::runtime_error("encoding error in pointer_to_string");
            }
            
            // The inclusion of the '0x' prefix is implementation dependent, and not all compilers may include it.
            if (buffer[0] == '0' && buffer[1] == 'x') {
                return { buffer, static_cast<std::size_t>(num_characters) };
            }
            else {
                return "0x" + std::string(buffer, static_cast<std::size_t>(num_characters));
            }
        }
        
        template <typename T>
        [[nodiscard]] std::string stringify(const T& value, const PlaceholderFormatting& formatting = {}) {
            using Type = typename std::decay<T>::type;
            
            if constexpr (std::is_fundamental<Type>::value) {
                if constexpr (std::is_null_pointer<Type>::value) {
                    // std::nullptr_t (nullptr)
                    return pointer_to_string(value);
                }
                else if constexpr (std::is_same<Type, bool>::value) {
                    return value ? "true" : "false";
                }
                else {
                    // TODO: other fundamental types.
                    return "9";
                }
            }
            else if constexpr (is_standard_container<Type>::value) {
                // Standard container types.
                auto iter = std::begin(value);
                auto end = std::end(value);
                
                if ((end - iter) == 0u) {
                    // Container has no elements.
                    return "[ ]";
                }
                
                std::string result = "[ ";
                result.append(stringify(*iter));
                for (++iter; iter != end; ++iter) {
                    result.append(", " + stringify(*iter));
                }
                result.append(" ]");

                return std::move(result);
            }
            else if constexpr (is_pair<Type>::value) {
                // std::pair
                std::string result = "{ ";
                result.append(stringify(value.first));
                result.append(", ");
                result.append(stringify(value.second));
                result.append(" }");
                return std::move(result);
            }
            else if constexpr (is_tuple<Type>::value) {
                // std::tuple
                std::string result = "{ ";
                
                // Use std::apply + fold expression to iterate over and format the elements of the tuple.
                std::apply([&result](auto&&... args) {
                    ((result.append(stringify(args) + ", ")), ...);
                }, value);
                
                // Overwrite the trailing ", " with " }".
                std::size_t length = result.length();
                result[length - 2u] = ' ';
                result[length - 1u] = '}';
                
                return std::move(result);
            }
            else if constexpr (std::is_same<Type, std::string>::value) {
                // std::string
                return "\"" + value + "\"";
            }
            else if constexpr (std::is_pointer<Type>::value) {
                // pointer types
                return pointer_to_string(value);
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
        Result<std::string> FormatString::format(const Ts& ...args) const {
            Result<std::string> result { m_format };
            
            if (!m_placeholder_identifiers.empty()) {
                auto tuple = std::make_tuple(args...);
                
                if (m_placeholder_identifiers[0].type == PlaceholderIdentifier::Type::None) {
                    // For auto-numbered placeholders, there is a 1:1 correlation between a placeholder value and its insertion point.
                    // Hence, the number of arguments provided to format(...) should be at least as many as the number of placeholders.
                    // Note: while it is valid to provide more arguments than necessary, these arguments will be ignored.
                    std::size_t placeholder_count = get_unique_placeholder_count();
                    
                    if (placeholder_count > sizeof...(args)) {
                        // Not enough arguments provided to format(...);
                        return Result<std::string>::NOT_OK("expecting {} arguments, but received {}", placeholder_count, sizeof...(args));
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
                            
                            return Result<std::string>::NOT_OK("encountered value for named placeholder {} at index {} - structured placeholder values are not allowed in auto-numbered format strings", name, i);
                        }
                    }
                }
                else {
                    // Format string should only contain positional / named placeholders.
                    std::size_t positional_placeholder_count = get_positional_placeholder_count();
                    std::size_t named_placeholder_count = get_named_placeholder_count();
                    
                    if (positional_placeholder_count + named_placeholder_count > sizeof...(args)) {
                        // Not enough arguments provided to format(...);
                        return Result<std::string>::NOT_OK("expecting {} arguments, but received {}", positional_placeholder_count + named_placeholder_count, sizeof...(args));
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
                                return Result<std::string>::NOT_OK("expecting value for positional placeholder {} (not referenced), but received value for named placeholder {} - values for all positional placeholders must come before any values for named placeholders", i, name);
                            }
                            else {
                                return Result<std::string>::NOT_OK("expecting value for positional placeholder {} (first referenced at index {}), but received value for named placeholder {} - values for all positional placeholders must come before any values for named placeholders", i, position, name);
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
                    const PlaceholderFormatting& formatting = placeholder.formatting;
                    
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
                    
                    result->insert(insertion_point.position + offset, placeholder_value);
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
    arg<T>::arg(std::string name, const T& value) : name(std::move(name)),
                                                    value(value) {
    }
    
    template <typename T>
    arg<T>::~arg() = default;
    
    template <typename ...Ts>
    std::string format(const std::string& fmt, const Ts&... args) {
        using namespace internal;
        
        Result<FormatString> parse_result = FormatString::parse(fmt);
        if (!parse_result.ok()) {
            throw std::runtime_error(parse_result.what());
        }
        
        const FormatString& format_string = *parse_result;

        Result<std::string> format_result = format_string.format(args...);
        if (!format_result.ok()) {
            throw std::runtime_error(format_result.what());
        }
        
        return *format_result;
    }
    
}

#endif // UTILS_STRING_TPP
