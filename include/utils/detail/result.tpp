
#pragma once

#ifndef RESULT_TPP
#define RESULT_TPP

#include "utils/format.hpp"
#include "utils/constexpr.hpp"

#include <utility> // std::move
#include <stdexcept> // std::runtime_error

namespace utils {

    // Response implementation
    
    template <typename ...Ts>
    Response Response::NOT_OK(const FormatString& fmt, const Ts& ...args) {
        return fmt.format(args...);
    }
    
    // Result implementation
    
    template <typename T, typename E>
    Result<T, E>::Result() = default;
    
    template <typename T, typename E>
    Result<T, E>::~Result() = default;
    
    template <typename T, typename E>
    template <typename ...Ts>
    Result<T, E> Result<T, E>::OK(const Ts&... args) {
        Result<T, E> result { };
        
        if constexpr (is_string_type<T>::value && sizeof...(args) > 1u) {
            // Treat first argument as format string and the rest as positional argument values.
            result.m_result.emplace(result.format<Ts...>(args...));
        }
        else {
            result.m_result.emplace(args...);
        }
        
        return std::move(result);
    }
    
    template <typename T, typename E>
    template <typename ...Ts>
    Result<T, E> Result<T, E>::NOT_OK(const Ts&... args) {
        Result<T, E> error { };
        
        if constexpr (is_string_type<E>::value && sizeof...(args) > 1u) {
            // Treat first argument as format string and the rest as positional argument values.
            error.m_error.emplace(error.format<Ts...>(args...));
        }
        else {
            error.m_error.emplace(args...);
        }
        
        return std::move(error);
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
    
    template <typename T, typename E>
    template <typename S, typename... Ts>
    std::string Result<T, E>::format(const S& str, const Ts& ... args) const {
        static_assert(is_string_type<S>::value);
        FormatString fmt = { str };
        return fmt.format(args...);
    }
    
    // ParsedResult implementation
    
    template <typename T, typename E>
    template <typename ...Ts>
    ParseResult<T, E> ParseResult<T, E>::OK(std::size_t offset, const Ts& ...args) {
        ParseResult<T, E> result { };
        
        result.Result<T, E>::template OK<Ts...>(args...);
        result.m_offset = offset;
        
        return std::move(result);
    }
    
    template <typename T, typename E>
    template <typename... Ts>
    ParseResult<T, E> ParseResult<T, E>::NOT_OK(std::size_t offset, const Ts& ... args) {
        ParseResult<T, E> result { };
        
        result.template Result<T, E>::template NOT_OK<Ts...>(args...);
        result.m_offset = offset;
        
        return std::move(result);
    }
    
    template <typename T, typename E>
    ParseResult<T, E>::ParseResult() : Result<T, E>(),
                                       m_offset(0) {
    }
    
    template <typename T, typename E>
    ParseResult<T, E>::~ParseResult() = default;
    
    template <typename T, typename E>
    std::size_t ParseResult<T, E>::offset() const {
        return m_offset;
    }
    
}

#endif // RESULT_TPP
