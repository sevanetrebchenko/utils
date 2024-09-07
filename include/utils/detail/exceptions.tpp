
#pragma once

#ifndef EXCEPTIONS_TPP
#define EXCEPTIONS_TPP

namespace utils {
    
    template <typename ...Ts>
    FormattedError::FormattedError(std::string_view fmt, const Ts&... args) : std::runtime_error(utils::format(fmt, args...)) {
    }
    
}

#endif // EXCEPTIONS_TPP