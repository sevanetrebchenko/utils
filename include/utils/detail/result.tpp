
#pragma once

#ifndef RESULT_TPP
#define RESULT_TPP

#include "utils/constexpr.hpp"
#include "utils/string.hpp"

#include <utility> // std::move
#include <stdexcept> // std::runtime_error
#include <regex>

namespace utils {
    
    // Response implementation
    
    template <typename E>
    Response<E>::Response() = default;
    
    template <typename E>
    Response<E> Response<E>::OK() {
        return { };
    }
    
    template <typename E>
    template <typename ...Ts>
    Response<E> Response<E>::NOT_OK(Ts&& ...args) {
        Response<E> e { };
        e.m_error = { std::move(args)... };
        return std::move(e);
    }
    
    template <typename E>
    bool Response<E>::ok() const {
        return !m_error.has_value();
    }
    
    template <typename E>
    const E& Response<E>::error() const {
        if (!m_error.has_value()) {
            throw std::runtime_error("error() called on Result that is ok!");
        }
        
        return m_error.value();
    }
    
    // Result implementation
    
    template <typename T, typename E>
    Result<T, E>::Result() = default;
    
    template <typename T, typename E>
    template <typename ...Ts>
    Result<T, E> Result<T, E>::OK(Ts&&... args) {
        Result<T, E> r { };
        r.m_result = { std::move(args)... };
        return std::move(r);
    }
    
    template <typename T, typename E>
    template <typename ...Ts>
    Result<T, E> Result<T, E>::NOT_OK(Ts&&... args) {
        Result<T, E> e { };
        e.m_error = { std::move(args)... };
        return std::move(e);
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
    
    // ParseResponse implementation
    
    template <typename E>
    ParseResponse<E>::ParseResponse() : Response<E>(),
                                        m_offset(0u) {
    }
    
    template <typename E>
    ParseResponse<E> ParseResponse<E>::OK(std::size_t num_characters_parsed) {
        ParseResponse<E> r { };
        r.m_offset = num_characters_parsed;
        return std::move(r);
    }
    
    template <typename E>
    template <typename ...Ts>
    ParseResponse<E> ParseResponse<E>::NOT_OK(std::size_t error_position, Ts&&... args) {
        ParseResponse<E> e { };
        e.m_offset = error_position;
        e.Response<E>::m_error = { std::move(args)... };
        return std::move(e);
    }
    
    template <typename E>
    std::size_t ParseResponse<E>::offset() const {
        return m_offset;
    }
    
    // ParsedResult implementation
    
    template <typename T, typename E>
    ParseResult<T, E>::ParseResult() : Result<T, E>(),
                                       m_offset(0u) {
    }
    
    template <typename T, typename E>
    template <typename ...Ts>
    ParseResult<T, E> ParseResult<T, E>::OK(std::size_t num_characters_parsed, Ts&& ...args) {
        ParseResult<T, E> r { };
        r.m_offset = num_characters_parsed;
        r.Result<T, E>::m_result = { std::move(args)... };
        return std::move(r);
    }
    
    template <typename T, typename E>
    template <typename ...Ts>
    ParseResult<T, E> ParseResult<T, E>::NOT_OK(std::size_t error_position, Ts&& ... args) {
        ParseResult<T, E> e { };
        e.m_offset = error_position;
        e.Result<T, E>::m_error = { std::move(args)... };
        return std::move(e);
    }

    template <typename T, typename E>
    ParseResult<T, E>::~ParseResult() = default;
    
    template <typename T, typename E>
    std::size_t ParseResult<T, E>::offset() const {
        return m_offset;
    }
    
}

#endif // RESULT_TPP
