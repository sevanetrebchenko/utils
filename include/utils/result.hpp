
#pragma once

#ifndef UTILS_RESULT_HPP
#define UTILS_RESULT_HPP

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
            static Response NOT_OK(std::string_view fmt, const Ts&... args);
            
            ~Response();
            
            [[nodiscard]] bool ok() const;
            [[nodiscard]] const std::string& what() const;
            
        private:
            Response();
            explicit Response(std::string error);
            
            std::string m_error;
    };
    
    template <typename T, typename E>
    class Result final {
        public:
            template <typename ...Ts>
            static Result<T, E> OK(const Ts&... args);
            
            template <typename ...Ts>
            static Result<T, E> NOT_OK(const Ts&... args);
            
            ~Result();
            
            [[nodiscard]] bool ok() const;
            
            [[nodiscard]] T& result();
            [[nodiscard]] const E& error() const;
            
        private:
            Result();
            
            std::optional<T> m_result;
            std::optional<E> m_error;
    };
    
}

#include "utils/detail/result.tpp"

#endif // UTILS_RESULT_HPP
