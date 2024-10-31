
#pragma once

#ifndef DATETIME_HPP
#define DATETIME_HPP

#include "utils/string.hpp"

#include <cstdint> // std::uint8_t, std::uint16_t, std::uint32_t
#include <chrono> // std::chrono

namespace utils {

    enum Month : std::uint8_t {
        January = 1,
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

    enum Weekday {
        Monday = 0,
        Tuesday,
        Wednesday,
        Thursday,
        Friday,
        Saturday,
        Sunday
    };

    struct Duration {
        template <typename T>
        Duration(const std::chrono::duration<T>& duration);
        
        Duration(std::uint32_t milliseconds = 0, std::uint32_t seconds = 0, std::uint32_t minutes = 0, std::uint32_t hours = 0, std::uint32_t days = 0); // Automatically converts on overflow
        ~Duration();

        template <typename T>
        operator std::chrono::duration<T>() const;
        
        // Returns truncated values
        std::uint32_t to_days() const;
        std::uint32_t to_hours() const;
        std::uint32_t to_minutes() const;
        std::uint32_t to_seconds() const;
        std::uint32_t to_milliseconds() const;
        
        std::uint32_t days;
        std::uint8_t hours; // ranges from 0 to 23
        std::uint8_t minutes; // ranges from 0 to 59
        std::uint8_t seconds; // ranges from 0 to 59
        std::uint16_t milliseconds; // ranges from 0 to 999
    };

    struct Date {
        static Date today();

        Date(); // Returns the current date, equivalent to calling Date::today()
        Date(Month month, std::uint8_t day, std::uint32_t year); // MM-DD-YYYY
        Date(std::uint8_t day, Month month, std::uint32_t year); // DD-MM-YYYY
        ~Date();

        // Convert this timestamp to days
        [[nodiscard]] std::uint32_t count_days() const;
        [[nodiscard]] Weekday weekday() const;

        // Gets the duration between the two dates (exclusive, does not count end date as an additional day)
        Duration operator-(const Date& other) const;

        bool operator==(const Date& other) const;
        bool operator>(const Date& other) const;
        bool operator<(const Date& other) const;
        bool operator>=(const Date& other) const;
        bool operator<=(const Date& other) const;

        std::uint32_t year;
        Month month; // ranges from 1 to 12
        std::uint8_t day; // ranges from 1 to 31
    };

    struct Time {
        static Time now();

        Time(); // Returns the current time, equivalent to calling Time::now()
        Time(std::uint32_t hours, std::uint32_t minutes, std::uint32_t seconds = 0, std::uint32_t milliseconds = 0); // Automatically converts on overflow
        ~Time();

        Duration operator-(const Time& other) const;

        bool operator==(const Time& other) const;
        bool operator>(const Time& other) const;
        bool operator<(const Time& other) const;
        bool operator>=(const Time& other) const;
        bool operator<=(const Time& other) const;
        
        std::uint32_t hour;
        std::uint8_t minute; // ranges from 0 to 59
        std::uint8_t second; // ranges from 0 to 59
        std::uint16_t millisecond; // ranges from 0 to 999
    };

    struct Timestamp {
        static Timestamp now();

        Timestamp(); // Returns a timestamp of the current date/time, equivalent to calling Timestamp::now()

        // Automatically accounts for overflow
        Timestamp(Month month, std::uint8_t day, std::uint32_t year, std::uint32_t hours = 0, std::uint32_t minutes = 0, std::uint32_t seconds = 0, std::uint32_t milliseconds = 0);
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
    
    // datetime Formatter definitions
    
    template <>
    class Formatter<Month> : public FormatterBase {
        public:
            Formatter();
            ~Formatter();
            
            void parse(const FormatSpec& spec);
            std::string format(const Month& month) const;
    
            enum class Representation : std::uint8_t {
                Decimal = 0,
                Full,
                // ex. Jan instead of January
                Abbreviated
            } representation;
        
        private:
            inline std::string to_decimal(const Month& month) const;
            inline std::string to_full_name(const Month& month) const;
            inline std::string to_abbreviated_name(const Month& month) const;
    };
    
    template <>
    class Formatter<Weekday> : public Formatter<const char*> {
        public:
            Formatter();
            ~Formatter();
            
            void parse(const FormatSpec& spec);
            std::string format(const Weekday& weekday) const;
            
            enum class Representation : std::uint8_t {
                Full = 0,
                // ex. Mon instead of Monday
                Abbreviated
            } representation;
            
        private:
            inline std::string to_full_name(const Weekday& weekday) const;
            inline std::string to_abbreviated_name(const Weekday& weekday) const;
    };

    template <>
    struct Formatter<Duration> : public FormatterBase {
        public:
            Formatter();
            ~Formatter();
            
            void parse(const FormatSpec& spec);
            std::string format(const Duration& duration) const;
            
        private:
            std::string_view m_format;
    };
    
    template <>
    class Formatter<Date> : public FormatterBase {
        public:
            Formatter();
            ~Formatter();
            
            void parse(const FormatSpec& spec);
            std::string format(const Date& date) const;
            
        private:
            std::string_view m_format;
    };
    
    template <>
    class Formatter<Time> : public FormatterBase {
        public:
            Formatter();
            ~Formatter();
            
            void parse(const FormatSpec& spec);
            std::string format(const Time& time) const;
            
        private:
            std::string_view m_format;
    };
    
    template <>
    class Formatter<Timestamp> : public FormatterBase {
        public:
            Formatter();
            ~Formatter();
            
            void parse(const FormatSpec& spec);
            std::string format(const Timestamp& timestamp) const;
            
        private:
            std::string_view m_format;
    };
    
}

// Template definitions
#include "utils/detail/datetime.tpp"

#endif // DATETIME_HPP
