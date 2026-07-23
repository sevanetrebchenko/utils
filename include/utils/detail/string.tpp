
#ifndef UTILS_STRING_TPP
#define UTILS_STRING_TPP

namespace utils {

    template <String T, String U>
    [[nodiscard]] bool icasecmp(const T& first, const U& second) {
        std::string_view a = first;
        std::string_view b = second;

        if (a.length() != b.length()) {
            return false;
        }

        for (std::size_t i = 0u; i < a.length(); ++i) {
            if (std::tolower(a[i]) != std::tolower(b[i])) {
                return false;
            }
        }

        return true;
    }

    template <String T, String U>
    [[nodiscard]] bool operator==(const T& first, const U& second) {
        std::string_view a = first;
        std::string_view b = second;

        if (a.length() != b.length()) {
            return false;
        }

        for (std::size_t i = 0u; i < a.length(); ++i) {
            if (a[i] != b[i]) {
                return false;
            }
        }

        return true;
    }

    template <typename T>
    NamedArgument<T>::NamedArgument(std::string_view name, const T& value) : name(name),
                                                                              value(value) {
        static_assert(!is_named_argument<T>::value, "NamedArgument type must not be NamedArgument");
    }

    template <typename T>
    NamedArgument<T>::~NamedArgument() = default;

    namespace detail {

        // Converts a plain argument to an fmt-compatible argument unchanged, and a NamedArgument<T> to the
        // fmt::arg(...) equivalent so fmt can resolve '{name}' placeholders against it
        template <typename T>
        decltype(auto) to_fmt_argument(const T& value) {
            if constexpr (is_named_argument<T>::value) {
                return fmt::arg(value.name.data(), value.value);
            }
            else {
                return value;
            }
        }

        // Returns the Nth delimited section of a raw format spec (the text between ':' and the closing '}')
        // Used by container formatters (pair / tuple / vector / unordered_map / unordered_set) to give each element or slot its own formatting
        [[nodiscard]] constexpr std::string_view parse_format_spec_section(std::string_view spec, std::size_t index) {
            std::size_t start = 0u;
            std::size_t current = 0u;

            for (std::size_t i = 0u; i < spec.length(); ++i) {
                // A leaf format spec never contains a raw ':', so splitting on it is unambiguous
                if (spec[i] == ':') {
                    if (current == index) {
                        return spec.substr(start, i - start);
                    }

                    start = i + 1u;
                    ++current;
                }
            }

            // Last section (or the only section, if 'spec' does not contain any ':' separators)
            return spec.substr(start);
        }

    }

    template <typename ...Ts>
    std::string format(const FormatString& str, const Ts&... args) {
        try {
            return fmt::vformat(str.format, fmt::make_format_args(detail::to_fmt_argument(args)...));
        }
        catch (const fmt::format_error& error) {
            throw std::runtime_error(fmt::format("{} ({}:{})", error.what(), str.source.file_name(), str.source.line()));
        }
    }

}

// std::pair

template <typename T, typename U>
fmt::format_parse_context::iterator fmt::formatter<std::pair<T, U>>::parse(fmt::format_parse_context& ctx) {
    fmt::format_parse_context::iterator it = ctx.begin();
    while (it != ctx.end() && *it != '}') {
        ++it;
    }

    std::string_view spec(ctx.begin(), static_cast<std::size_t>(it - ctx.begin()));

    fmt::format_parse_context first_context(utils::detail::parse_format_spec_section(spec, 0u));
    m_first.parse(first_context);

    fmt::format_parse_context second_context(utils::detail::parse_format_spec_section(spec, 1u));
    m_second.parse(second_context);

    return it;
}

template <typename T, typename U>
fmt::format_context::iterator fmt::formatter<std::pair<T, U>>::format(const std::pair<T, U>& value, fmt::format_context& ctx) const {
    fmt::format_context::iterator out = ctx.out();

    out = fmt::format_to(out, "{{ ");
    out = m_first.format(value.first, ctx);
    out = fmt::format_to(out, ", ");
    out = m_second.format(value.second, ctx);
    out = fmt::format_to(out, " }}");

    return out;
}

// std::tuple

template <typename ...Ts>
template <std::size_t I>
void fmt::formatter<std::tuple<Ts...>>::parse_element(std::string_view spec) {
    if constexpr (I < sizeof...(Ts)) {
        fmt::format_parse_context element_context(utils::detail::parse_format_spec_section(spec, I));
        std::get<I>(m_formatters).parse(element_context);

        parse_element<I + 1u>(spec);
    }
}

template <typename ...Ts>
fmt::format_parse_context::iterator fmt::formatter<std::tuple<Ts...>>::parse(fmt::format_parse_context& ctx) {
    fmt::format_parse_context::iterator it = ctx.begin();
    while (it != ctx.end() && *it != '}') {
        ++it;
    }

    parse_element<0u>(std::string_view(ctx.begin(), static_cast<std::size_t>(it - ctx.begin())));
    return it;
}

template <typename ...Ts>
template <std::size_t I>
fmt::format_context::iterator fmt::formatter<std::tuple<Ts...>>::format_element(const std::tuple<Ts...>& value, fmt::format_context::iterator out, fmt::format_context& ctx) const {
    if constexpr (I < sizeof...(Ts)) {
        if constexpr (I) {
            out = fmt::format_to(out, ", ");
        }

        out = std::get<I>(m_formatters).format(std::get<I>(value), ctx);
        return format_element<I + 1u>(value, out, ctx);
    }
    else {
        return out;
    }
}

template <typename ...Ts>
fmt::format_context::iterator fmt::formatter<std::tuple<Ts...>>::format(const std::tuple<Ts...>& value, fmt::format_context& ctx) const {
    fmt::format_context::iterator out = ctx.out();

    out = fmt::format_to(out, "{{ ");
    out = format_element<0u>(value, out, ctx);
    out = fmt::format_to(out, " }}");

    return out;
}

// std::vector

template <typename T>
fmt::format_parse_context::iterator fmt::formatter<std::vector<T>>::parse(fmt::format_parse_context& ctx) {
    return m_formatter.parse(ctx);
}

template <typename T>
fmt::format_context::iterator fmt::formatter<std::vector<T>>::format(const std::vector<T>& value, fmt::format_context& ctx) const {
    fmt::format_context::iterator out = ctx.out();
    out = fmt::format_to(out, "[ ");

    for (std::size_t i = 0u; i < value.size(); ++i) {
        if (i) {
            out = fmt::format_to(out, ", ");
        }

        out = m_formatter.format(value[i], ctx);
    }

    out = fmt::format_to(out, " ]");
    return out;
}

// std::unordered_map

template <typename K, typename V, typename H, typename P, typename A>
fmt::format_parse_context::iterator fmt::formatter<std::unordered_map<K, V, H, P, A>>::parse(fmt::format_parse_context& ctx) {
    fmt::format_parse_context::iterator it = ctx.begin();
    while (it != ctx.end() && *it != '}') {
        ++it;
    }

    std::string_view spec(ctx.begin(), static_cast<std::size_t>(it - ctx.begin()));

    fmt::format_parse_context key_context(utils::detail::parse_format_spec_section(spec, 0u));
    m_key_formatter.parse(key_context);

    fmt::format_parse_context value_context(utils::detail::parse_format_spec_section(spec, 1u));
    m_value_formatter.parse(value_context);

    return it;
}

template <typename K, typename V, typename H, typename P, typename A>
fmt::format_context::iterator fmt::formatter<std::unordered_map<K, V, H, P, A>>::format(const std::unordered_map<K, V, H, P, A>& value, fmt::format_context& ctx) const {
    fmt::format_context::iterator out = ctx.out();
    out = fmt::format_to(out, "{{ ");

    bool first_element = true;
    for (const auto& [key, mapped] : value) {
        if (!first_element) {
            out = fmt::format_to(out, ", ");
        }
        first_element = false;

        out = m_key_formatter.format(key, ctx);
        out = fmt::format_to(out, ": ");
        out = m_value_formatter.format(mapped, ctx);
    }

    out = fmt::format_to(out, " }}");
    return out;
}

// std::unordered_set

template <typename K, typename H, typename E, typename A>
fmt::format_parse_context::iterator fmt::formatter<std::unordered_set<K, H, E, A>>::parse(fmt::format_parse_context& ctx) {
    return m_formatter.parse(ctx);
}

template <typename K, typename H, typename E, typename A>
fmt::format_context::iterator fmt::formatter<std::unordered_set<K, H, E, A>>::format(const std::unordered_set<K, H, E, A>& value, fmt::format_context& ctx) const {
    fmt::format_context::iterator out = ctx.out();
    out = fmt::format_to(out, "{{ ");

    bool first_element = true;
    for (const K& element : value) {
        if (!first_element) {
            out = fmt::format_to(out, ", ");
        }
        first_element = false;

        out = m_formatter.format(element, ctx);
    }

    out = fmt::format_to(out, " }}");
    return out;
}

#endif // UTILS_STRING_TPP
