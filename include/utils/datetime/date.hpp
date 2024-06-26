
#ifndef UTILS_DATE_HPP
#define UTILS_DATE_HPP

#include "utils/datetime/duration.hpp"

#include <cstdint> // std::uint8_t, std::uint16_t, std::uint32_t

namespace utils {
    namespace datetime {
    
        enum Month : std::uint8_t {
            January = 1u,
            February,
            March,
            April,
            May,
            June,
            July,
            August,
            September,
            October,
            November,
            December
        };
        
        enum class Weekday : std::uint8_t {
            Monday = 0u,
            Tuesday,
            Wednesday,
            Thursday,
            Friday,
            Saturday,
            Sunday
        };
        
        struct Date {
            static Date today();
            
            Date(); // Returns the current date, equivalent to calling Date::today().
            Date(std::uint8_t month, std::uint8_t day, std::uint32_t year); // MM-DD-YYYY
            ~Date();
            
            // Convert this timestamp to days.
            [[nodiscard]] std::uint32_t count_days() const;
            [[nodiscard]] Weekday weekday() const;
            
            // Gets the duration between the two dates (exclusive, does not count end date as an additional day).
            Duration operator-(const Date& other) const;
            
            bool operator==(const Date& other) const;
            bool operator>(const Date& other) const;
            bool operator<(const Date& other) const;
            bool operator>=(const Date& other) const;
            bool operator<=(const Date& other) const;
            
            std::uint32_t year;
            std::uint8_t month; // ranges from 1 to 12
            std::uint8_t day; // ranges from 1 to 31
        };
    
    }
}

#endif // UTILS_DATE_HPP
