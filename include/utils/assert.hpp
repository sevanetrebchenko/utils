
#pragma once

#include <string> // std::string
#include <source_location> // std::source_location

namespace utils {
    
    template <typename ...Ts>
    void cppassert(const std::string& expression, bool result, std::source_location source, const std::string& message, const Ts&... args);
    
    void cppassert(const std::string& expression, bool result, std::source_location source, const std::string& message);

}

#if !defined NDEBUG
    #define ASSERT(EXPRESSION, MESSAGE, ...) utils::cppassert(#EXPRESSION, EXPRESSION, std::source_location::current(), MESSAGE, ##__VA_ARGS__)
#else
    #define ASSERT(EXPRESSION, MESSAGE, ...)  \
        do { }                                \
        while(false)
#endif


// Template definitions.
#include "utils/detail/assert.tpp"
