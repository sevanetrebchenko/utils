
#include "utils/string/specifiers.hpp"

namespace utils {
    
    [[nodiscard]] std::string to_string(Justification justification) {
        if (justification == Justification::Left) {
            return "left";
        }
        else if (justification == Justification::Right) {
            return "right";
        }
        else {
            return "center";
        }
    }
    
    template <>
    [[nodiscard]] Justification from_string(std::string_view str) {
        if (str == "left") {
            return Justification::Left;
        }
        else if (str == "right") {
            return Justification::Right;
        }
        else if (str == "center") {
            return Justification::Center;
        }
        
        throw FormattedError("unknown justification: '{}'", str);
    }
    
    template <>
    [[nodiscard]] Sign from_string(std::string_view str) {
        if (str == "negative" || str == "negativeonly") {
            return Sign::NegativeOnly;
        }
        else if (str == "align" || str == "aligned") {
            return Sign::Aligned;
        }
        else if (str == "both") {
            return Sign::Both;
        }
        
        throw FormattedError("unknown sign: '{}'", str);
    }
    
}
