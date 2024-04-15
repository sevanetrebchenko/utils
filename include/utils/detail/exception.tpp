
#pragma once

#ifndef UTILS_EXCEPTION_TPP
#define UTILS_EXCEPTION_TPP

#include "utils/string/format.hpp"

namespace utils {

    template <typename ...Ts>
    FormattedError::FormattedError(FormatStringWrapper fmt, const Ts&... args) : std::runtime_error(utils::format(fmt, args...)) {
    }
    
}

#endif // UTILS_EXCEPTION_TPP