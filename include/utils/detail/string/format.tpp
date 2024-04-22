
#pragma once

#ifndef UTILS_FORMAT_TPP
#define UTILS_FORMAT_TPP

#include "utils/string/conversions.hpp"
#include "utils/logging/logging.hpp"
#include "utils/tuple.hpp"
#include "utils/constexpr.hpp"
#include "utils/concepts.hpp"
#include "utils/result.hpp"

#include <queue> // std::priority_queue
#include <typeindex> // std::type_index
#include <stack> // std::stack
#include <type_traits>
#include <iostream>

namespace utils {
    
    namespace detail {
        
        template <typename T>
        struct is_named_argument : std::false_type { };
        
        template <typename T>
        struct is_named_argument<NamedArgument<T>> : std::true_type { };
     
        template <typename T>
        concept is_formattable = requires(T value, const Formatting& f) {
            { to_string(value, f) } -> std::same_as<std::string>;
        };
        
        template <typename T>
        struct is_named_argument_list : std::false_type { };
        
        template <typename ...Ts>
        struct is_named_argument_list<NamedArgumentList<Ts...>> : std::true_type { };
        
        template <typename T>
        concept is_deconstructible = requires(const T& value) {
            { deconstruct(value) };
            is_named_argument_list<decltype(deconstruct(value))>::value;
        };
        
        // Global format overrides
        extern std::unordered_map<std::type_index, std::stack<std::string>> format_overrides;
        
        // Empty format overrides are allowed, so there needs to be a different way to distinguish format overrides that don't exist.
        template <typename T>
        std::optional<std::string_view> get_format_override() {
            using Type = std::decay<T>::type;
            auto iter = format_overrides.find(typeid(Type));
            
            if (iter != format_overrides.end()) {
                std::stack<std::string>& overrides = iter->second;
                if (!overrides.empty()) {
                    return overrides.top();
                }
            }
            
            return { };
        }
        
    }
    
    template <typename ...Ts>
    std::string FormatString::format(const Ts& ...args) const {
        if constexpr (sizeof...(Ts) == 0u) {
            return m_result;
        }
        else {
            std::string result = m_result;
            
            if (!m_placeholder_identifiers.empty()) {
                auto tuple = std::make_tuple(args...);
        
                if (m_placeholder_identifiers[0].type == Identifier::Type::Auto) {
                    // For unstructured format strings, there is a 1:1 correlation between a placeholder value and its insertion point.
                    // Hence, the number of arguments provided to format(...) should be at least as many as the number of placeholders.
                    // Note: while it is valid to provide more arguments than necessary, these arguments will be ignored.
                    std::size_t placeholder_count = get_placeholder_count();
                    if (placeholder_count > sizeof...(args)) {
                        // Not enough arguments provided to format(...);
                        throw FormattedError("expecting {} arguments, but received {}", placeholder_count, sizeof...(args));
                    }
                }
                else {
                    // Format string should only contain positional / named placeholders.
                    std::size_t required_positional_placeholder_count = get_positional_placeholder_count();
                    std::size_t required_named_placeholder_count = get_named_placeholder_count();
        
                    if (required_positional_placeholder_count + required_named_placeholder_count > sizeof...(args)) {
                        // Not enough arguments provided to format(...);
                        throw FormattedError("expecting {} arguments, but received {}", required_positional_placeholder_count + required_named_placeholder_count, sizeof...(args));
                    }
        
                    // All positional placeholders must come before any named placeholders.
                    for (std::size_t i = 0u; i < required_positional_placeholder_count; ++i) {
                        auto is_structured_type = []<typename T>(const T& value) -> std::string_view {
                            if constexpr (detail::is_named_argument<T>::value) {
                                return value.name;
                            }
                            return ""; // Empty names are not allowed.
                        };
                        
                        std::string_view name = runtime_get(tuple, i, is_structured_type);
                        if (!name.empty()) {
                            throw FormattedError("expecting value for positional placeholder {}, but received value for named placeholder '{}' - all positional placeholders values must come before any named placeholder values", i, name);
                        }
                    }
        
                    // Check positional placeholder indices and warn on arguments not being used due to gaps in the positional placeholder values.
                    std::vector<bool> is_placeholder_used { };
                    is_placeholder_used.resize(required_positional_placeholder_count, false);
                    
                    for (const Identifier& identifier : m_placeholder_identifiers) {
                        if (identifier.type == Identifier::Type::Position) {
                            is_placeholder_used[identifier.position] = true;
                        }
                    }
                    
                    for (std::size_t i = 0u; i < required_positional_placeholder_count; ++i) {
                        if (!is_placeholder_used[i]) {
                            logging::warning("positional placeholder '{}' is never referenced (value is never used)", i);
                        }
                    }
                    
                    // Verify that all named placeholders have values provided
                    for (const Identifier& identifier : m_placeholder_identifiers) {
                        if (identifier.type == Identifier::Type::Name) {
                            std::string_view name = identifier.name;
                            bool found = false;
                            
                            for (std::size_t i = 0u; i < sizeof...(args); ++i) {
                                found |= runtime_get(tuple, i, [name]<typename T>(const T& value) -> bool {
                                    if constexpr (detail::is_named_argument<T>::value) {
                                        return value.name == name;
                                    }
                                    return false;
                                });
                            }
                            
                            if (!found) {
                                throw FormattedError("missing value for named placeholder '{}'", name);
                            }
                        }
                    }
                }
                
                // Format all unique placeholders once to save on computation power.
                // This is only really applicable for positional / named placeholder values, since these can be referenced multiple times
                // in the format string, but this logic is near identical for auto-numbered placeholder values.
                std::size_t unique_placeholder_count = get_placeholder_count();
        
                std::vector<std::string> formatted_placeholders;
                formatted_placeholders.reserve(unique_placeholder_count);
        
                for (std::size_t i = 0u; i < unique_placeholder_count; ++i) {
                    const FormattedPlaceholder& placeholder = m_formatted_placeholders[i];
                    formatted_placeholders.emplace_back(runtime_get(tuple, placeholder.identifier_index, [&placeholder, this] <typename T>(const T& value) -> std::string {
                        using Type = std::decay<T>::type;
                        
                        if constexpr (detail::is_deconstructible<T>) {
                            std::optional<std::string_view> fmt = detail::get_format_override<Type>();
                            if (fmt.has_value()) {
                                return std::apply([fmt](const auto&... args) {
                                    return utils::format(fmt, args...);
                                }, to_placeholder_list(value).to_tuple());
                            }
                            
                            // Types may have custom to_placeholder_list conversion functions defined but no format override
                            // In this case, we just want to format it normally using to_string
                        }
                        
                        if constexpr (detail::is_formattable<Type>) {
                            return to_string(value, placeholder.formatting);
                        }
                        else {
                            // TODO: better warning message
                            logging::warning("ignoring format specifiers for placeholder {} (originally called from {})", placeholder.identifier_index, m_source);
                            return to_string(value);
                        }
                    }));
                }
        
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
                        insertion_points.emplace(i, position);
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
        
            return std::move(result);
        }
    }
    
    template <typename T>
    NamedArgument<T>::NamedArgument(std::string name, const T& value) : name(std::move(name)),
                                                                        value(value) {
    }

    template <typename T>
    NamedArgument<T>::~NamedArgument() = default;
    
    template <typename... Ts>
    NamedArgumentList<Ts...>::NamedArgumentList(NamedArgument<Ts>&&... args) : m_tuple(std::move(args)...) {
    }
    
    template <typename... Ts>
    NamedArgumentList<Ts...>::~NamedArgumentList() = default;
    
    template <typename... Ts>
    const std::tuple<NamedArgument<Ts>...>& NamedArgumentList<Ts...>::to_tuple() const {
        return m_tuple;
    }
    
    template <typename... Ts>
    template <typename T>
    const T& NamedArgumentList<Ts...>::get(std::string_view name) const {
        const T* out = nullptr;
    
        get(name, [&out]<typename U>(const NamedArgument<U>& value) -> void {
            if constexpr (std::is_same<typename std::decay<T>::type, typename std::decay<U>::type>::value) {
                out = &value.value;
            }
        });
        
        if (!out) {
            throw FormattedError("");
        }
        
        return *out;
    }
    
    template <typename... Ts>
    template <typename T>
    T& NamedArgumentList<Ts...>::get(std::string_view name) {
        T* out = nullptr;
    
        get(name, [&out]<typename U>(NamedArgument<U>& value) -> void {
            if constexpr (std::is_same<typename std::decay<T>::type, typename std::decay<U>::type>::value) {
                out = &value.value;
            }
        });
        
        if (!out) {
            throw FormattedError("");
        }
        
        return *out;
    }
    
    template <typename... Ts>
    template <typename Fn, std::size_t Index>
    void NamedArgumentList<Ts...>::get(std::string_view name, const Fn& fn) {
        if constexpr (Index < sizeof...(Ts)) {
            auto current = std::get<Index>(m_tuple);
            if (current.name == name) {
                fn(current);
            }
            else {
                get<Fn, Index + 1>(name, fn);
            }
        }
    }
    
    template <typename ...Ts>
    std::string format(const FormatString& fmt, const Ts&... args) {
        return fmt.format(args...);
    }
    
    template <typename T>
    void push_format_override(std::string fmt) {
        using Type = std::decay<T>::type;
        detail::format_overrides[typeid(Type)].push(std::move(fmt));
    }
    
    template <typename T>
    void pop_format_override() {
        using Type = std::decay<T>::type;
        
        auto iter = detail::format_overrides.find(typeid(Type));
        if (iter != detail::format_overrides.end()) {
            std::stack<std::string>& overrides = iter->second;
            overrides.pop();
        }
    }
    
}

#endif // UTILS_FORMAT_TPP