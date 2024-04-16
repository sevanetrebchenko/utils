
#pragma once

#ifndef UTILS_EXCEPTIONS_TPP
#define UTILS_EXCEPTIONS_TPP

#include "utils/string/format.hpp"

namespace utils {

    template <typename ...Ts>
    FormattedError::FormattedError(FormatString fmt, const Ts&... args) : std::runtime_error(fmt.format(args...)) {
    }
    
}

#endif // UTILS_EXCEPTIONS_TPP