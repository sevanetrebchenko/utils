
#ifndef UTILS_STRING_HPP
#define UTILS_STRING_HPP

#include "utils/concepts.hpp"
#include <spdlog/fmt/std.h>
#include <string> // std::string
#include <source_location> // std::source_location
#include <thread> // std::thread
#include <vector> // std::vector
#include <unordered_map> // std::unordered_map
#include <unordered_set> // std::unordered_set
#include <utility> // std::pair
#include <tuple> // std::tuple

namespace utils {

    // Returns a vector containing the result of splitting 'in' by 'delimiter'.
    [[nodiscard]] std::vector<std::string_view> split(std::string_view in, std::string_view delimiter);

    // Trim off all whitespace characters on either side of 'in'.
    [[nodiscard]] std::string_view trim(std::string_view in);

    // std::strcasecmp requires null-terminated strings (does not work for std::string_view)
    template <String T, String U>
    [[nodiscard]] bool icasecmp(const T& first, const U& second);

    template <String T, String U>
    [[nodiscard]] bool operator==(const T& first, const U& second);

    std::size_t from_string(std::string_view in, unsigned char& out, int base = 0);
    std::size_t from_string(std::string_view in, short& out, int base = 0);
    std::size_t from_string(std::string_view in, unsigned short& out, int base = 0);
    std::size_t from_string(std::string_view in, int& out, int base = 0);
    std::size_t from_string(std::string_view in, unsigned& out, int base = 0);
    std::size_t from_string(std::string_view in, long& out, int base = 0);
    std::size_t from_string(std::string_view in, unsigned long& out, int base = 0);
    std::size_t from_string(std::string_view in, long long& out, int base = 0);
    std::size_t from_string(std::string_view in, unsigned long long& out, int base = 0);

    // Supports both scientific and fixed notation
    std::size_t from_string(std::string_view in, float& out);
    std::size_t from_string(std::string_view in, double& out);
    std::size_t from_string(std::string_view in, long double& out);

    // Allows a format placeholder to be referenced by name instead of by position, e.g. utils::format("{x}", NamedArgument("x", 1))
    template <typename T>
    struct NamedArgument {
        using type = T;

        NamedArgument(std::string_view name, const T& value);
        ~NamedArgument();

        std::string_view name;
        const T& value; // Store as a reference to avoid copying non-trivially copyable types
    };

    template <typename T>
    struct is_named_argument : std::false_type {
    };

    template <typename T>
    struct is_named_argument<NamedArgument<T>> : std::true_type {
    };

    // Captures a runtime format string together with its call site
    struct FormatString {
        FormatString(const std::string& format, std::source_location source = std::source_location::current());
        FormatString(std::string_view format, std::source_location source = std::source_location::current());
        FormatString(const char* format, std::source_location source = std::source_location::current());
        ~FormatString();

        std::string_view format;
        std::source_location source;
    };

    // Throws std::runtime_error if the format string is invalid or an argument is missing
    template <typename ...Ts>
    std::string format(const FormatString& str, const Ts&... args);

}

// fmt::formatter specializations for compound / container types
// std::pair
template <typename T, typename U>
struct fmt::formatter<std::pair<T, U>> {
    fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx);
    fmt::format_context::iterator format(const std::pair<T, U>& value, fmt::format_context& ctx) const;

    private:
        fmt::formatter<T> m_first;
        fmt::formatter<U> m_second;
};

// std::tuple
template <typename ...Ts>
struct fmt::formatter<std::tuple<Ts...>> {
    fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx);
    fmt::format_context::iterator format(const std::tuple<Ts...>& value, fmt::format_context& ctx) const;

    private:
        template <std::size_t I>
        void parse_element(std::string_view spec);

        template <std::size_t I>
        fmt::format_context::iterator format_element(const std::tuple<Ts...>& value, fmt::format_context::iterator out, fmt::format_context& ctx) const;

        std::tuple<fmt::formatter<Ts>...> m_formatters;
};

// std::vector
template <typename T>
struct fmt::formatter<std::vector<T>> {
    fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx);
    fmt::format_context::iterator format(const std::vector<T>& value, fmt::format_context& ctx) const;

    private:
        fmt::formatter<T> m_formatter;
};

// std::unordered_map
template <typename K, typename V, typename H, typename P, typename A>
struct fmt::formatter<std::unordered_map<K, V, H, P, A>> {
    fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx);
    fmt::format_context::iterator format(const std::unordered_map<K, V, H, P, A>& value, fmt::format_context& ctx) const;

    private:
        fmt::formatter<K> m_key_formatter;
        fmt::formatter<V> m_value_formatter;
};

// std::unordered_set
template <typename K, typename H, typename E, typename A>
struct fmt::formatter<std::unordered_set<K, H, E, A>> {
    fmt::format_parse_context::iterator parse(fmt::format_parse_context& ctx);
    fmt::format_context::iterator format(const std::unordered_set<K, H, E, A>& value, fmt::format_context& ctx) const;

    private:
        fmt::formatter<K> m_formatter;
};

// Template definitions
#include "utils/detail/string.tpp"

#endif // UTILS_STRING_HPP
