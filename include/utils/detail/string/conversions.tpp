//
//#pragma once
//
//#include "utils/string/format.hpp"
//#include "utils/constexpr.hpp"
//
//#include <charconv> // std:to_chars
//
//namespace utils {
//
//    namespace detail {
//
//    }
//
//    template <typename T, typename S>
//    std::string to_string(const std::pair<T, S>& value, const Formatting& formatting) {
//        return "[" + to_string(value.first, formatting) + ", " + to_string(value.second, formatting) + "]";
//    }
//
//    template <typename... Ts>
//    std::string to_string(const std::tuple<Ts...>& value, const Formatting& formatting) {
//        std::string result = "[ ";
//
//        // Use std::apply + fold expression to iterate over and format the elements of the tuple.
//        std::apply([&result, &formatting](const auto&&... args) {
//            ((result.append(to_string(args, formatting) + ", ")), ...);
//        }, value);
//
//        // Overwrite the trailing ", " with " ]".
//        std::size_t length = result.length();
//        result[length - 2u] = ' ';
//        result[length - 1u] = ']';
//        return std::move(result);
//    }
//
//    template <typename T>
//    std::string to_string(const T& value, const Formatting& formatting) requires is_const_iterable<T> {
//        std::string result;
//
//        result.append("[ ");
//
//        auto iter = value.begin();
//        result.append(to_string(*iter, formatting));
//
//        for (++iter; iter != value.end(); ++iter) {
//            result.append(", ");
//            result.append(to_string(*iter, formatting));
//        }
//
//        result.append(" ]");
//        return std::move(result);
//    }
//
//    template <typename T>
//    std::string to_string(const T* value, const Formatting& formatting) {
//        return value ? to_string((std::uintptr_t)(value), formatting) : to_string(nullptr, formatting);
//    }
//
//    template <typename T>
//    std::string to_string(const NamedArgument<T>& value, const Formatting& formatting) {
//        return to_string(value.value, formatting);
//    }
//
//}
