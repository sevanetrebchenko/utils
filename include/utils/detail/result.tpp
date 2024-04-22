
#pragma once

#ifndef UTILS_RESULT_TPP
#define UTILS_RESULT_TPP

// #include "utils/string/format.hpp"

#include <stdexcept> // std::runtime_error
#include <utility> // std::move

namespace utils {

    template <typename ...Ts>
    Response Response::NOT_OK(std::string_view fmt, const Ts& ...args) {
        return Response("asdf"); // format(fmt, args...));
    }
    
    template <typename T, typename E>
    Result<T, E>::Result() = default;
    
    template <typename T, typename E>
    Result<T, E>::~Result() = default;
    
    template <typename T, typename E>
    template <typename ...Ts>
    Result<T, E> Result<T, E>::OK(const Ts&... args) {
        using ResultType = typename std::decay<E>::type;
        
        Result<T, E> result { };
        
        if constexpr (std::is_same<ResultType, std::string>::value && sizeof...(args) > 1u) {
            // Treat first argument as format string and the rest as positional argument values.
            // result.m_result.emplace(format(args...));
        }
        else {
            // result.m_result.emplace(args...);
        }
        
        return std::move(result);
    }
    
    template <typename T, typename E>
    template <typename ...Ts>
    Result<T, E> Result<T, E>::NOT_OK(const Ts&... args) {
        using ErrorType = typename std::decay<E>::type;
        Result<T, E> result { };
        
        if constexpr (std::is_same<ErrorType, std::string>::value && sizeof...(args) > 1u) {
            // Treat first argument as format string and the rest as positional argument values.
            result.m_error.emplace(format(args...));
        }
        else {
            result.m_error.emplace(args...);
        }
        
        return std::move(result);
    }
    
    template <typename T, typename E>
    bool Result<T, E>::ok() const {
        return m_result.has_value();
    }

    template <typename T, typename E>
    T& Result<T, E>::result() {
        if (!m_result.has_value()) {
            throw std::runtime_error("result() called on Result that is not ok!");
        }
        
        return m_result.value();
    }
    
    template <typename T, typename E>
    const E& Result<T, E>::error() const {
        if (!m_error.has_value()) {
            throw std::runtime_error("error() called on Result that is ok!");
        }
        
        return m_error.value();
    }
    
}

#endif // UTILS_RESULT_TPP
