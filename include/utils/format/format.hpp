
#ifndef FORMAT_HPP
#define FORMAT_HPP

#include "utils/concepts.hpp"

#include <cstdint> // std::uint8_t, std::size_t
#include <string> // std::string
#include <string_view> // std::string_view
#include <variant> // std::variant
#include <source_location> // std::source_location
#include <stdexcept> // std::runtime_error
#include <vector> // std::vector

namespace utils {
    
    // Forward declarations
    template <typename T, typename E>
    class ParseResult;

    template <typename E>
    class ParseResponse;
    

    
}

// Template definitions
#include "utils/detail/format/format.tpp"

#endif // FORMAT_HPP
