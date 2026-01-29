
#pragma once

#ifndef RESULT_HPP
#define RESULT_HPP

#include <string> // std::string
#include <string_view> // std::string_view
#include <optional> // std::optional

namespace utils {
    
    // Unlike Result, a Response instance does not contain a data payload
    // This is suitable, for example, for when a validating function doesn't need to return any value on success but an error type on failure
    
    template <typename E = std::string>
    class Response {
        public:
            static Response<E> OK();
            
            template <typename ...Ts>
            static Response<E> NOT_OK(Ts&&... args);
            
            [[nodiscard]] bool ok() const;
            [[nodiscard]] const E& error() const;
            
        protected:
            Response();
            
            std::optional<E> m_error;
    };

    template <typename T, typename E = std::string>
    class Result {
        public:
            template <typename ...Ts>
            static Result<T, E> OK(Ts&&... args);
            
            template <typename ...Ts>
            static Result<T, E> NOT_OK(Ts&&... args);
            
            [[nodiscard]] bool ok() const;
            
            [[nodiscard]] T& result();
            [[nodiscard]] const E& error() const;
        
        protected:
            Result();
            
            std::optional<T> m_result;
            std::optional<E> m_error;
    };
    
    
    // ParseResponse/ParseResult types are specialized types for returning the result of parsing a string, following the same pattern as the Response/Result counter types
    // Member function offset() returns the number of characters parsed on success or index of failed character on failure
    
    template <typename E>
    class ParseResponse : public Response<E> {
        public:
            static ParseResponse<E> OK(std::size_t num_characters_parsed);
            
            template <typename ...Ts>
            static ParseResponse<E> NOT_OK(std::size_t error_position, Ts&&... args);
            
            std::size_t offset() const;
            
        private:
            ParseResponse();
            
            std::size_t m_offset;
    };
    
    template <typename T, typename E>
    class ParseResult : public Result<T, E> {
        public:
            template <typename ...Ts>
            static ParseResult<T, E> OK(std::size_t num_characters_parsed, Ts&&... args);
            
            template <typename ...Ts>
            static ParseResult<T, E> NOT_OK(std::size_t error_position, Ts&&... args);

            ~ParseResult();
            
            [[nodiscard]] std::size_t offset() const;
            
        private:
            ParseResult();
            
            std::size_t m_offset;
    };
    
}

#include "utils/detail/result.tpp"

#endif // RESULT_HPP

