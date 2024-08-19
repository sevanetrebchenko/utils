//
//#include "utils/format.hpp"
//#include "utils/result.hpp"
//#include "utils/string.hpp"
//#include "utils/logging/logging.hpp"
//
//#include <limits> // std::numeric_limits
//
//namespace utils {
//
//    namespace detail {
//
//        int round_up_to_multiple(int value, int multiple) {
//            if (multiple == 0) {
//                return value;
//            }
//
//            int remainder = value % multiple;
//            if (remainder == 0) {
//                return value;
//            }
//
//            return value + multiple - remainder;
//        }
//
//        template <typename T>
//        constexpr std::size_t count_digits(T num) {
//            return (num < 10) ? 1 : 1 + count_digits(num / 10);
//        }
//
//    }
//
//
//
//    // FormatString implementation
//
//    FormatString::FormatString(const FormatString& fmt) : m_format(fmt.m_format),
//                                                          m_source(fmt.m_source),
//                                                          m_identifiers(fmt.m_identifiers),
//                                                          m_specifications(fmt.m_specifications),
//                                                          m_placeholders(fmt.m_placeholders) {
//    }
//
//    FormatString::~FormatString() = default;
//
//
//
//
//
//
//
//}