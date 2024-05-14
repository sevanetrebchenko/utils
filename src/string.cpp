
#include "utils/string.hpp"
#include "utils/format.hpp"

#include <limits> // std::numeric_limits

namespace utils {
    
    [[nodiscard]] std::vector<std::string> split(std::string_view in, std::string_view delimiter) {
        std::vector<std::string> components { };

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
        
        std::size_t start = 0u;
        while (std::isspace(in[start])) {
            ++start;
        }
        
        std::size_t end = in.length() - 1u;
        while (end != start && std::isspace(in[end])) {
            --end;
        }
        
        return in.substr(start, end + 1);
    }
    
    bool casecmp(std::string_view first, std::string_view second) {
        std::size_t length = first.length();
        
        if (length != second.length()) {
            return false;
        }
        
        for (std::size_t i = 0u; i < length; ++i) {
            if (std::tolower(first[i]) != std::tolower(second[i])) {
                return false;
            }
        }
        return true;
    }
    
    template <typename T>
    inline std::size_t fundamental_from_string(std::string_view in, T& out) {
        // Leading whitespace is not ignored
        std::string_view str = trim(in);

        if (str.empty()) {
            throw FormattedError("");
        }

        // Only a leading '-' is permitted at the beginning
        if (str[0] == '+') {
            str = str.substr(1);
        }

        // Leading base prefixes are not recognized
        int base = 10;
        bool has_base = false;

        if (str.length() > 1) {
            if (str[0] == '0') {
                if (str[1] == 'x' || str[1] == 'X') {
                    // Hexadecimal
                    base = 16;
                    has_base = true;
                }
                else if (str[1] == 'b' || str[1] == 'B') {
                    // Binary
                    base = 2;
                    has_base = true;
                }
            }
        }

        if (has_base) {
            str = str.substr(2);
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
            // failed to convert
            logging::fatal("");
            throw FormattedError("");
        }

        if (error_code == std::errc::result_out_of_range) {
            logging::error("");
            out = std::numeric_limits<T>::max();
        }

        // Return the number of characters processed
        return ptr - start;
    }
    
    std::size_t from_string(std::string_view in, unsigned char& out) {
        return fundamental_from_string(in, out);
    }
    
    std::size_t from_string(std::string_view in, short& out) {
        return fundamental_from_string(in, out);
    }
    
    std::size_t from_string(std::string_view in, unsigned short& out) {
        return fundamental_from_string(in, out);
    }
    
    std::size_t from_string(std::string_view in, int& out) {
        return fundamental_from_string(in, out);
    }
    
    std::size_t from_string(std::string_view in, unsigned int& out) {
        return fundamental_from_string(in, out);
    }
    
    std::size_t from_string(std::string_view in, long& out) {
        return fundamental_from_string(in, out);
    }
    
    std::size_t from_string(std::string_view in, unsigned long& out) {
        return fundamental_from_string(in, out);
    }
    
    std::size_t from_string(std::string_view in, long long int& out) {
        return fundamental_from_string(in, out);
    }
    
    std::size_t from_string(std::string_view in, unsigned long long int& out) {
        return fundamental_from_string(in, out);
    }
    
    std::size_t from_string(std::string_view in, float& out) {
        return fundamental_from_string(in, out);
    }
    
    std::size_t from_string(std::string_view in, double& out) {
        return fundamental_from_string(in, out);
    }
    
    std::size_t from_string(std::string_view in, long double& out) {
        return fundamental_from_string(in, out);
    }
    
}