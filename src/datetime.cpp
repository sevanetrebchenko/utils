
#include "utils/datetime.hpp"
#include "utils/string.hpp"

#include <chrono> // std::chrono

namespace utils {

    bool is_leap_year(unsigned year) {
        return (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
    }
    
    std::uint8_t num_days_in_month(Month month, std::uint32_t year) {
        static const int num_days_in_month[] = { 31, is_leap_year(year) ? 29 : 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        return num_days_in_month[month - 1];
    }
    
    void validate_date(std::uint8_t day, Month month, std::uint32_t year) {
        if (month < 1 || month > 12) {
            throw std::runtime_error("invalid month - month must be in range [1, 12]");
        }

        std::uint8_t num_days = num_days_in_month(month, year);
        
        if (day < 1 || day > num_days) {
            throw std::runtime_error(utils::format("invalid day - must be in range [1, {}]", num_days));
        }
    }
    

    
    Duration::Duration(std::uint32_t ms, std::uint32_t s, std::uint32_t m, std::uint32_t h, std::uint32_t d) {
        s += ms / 1000;
        milliseconds = ms % 1000;
        
        m += s / 60;
        seconds = s % 60;
        
        h += m / 60;
        minutes = m % 60;
        
        d += h / 24;
        hours = h % 24;
        
        days = d;
    }
    
    std::uint32_t Duration::to_days() const {
        return days;
    }
    
    std::uint32_t Duration::to_hours() const {
        return days * 24 + hours;
    }
    
    std::uint32_t Duration::to_minutes() const {
        return (days * 24 * 60) + (hours * 60) + minutes;
    }
    
    std::uint32_t Duration::to_seconds() const {
        return (days * 24 * 60 * 60) + (hours * 60 * 60) + (minutes * 60) + seconds;
    }
    
    std::uint32_t Duration::to_milliseconds() const {
        return (days * 24 * 60 * 60 * 1000) + (hours * 60 * 60 * 1000) + (minutes * 60 * 1000) + (seconds * 1000) + milliseconds;
    }
    
    Duration::~Duration() = default;
    
    Date Date::today() {
        return { };
    }
    
    Date::Date() {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        std::time_t time_point = std::chrono::system_clock::to_time_t(now);
        std::tm time { }; // UTC
        gmtime_s(&time, &time_point);
        
        day = (std::uint8_t) time.tm_mday;
        month = (Month) (time.tm_mon + 1); // Convert from range [0, 11] to [1, 12]
        year = (std::uint32_t) time.tm_year + 1900;
    }
    
    Date::Date(Month month, std::uint8_t day, std::uint32_t year) : year(year),
                                                                    month(month),
                                                                    day(day) {
        validate_date(day, month, year);
    }
    
    Date::Date(std::uint8_t day, Month month, std::uint32_t year) : year(year),
                                                                    month(month),
                                                                    day(day) {
        validate_date(day, month, year);
    }
    
    Date::~Date() = default;
    
    std::uint32_t Date::count_days() const {
        unsigned count = 0;
        
        for (unsigned y = 1; y < year; ++y) { // Years start at 1
            count += is_leap_year(y) ? 366 : 365;
        }
        
        for (unsigned m = 1; m < month; ++m) {
            count += num_days_in_month((Month) m, year);
        }
        
        return count + day;
    }
    
    Weekday Date::weekday() const {
        // Calculating the day of the week: https://artofmemory.com/blog/how-to-calculate-the-day-of-the-week/
        unsigned yy = year % 100;
        unsigned yc = (yy + (yy / 4)) % 7;
        static const int mcs[12] = { 0, 3, 3, 6, 1, 4, 6, 2, 5, 0, 3, 5 };
        unsigned mc = mcs[month - 1];
        
        static const int ccs[4] = { 6, 4, 2, 0 };
        unsigned cc = ccs[static_cast<unsigned>(year / 100) % 4u];
        
        unsigned lyc = is_leap_year(year) && (month == 1 || month == 2) ? 1 : 0u;
        return (Weekday) (((yc + mc + cc + day - lyc) % 7 + 6) % 7);
    }
    
    Duration Date::operator-(const Date& other) const {
        std::uint32_t d1 = count_days();
        std::uint32_t d2 = other.count_days();
        
        // Duration is absolute
        return { 0, 0, 0, 0, d1 > d2 ? d1 - d2 - 1 : d2 - d1 - 1 };
    }
    
    bool Date::operator==(const Date& other) const {
        return year == other.year && month == other.month && day == other.day;
    }
    
    bool Date::operator>(const Date& other) const {
        return year > other.year ||
               (year == other.year && month > other.month) ||
               (year == other.year && month == other.month && day > other.day);
    }
    
    bool Date::operator<(const Date& other) const {
        return year < other.year ||
               (year == other.year && month < other.month) ||
               (year == other.year && month == other.month && day < other.day);
    }
    
    bool Date::operator>=(const Date& other) const {
        return year >= other.year ||
               (year == other.year && month >= other.month) ||
               (year == other.year && month == other.month && day >= other.day);
    }
    
    bool Date::operator<=(const Date& other) const {
        return year <= other.year ||
               (year == other.year && month <= other.month) ||
               (year == other.year && month == other.month && day <= other.day);
    }
    
    Time Time::now() {
        return { };
    }
    
    Time::Time() {
        std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
        std::time_t time_point = std::chrono::system_clock::to_time_t(now);
        std::tm time { }; // UTC
        gmtime_s(&time, &time_point);
        
        hour = (std::uint32_t) time.tm_hour;
        minute = (std::uint8_t) time.tm_min;
        second = (std::uint8_t) time.tm_sec;
        millisecond = (std::uint16_t) (std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch() % std::chrono::seconds(1)).count());
    }
    
    Time::Time(std::uint32_t hour, std::uint32_t minute, std::uint32_t second, std::uint32_t millisecond) : hour(hour),
                                                                                                            minute(minute),
                                                                                                            second(second),
                                                                                                            millisecond(millisecond)
                                                                                                            {
        if (hour > 23u) {
            throw std::runtime_error("invalid hour value - must be in range [0, 23]");
        }
        
        if (minute > 59u) {
            throw std::runtime_error("invalid minute value - must be in range [0, 59]");
        }
        
        if (second > 59u) {
            throw std::runtime_error("invalid second value - must be in range [0, 59]");
        }
        
        if (millisecond > 999u) {
            throw std::runtime_error("invalid millisecond value - must be in range [0, 999]");
        }
    }
    
    Time::~Time() = default;
    
    Duration Time::operator-(const Time& other) const {
        if (*this == other) {
            return { };
        }
        
        Time start, end;
        if (*this > other) {
            start = other;
            end = *this;
        }
        else {
            start = *this;
            end = other;
        }
        
        unsigned h = end.hour - start.hour;
        unsigned m;
        unsigned s;
        unsigned ms;
        
        if (end.minute >= start.minute) {
            m = end.minute - start.minute;
        }
        else {
            h -= 1;
            m = (60u + end.minute) - start.minute;
        }
        
        if (end.second >= start.second) {
            s = end.second - start.second;
        }
        else {
            m -= 1;
            s = (60 + end.second) - start.second;
        }
        
        if (end.millisecond >= start.millisecond) {
            ms = end.millisecond - start.millisecond;
        }
        else {
            s -= 1;
            ms = (1000 + end.millisecond) - start.millisecond;
        }
        
        return { ms, s, m, h, 0 };
    }
    
    bool Time::operator==(const Time& other) const {
        return hour == other.hour && minute == other.minute && second == other.second && millisecond == other.millisecond;
    }
    
    bool Time::operator>(const Time& other) const {
        return hour > other.hour ||
               (hour == other.hour && minute > other.minute) ||
               (hour == other.hour && minute == other.minute && second > other.second) ||
               (hour == other.hour && minute == other.minute && second == other.second && millisecond > other.millisecond);
    }
    
    bool Time::operator<(const Time& other) const {
        return hour < other.hour ||
               (hour == other.hour && minute < other.minute) ||
               (hour == other.hour && minute == other.minute && second < other.second) ||
               (hour == other.hour && minute == other.minute && second == other.second && millisecond < other.millisecond);
    }
    
    bool Time::operator>=(const Time& other) const {
        return hour >= other.hour ||
               (hour == other.hour && minute >= other.minute) ||
               (hour == other.hour && minute == other.minute && second >= other.second) ||
               (hour == other.hour && minute == other.minute && second == other.second && millisecond >= other.millisecond);
    }
    
    bool Time::operator<=(const Time& other) const {
        return hour <= other.hour ||
               (hour == other.hour && minute <= other.minute) ||
               (hour == other.hour && minute == other.minute && second <= other.second) ||
               (hour == other.hour && minute == other.minute && second == other.second && millisecond <= other.millisecond);
    }
    
    Timestamp Timestamp::now() {
        return { };
    }
    
    Timestamp::Timestamp() : date(),
                             time() {
    }
    
    Timestamp::Timestamp(Month month, std::uint8_t day, std::uint32_t year, std::uint32_t h, std::uint32_t m, std::uint32_t s, std::uint32_t ms) {
        // Handle overflow for time
        s += ms / 1000;
        std::uint32_t millisecond = ms % 1000;
        
        m += s / 60;
        std::uint8_t second = s % 60;
        
        h += m / 60;
        std::uint8_t minute = m % 60;
        
        std::uint32_t d = day + h / 24;
        std::uint8_t hour = h % 24;

        time = Time(hour, minute, second, millisecond);
        
        // Handle overflow for date
        bool leap = is_leap_year(year);
        
        while (d > num_days_in_month(month, year)) {
            d -= num_days_in_month(month, year);
            if (month == December) {
                month = January;
                ++year;
            }
            else {
                month = (Month) (month + 1);
            }
        }
        
        date = Date(d, month, year);
    }
    
    Timestamp::~Timestamp() = default;
    
    Duration Timestamp::operator-(const Timestamp& other) const {
        if (*this == other) {
            return { };
        }
        
        Timestamp start, end;
        
        if (*this > other) {
            start = other;
            end = *this;
        }
        else {
            start = *this;
            end = other;
        }
        
        unsigned days = end.date.count_days() - start.date.count_days();
        unsigned hours;
        unsigned minutes;
        unsigned seconds;
        unsigned milliseconds;
        
        if (end.time.hour >= start.time.hour) {
            hours = end.time.hour - start.time.hour;
        }
        else {
            days -= 1;
            hours = (24 + end.time.hour) - start.time.hour;
        }
        
        if (end.time.minute >= start.time.minute) {
            minutes = end.time.minute - start.time.minute;
        }
        else {
            hours -= 1;
            minutes = (60 + end.time.minute) - start.time.minute;
        }
        
        if (end.time.second >= start.time.second) {
            seconds = end.time.second - start.time.second;
        }
        else {
            minutes -= 1;
            seconds = (60 + end.time.second) - start.time.second;
        }
        
        if (end.time.millisecond >= start.time.millisecond) {
            milliseconds = end.time.millisecond - start.time.millisecond;
        }
        else {
            seconds -= 1;
            milliseconds = (1000 + end.time.millisecond) - start.time.millisecond;
        }
        
        return { milliseconds, seconds, minutes, hours, days };
    }
    
    bool Timestamp::operator==(const Timestamp& other) const {
        return date == other.date && time == other.time;
    }
    
    bool Timestamp::operator<(const Timestamp& other) const {
        return date < other.date && time < other.time;
    }
    
    bool Timestamp::operator<=(const Timestamp& other) const {
        return date <= other.date && time <= other.time;
    }
    
    bool Timestamp::operator>(const Timestamp& other) const {
        return date > other.date && time > other.time;
    }
    
    bool Timestamp::operator>=(const Timestamp& other) const {
        return date >= other.date && time >= other.time;
    }
    
    // datetime Formatter implementations
    
    Formatter<Month>::Formatter() : FormatterBase(),
                                    representation(Representation::Decimal) {
    }

    Formatter<Month>::~Formatter() {
    }
    
    void Formatter<Month>::parse(const utils::FormatSpec& spec) {
        ASSERT(spec.type() == FormatSpec::Type::SpecifierList, "format spec for Month types must be a specifier list");
        
        if (spec.has_specifier("representation")) {
            std::string_view value = spec.get_specifier("representation");
            if (icasecmp(value, "decimal")) {
                representation = Representation::Decimal;
            }
            else if (icasecmp(value, "full")) {
                representation = Representation::Full;
            }
            else if (icasecmp(value, "abbreviated")) {
                representation = Representation::Abbreviated;
            }
        }
        
        FormatterBase::parse(spec);
    }
    
    std::string Formatter<Month>::format(const Month& month) const {
        if (representation == Representation::Decimal) {
            return to_decimal(month);
        }
        else if (representation == Representation::Full) {
            return to_full_name(month);
        }
        else { // if (representation == Representation::Abbreviated) {
            return to_abbreviated_name(month);
        }
    }
    
    std::string Formatter<Month>::to_decimal(const Month& month) const {
        if (month == January) {
            return std::move(FormatterBase::format("1", 1));
        }
        else if (month == February) {
            return std::move(FormatterBase::format("2", 1));
        }
        else if (month == March) {
            return std::move(FormatterBase::format("3", 1));
        }
        else if (month == April) {
            return std::move(FormatterBase::format("4", 1));
        }
        else if (month == May) {
            return std::move(FormatterBase::format("5", 1));
        }
        else if (month == June) {
            return std::move(FormatterBase::format("6", 1));
        }
        else if (month == July) {
            return std::move(FormatterBase::format("7", 1));
        }
        else if (month == August) {
            return std::move(FormatterBase::format("8", 1));
        }
        else if (month == September) {
            return std::move(FormatterBase::format("9", 1));
        }
        else if (month == October) {
            return std::move(FormatterBase::format("10", 2));
        }
        else if (month == November) {
            return std::move(FormatterBase::format("11", 2));
        }
        else { // if (month == December) {
            return std::move(FormatterBase::format("12", 2));
        }
    }
    
    std::string Formatter<Month>::to_full_name(const Month& month) const {
        if (month == January) {
            return std::move(FormatterBase::format("January", 7));
        }
        else if (month == February) {
            return std::move(FormatterBase::format("February", 8));
        }
        else if (month == March) {
            return std::move(FormatterBase::format("March", 5));
        }
        else if (month == April) {
            return std::move(FormatterBase::format("April", 5));
        }
        else if (month == May) {
            return std::move(FormatterBase::format("May", 3));
        }
        else if (month == June) {
            return std::move(FormatterBase::format("June", 4));
        }
        else if (month == July) {
            return std::move(FormatterBase::format("July", 4));
        }
        else if (month == August) {
            return std::move(FormatterBase::format("August", 6));
        }
        else if (month == September) {
            return std::move(FormatterBase::format("September", 9));
        }
        else if (month == October) {
            return std::move(FormatterBase::format("October", 7));
        }
        else if (month == November) {
            return std::move(FormatterBase::format("November", 8));
        }
        else { // if (month == December) {
            return std::move(FormatterBase::format("December", 8));
        }
    }
    
    std::string Formatter<Month>::to_abbreviated_name(const Month& month) const {
        const char* name;
        
        if (month == January) {
            name = "Jan";
        }
        else if (month == February) {
            name = "Feb";
        }
        else if (month == March) {
            name = "Mar";
        }
        else if (month == April) {
            name = "Apr";
        }
        else if (month == May) {
            name = "May";
        }
        else if (month == June) {
            name = "Jun";
        }
        else if (month == July) {
            name = "Jul";
        }
        else if (month == August) {
            name = "Aug";
        }
        else if (month == September) {
            name = "Sep";
        }
        else if (month == October) {
            name = "Oct";
        }
        else if (month == November) {
            name = "Nov";
        }
        else { // if (month == December) {
            name = "Dec";
        }
        
        return std::move(FormatterBase::format(name, 3));
    }
    
    Formatter<Weekday>::Formatter() : Formatter<const char*>(),
                                      representation(Representation::Full) {
    }

    Formatter<Weekday>::~Formatter() {
    }

    void Formatter<Weekday>::parse(const utils::FormatSpec& spec) {
        ASSERT(spec.type() == FormatSpec::Type::SpecifierList, "format spec for Weekday types must be a specifier list");
        
        if (spec.has_specifier("representation")) {
            std::string_view value = spec.get_specifier("representation");
            if (icasecmp(value, "full")) {
                representation = Representation::Full;
            }
            else if (icasecmp(value, "abbreviated")) {
                representation = Representation::Abbreviated;
            }
        }
        
        Formatter<const char*>::parse(spec);
    }
    
    std::string Formatter<Weekday>::format(const Weekday& weekday) const {
        if (representation == Representation::Full) {
            return to_full_name(weekday);
        }
        else { // if (representation == Representation::Abbreviated) {
            return to_abbreviated_name(weekday);
        }
    }
    
    std::string Formatter<Weekday>::to_full_name(const Weekday& weekday) const {
        if (weekday == Monday) {
            return std::move(Formatter<const char*>::format("Monday", 6));
        }
        else if (weekday == Tuesday) {
            return std::move(Formatter<const char*>::format("Tuesday", 7));
        }
        else if (weekday == Wednesday) {
            return std::move(Formatter<const char*>::format("Wednesday", 9));
        }
        else if (weekday == Thursday) {
            return std::move(Formatter<const char*>::format("Thursday", 8));
        }
        else if (weekday == Friday) {
            return std::move(Formatter<const char*>::format("Friday", 6));
        }
        else if (weekday == Saturday) {
            return std::move(Formatter<const char*>::format("Saturday", 8));
        }
        else { // if (weekday == Sunday) {
            return std::move(Formatter<const char*>::format("Sunday", 6));
        }
    }
    
    std::string Formatter<Weekday>::to_abbreviated_name(const Weekday& weekday) const {
        const char* name;

        if (weekday == Monday) {
            name = "Mon";
        }
        else if (weekday == Tuesday) {
            name = "Tue";
        }
        else if (weekday == Wednesday) {
            name = "Wed";
        }
        else if (weekday == Thursday) {
            name = "Thu";
        }
        else if (weekday == Friday) {
            name = "Fri";
        }
        else if (weekday == Saturday) {
            name = "Sat";
        }
        else { // if (weekday == Sunday) {
            name = "Sun";
        }

        return std::move(Formatter<const char*>::format(name, 3));
    }
    
    Formatter<Duration>::Formatter() : m_format() {
    }
    
    Formatter<Duration>::~Formatter() {
    }

    void Formatter<Duration>::parse(const FormatSpec& spec) {
        ASSERT(spec.type() == FormatSpec::Type::SpecifierList, "format spec for Duration types must be a specifier list");
        
        if (spec.has_specifier("format")) {
            m_format = spec.get_specifier("format");
        }
        
        FormatterBase::parse(spec);
    }
    
    std::string Formatter<Duration>::format(const Duration& duration) const {
        return FormatterBase::format(utils::format(m_format, NamedArgument("d", duration.days),
                                                             NamedArgument("h", duration.hours),
                                                             NamedArgument("m", duration.minutes),
                                                             NamedArgument("s", duration.seconds)));
                                                             NamedArgument("ms", duration.milliseconds),
                                                             NamedArgument("days", duration.to_days()),
                                                             NamedArgument("hours", duration.to_hours()),
                                                             NamedArgument("minutes", duration.to_minutes()),
                                                             NamedArgument("seconds", duration.to_seconds()),
                                                             NamedArgument("milliseconds", duration.to_milliseconds());
    }
    
    Formatter<Date>::Formatter() : m_format("{month:width=[2],fill=[0],justification=[right]}/{day:width=[2],fill=[0],justification=[right]}/{year}") {
    }
    
    Formatter<Date>::~Formatter() = default;
    
    void Formatter<Date>::parse(const FormatSpec& spec) {
        ASSERT(spec.type() == FormatSpec::Type::SpecifierList, "format spec for Date types must be a specifier list");
        
        if (spec.has_specifier("format")) {
            m_format = spec.get_specifier("format");
        }
        FormatterBase::parse(spec);
    }
    
    std::string Formatter<Date>::format(const Date& date) const {
        return FormatterBase::format(utils::format(m_format, NamedArgument("day", date.day),
                                                             NamedArgument("weekday", date.weekday()),
                                                             NamedArgument("month", date.month),
                                                             NamedArgument("year", date.year)));
    }
    
    Formatter<Time>::Formatter() : FormatterBase(),
                                   m_format("{hour:width=[2],fill=[0],justification=[right]}:{minute:width=[2],fill=[0],justification=[right]}:{second:width=[2],fill=[0],justification=[right]}:{millisecond:width=[4],fill=[0],justification=[right]}") {
    }

    Formatter<Time>::~Formatter() {
    }

    void Formatter<Time>::parse(const FormatSpec& spec) {
        ASSERT(spec.type() == FormatSpec::Type::SpecifierList, "format spec for Time types must be a specifier list");
        
        if (spec.has_specifier("format")) {
            m_format = spec.get_specifier("format");
        }
        
        FormatterBase::parse(spec);
    }
    
    std::string Formatter<Time>::format(const Time& time) const {
        return FormatterBase::format(utils::format(m_format, NamedArgument("hour", time.hour),
                                                             NamedArgument("minute", time.minute),
                                                             NamedArgument("second", time.second),
                                                             NamedArgument("millisecond", time.millisecond)));
    }
    
    Formatter<Timestamp>::Formatter() : FormatterBase(),
                                        m_format("{date} {time}") {
    }
    
    Formatter<Timestamp>::~Formatter() {
    }

    void Formatter<Timestamp>::parse(const FormatSpec& spec) {
        ASSERT(spec.type() == FormatSpec::Type::SpecifierList, "format spec for Timestamp types must be a specifier list");
        
        if (spec.has_specifier("format")) {
            m_format = spec.get_specifier("format");
        }
        
        FormatterBase::parse(spec);
    }
    
    std::string Formatter<Timestamp>::format(const Timestamp& timestamp) const {
        return FormatterBase::format(utils::format(m_format, NamedArgument("date", timestamp.date),
                                                             NamedArgument("day", timestamp.date.day),
                                                             NamedArgument("weekday", timestamp.date.weekday()),
                                                             NamedArgument("month", timestamp.date.month),
                                                             NamedArgument("year", timestamp.date.year),
                                                             NamedArgument("time", timestamp.time),
                                                             NamedArgument("hour", timestamp.time.hour),
                                                             NamedArgument("minute", timestamp.time.minute),
                                                             NamedArgument("second", timestamp.time.second),
                                                             NamedArgument("millisecond", timestamp.time.millisecond)));
    }
    
}