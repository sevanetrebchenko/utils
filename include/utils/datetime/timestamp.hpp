
#ifndef UTILS_TIMESTAMP_HPP
#define UTILS_TIMESTAMP_HPP

#include "utils/datetime/date.hpp"
#include "utils/datetime/time.hpp"
#include "utils/datetime/duration.hpp"

#include <cstdint> // std::uint8_t, std::uint16_t, std::uint32_t

namespace utils {
    namespace datetime {
        
        struct Timestamp {
            static Timestamp now();
    
            Timestamp(); // Returns a timestamp of the current date/time, equivalent to calling Timestamp::now().
            // Automatically accounts for overflow.
            Timestamp(std::uint8_t month, std::uint8_t day, std::uint32_t year, std::uint32_t hours = 0u, std::uint32_t minutes = 0u, std::uint32_t seconds = 0u, std::uint32_t milliseconds = 0u);
            ~Timestamp();
            
            Duration operator-(const Timestamp& other) const;
            
            bool operator==(const Timestamp& other) const;
            bool operator<(const Timestamp& other) const;
            bool operator<=(const Timestamp& other) const;
            bool operator>(const Timestamp& other) const;
            bool operator>=(const Timestamp& other) const;
            
            Date date;
            Time time;
        };
        
    }
}

#endif // UTILS_TIMESTAMP_HPP
