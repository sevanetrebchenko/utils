
#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include "utils/string.hpp"
#include <stdexcept> // std::runtime_error

namespace utils {
    
    struct FormattedError : public std::runtime_error {
        template <typename ...Ts>
        FormattedError(Message error, const Ts&... args);
    };
    
}

#endif // EXCEPTIONS_HPP