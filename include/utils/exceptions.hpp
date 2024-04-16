
#pragma once

#ifndef UTILS_EXCEPTIONS_HPP
#define UTILS_EXCEPTIONS_HPP

#include <stdexcept> // std::runtime_error

namespace utils {
    
    // Forward declarations.
    class FormatString;
    
    struct FormattedError : public std::runtime_error {
        template <typename ...Ts>
        explicit FormattedError(FormatString fmt, const Ts&... args);
        ~FormattedError() override;
    };
    
}

// Template definitions.
#include "utils/detail/exceptions.tpp"

#endif // UTILS_EXCEPTIONS_HPP
