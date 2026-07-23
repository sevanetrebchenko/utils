
#include "utils/string.hpp"

#include <limits> // std::numeric_limits
#include <charconv> // std::from_chars, std::from_chars_result
#include <cstring> // std::strlen
#include <stdexcept> // std::runtime_error

namespace utils {

    [[nodiscard]] std::vector<std::string_view> split(std::string_view in, std::string_view delimiter) {
        std::vector<std::string_view> components { };

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

        std::size_t start = 0;
        while (std::isspace(in[start])) {
            ++start;
        }

        std::size_t end = in.length() - 1;
        while (end != start && std::isspace(in[end])) {
            --end;
        }

        return in.substr(start, end + 1);
    }

    template <typename T>
    inline std::size_t fundamental_from_string(std::string_view in, T& out, int base) {
        // Leading whitespace is not ignored
        std::string_view str = trim(in);

        if (str.empty()) {
            // throw FormattedError("");
        }

        // Only a leading '-' is permitted at the beginning
        if (str[0] == '+') {
            str = str.substr(1);
        }

        if constexpr (is_integer_type<T>::value) {
            if (base == 0) {
                // Auto-detect base 16 / 2 from a '0x' / '0b' prefix (falling back to base 10), matching strtol's own base = 0 convention
                base = 10;

                if (str.length() > 1 && str[0] == '0') {
                    if (str[1] == 'x' || str[1] == 'X') {
                        base = 16;
                        str = str.substr(2);
                    }
                    else if (str[1] == 'b' || str[1] == 'B') {
                        base = 2;
                        str = str.substr(2);
                    }
                }
            }
            // An explicitly-provided base is taken at face value - 'str' is expected to already contain raw digits with no prefix
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
            throw std::runtime_error(format("Failed to convert '{}' to a number", in));
        }

        if (error_code == std::errc::result_out_of_range) {
            out = std::numeric_limits<T>::max();
        }

        // Return the number of characters processed
        return ptr - start;
    }

    std::size_t from_string(std::string_view in, unsigned char& out, int base) {
        return fundamental_from_string(in, out, base);
    }

    std::size_t from_string(std::string_view in, short& out, int base) {
        return fundamental_from_string(in, out, base);
    }

    std::size_t from_string(std::string_view in, unsigned short& out, int base) {
        return fundamental_from_string(in, out, base);
    }

    std::size_t from_string(std::string_view in, int& out, int base) {
        return fundamental_from_string(in, out, base);
    }

    std::size_t from_string(std::string_view in, unsigned int& out, int base) {
        return fundamental_from_string(in, out, base);
    }

    std::size_t from_string(std::string_view in, long& out, int base) {
        return fundamental_from_string(in, out, base);
    }

    std::size_t from_string(std::string_view in, unsigned long& out, int base) {
        return fundamental_from_string(in, out, base);
    }

    std::size_t from_string(std::string_view in, long long int& out, int base) {
        return fundamental_from_string(in, out, base);
    }

    std::size_t from_string(std::string_view in, unsigned long long int& out, int base) {
        return fundamental_from_string(in, out, base);
    }

    std::size_t from_string(std::string_view in, float& out) {
        return fundamental_from_string(in, out, 0);
    }

    std::size_t from_string(std::string_view in, double& out) {
        return fundamental_from_string(in, out, 0);
    }

    std::size_t from_string(std::string_view in, long double& out) {
        return fundamental_from_string(in, out, 0);
    }

    FormatString::FormatString(const std::string& format, std::source_location source) : format(format),
                                                                                         source(source) {
    }

    FormatString::FormatString(std::string_view format, std::source_location source) : format(format),
                                                                                       source(source) {
    }

    FormatString::FormatString(const char* format, std::source_location source) : format(format),
                                                                                  source(source) {
    }

    FormatString::~FormatString() = default;

}
