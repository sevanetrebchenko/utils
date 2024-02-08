
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
                int position; // Allows for ~2 billion unique positions in a single format string.
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
                [[nodiscard]] std::string format(const Ts&... args) const;
                
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
                    // uniqueness is determined by its formatting specifiers - if a placeholder has the same identifier and format specifiers as another, both
                    // placeholders will be formatted in the same way.
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
        
//        template <typename T, typename ...Rest>
//        auto to_string_tuple(const T& value, const Rest&... rest) {
//            return std::tuple_cat(std::make_tuple(stringify(value)), internal::to_string_tuple<Rest>(rest)...);
//        }

        
        template <typename ...Ts>
        std::string FormatString::format(const Ts& ...args) const {
            std::string result = m_format;

            if (!m_placeholder_identifiers.empty()) {
                auto tuple = std::make_tuple(args...);
                
                if (get_unique_placeholder_count() > sizeof...(args)) {
                    // Not enough arguments provided to format(...);
                    return "missing arguments";
                }
                
                auto is_named_argument_type = []<typename T>(const T& value) -> bool {
                    return is_format_arg<typename std::decay<T>::type>::value;
                };
                
                // Values for placeholders are inserted into the resulting string in reverse order of appearance so that
                // inserting a placeholder value does not offset / affect the insertion positions of any placeholders that come before it.
                
                // The total number of placeholders is hard-capped to the maximum number able to be represented by an int.
                
                if (m_placeholder_identifiers[0].type == PlaceholderIdentifier::Type::None) {
                    // Format string contains only auto-numbered placeholders.
                    
                    // Verify argument types.
                    bool has_named_arguments = for_each(tuple, is_named_argument_type);
                    if (has_named_arguments) {
                        // Named arguments not allowed in auto-numbered format list.
                        return "name arguments not allowed";
                    }
                    
                    // For auto-numbered placeholders, there is a 1:1 correlation between placeholder and insertion point.
                    // Hence, the number of arguments provided to format(...) should be at least as many as the number of placeholders.
                    // Note: while it is valid to provide more arguments than necessary, these arguments will be ignored in the resulting string.
                    
//                    for (int i = static_cast<int>(m_insertion_points.size() - 1); i >= 0; --i) {
//                        const InsertionPoint& insertion_point = m_insertion_points[i];
//                        std::string value = get(tuple, i, [&insertion_point]<typename T>(const T& value) -> std::string {
//                            return stringify(value, insertion_point.formatting);
//                        });
//                        result.insert(insertion_point.insert_position, value);
//                    }
                }
                else {
                    // Format string contains only positional / named placeholders.
                    
                    std::size_t positional_placeholder_count = get_positional_placeholder_count();
                    
                    for (std::size_t i = 0u; i < positional_placeholder_count; ++i) {
                        if (get(tuple, i, is_named_argument_type)) {
                            return "positional arguments must come first";
                        }
                    }
                    
                    for (std::size_t i = 0u; i < get_named_placeholder_count(); ++i) {
                        if (!get(tuple, i + positional_placeholder_count, is_named_argument_type)) {
                            return "expecting named argument type";
                        }
                    }
                    
                    // Verify that all named placeholders have a corresponding argument.
                    for (std::size_t i = 0u; i < get_unique_placeholder_count(); ++i) {
                        const PlaceholderIdentifier& identifier = m_placeholder_identifiers[i];
                        if (identifier.type == PlaceholderIdentifier::Type::Name) {
                            bool found = for_each(tuple, [&identifier]<typename T>(const T& a) -> bool {
                                if constexpr (is_format_arg<typename std::decay<T>::type>::value) {
                                    return a.name == identifier.name;
                                }
                                else {
                                    return false;
                                }
                            });
                            
                            if (!found) {
                                return "named placeholder " + identifier.name + " missing argument";
                            }
                        }
                    }

                    // Format all unique placeholders once to save on computation power, since positional / named placeholders can be reused.
                    std::size_t unique_placeholder_count = get_unique_placeholder_count();
                    
                    std::vector<std::string> formatted_placeholders;
                    formatted_placeholders.reserve(unique_placeholder_count);

                    for (std::size_t i = 0u; i < unique_placeholder_count; ++i) {
                        const FormattedPlaceholder& placeholder = m_formatted_placeholders[i];
                        const PlaceholderFormatting& formatting = placeholder.formatting;
                        
                        formatted_placeholders.emplace_back(get(tuple, i, [&formatting]<typename T>(const T& value) -> std::string {
                            return stringify(value, formatting);
                        }));
                    }
                    
                    // Values for placeholders are inserted into the resulting string in reverse order of appearance so that inserting a
                    // placeholder value does not offset the insertion positions of any placeholders that come before it.

                    std::size_t previous = std::numeric_limits<std::size_t>::max();
                    
                    for (std::size_t i = 0u; i < get_total_placeholder_count(); ++i) {
                        std::size_t placeholder_index = 0u;
                        std::size_t current = 0u;
                        
                        for (std::size_t j = 0u; j < unique_placeholder_count; ++j) {
                            const FormattedPlaceholder& placeholder = m_formatted_placeholders[j];
                            
                            for (std::size_t insertion_point : placeholder.insertion_points) {
                                if (insertion_point > current && insertion_point < previous) {
                                    current = insertion_point;
                                    placeholder_index = placeholder.placeholder_index;
                                }
                            }
                        }
                        
                        result.erase(current - 1, 1);
                        result.insert(current - 1, formatted_placeholders[placeholder_index]);
                        
                        // Update maximum for the next iteration.
                        previous = current;
                    }
                }
            }
            
            return std::move(result);
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
    std::string format(const std::string& format_string, const Ts&... ts) {
        using namespace internal;
        
        Result<FormatString> result = FormatString::parse(format_string);
        if (!result.ok()) {
            throw std::runtime_error(result.what());
        }
        
        return result->format(ts...);
    }
    
}

#endif // UTILS_STRING_TPP
