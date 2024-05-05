
#pragma once

#ifndef RESULT_HPP
#define RESULT_HPP

#include "utils/format.hpp"

#include <string> // std::string
#include <string_view> // std::string_view
#include <optional> // std::optional

namespace utils {
    
    // Unlike Result, a Response instance does not contain a data payload. This is suitable, for example, for when
    // a validating function doesn't need to return any value on success but returns an error message on failure.
    
    class Response {
        public:
            static Response OK();
            
            template <typename ...Ts>
            static Response NOT_OK(const FormatString& fmt, const Ts&... args);
            
            ~Response();
            
            [[nodiscard]] bool ok() const;
            [[nodiscard]] const std::string& what() const;
            
        private:
            Response();
            explicit Response(std::string error);
            
            std::string m_error;
    };
    
    template <typename T, typename E>
    class Result {
        public:
            template <typename ...Ts>
            static Result<T, E> OK(const Ts&... args);
            
            template <typename ...Ts>
            static Result<T, E> NOT_OK(const Ts&... args);
            
            ~Result();
            
            [[nodiscard]] bool ok() const;
            
            [[nodiscard]] T& result();
            [[nodiscard]] const E& error() const;
            
        protected:
            Result();
            
            std::optional<T> m_result;
            std::optional<E> m_error;
            
        private:
            template <typename S, typename ...Ts>
            std::string format(const S& str, const Ts&... args) const;
            
    };
    
    // For returning the result of parsing a string
    // Returns number of characters parsed on success or index of failed character on failure
    template <typename T, typename E>
    class ParseResult : public Result<T, E> {
        public:
            template <typename ...Ts>
            static ParseResult<T, E> OK(std::size_t offset, const Ts&... args);
            
            template <typename ...Ts>
            static ParseResult<T, E> NOT_OK(std::size_t offset, const Ts&... args);

            ~ParseResult();
            
            [[nodiscard]] std::size_t offset() const;
            
        protected:
            ParseResult();
            
            std::size_t m_offset;
    };
    
}

#include "utils/detail/result.tpp"

#endif // RESULT_HPP

