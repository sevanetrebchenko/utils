
#pragma once

#ifndef EXCEPTIONS_HPP
#define EXCEPTIONS_HPP

#include "utils/string.hpp"
#include <stdexcept> // std::runtime_error

namespace utils {
    
    struct FormattedError : public std::runtime_error {
        template <typename ...Ts>
        FormattedError(std::string_view fmt, const Ts&... args);
    };
    
}

// Template definitions
#include "utils/detail/exceptions.tpp"

#endif // EXCEPTIONS_HPP