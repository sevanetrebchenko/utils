
#pragma once

#ifndef UTILS_EXCEPTION_HPP
#define UTILS_EXCEPTION_HPP

#include <stdexcept> // std::runtime_error

namespace utils {
    
    class FormatStringWrapper;
    
    struct FormattedError : public std::runtime_error {
        template <typename ...Ts>
        explicit FormattedError(FormatStringWrapper fmt, const Ts&... args);
        ~FormattedError() override;
    };
    
}

// Template definitions.
#include "utils/detail/exception.tpp"

#endif // UTILS_EXCEPTION_HPP
