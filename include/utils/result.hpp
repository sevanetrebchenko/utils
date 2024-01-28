
#ifndef UTILS_RESULT_HPP
#define UTILS_RESULT_HPP

#include <string> // std::string
#include <optional> // std::optional

namespace utils {
    
    // Unlike Result<T>, a Response instance does not contain a data payload. This is suitable, for example, for when
    // a validating function doesn't need to return any value on success but returns an error message on failure.
    // These classes are purposefully designed without polymorphism in mind, and users should interact with these classes
    // through their concrete types (and not through pointers / references to the base Response class).
    
    class Response {
        public:
            static Response OK();
            static Response NOT_OK(const std::string& error);
            ~Response();
            
            [[nodiscard]] bool ok() const;
            [[nodiscard]] const std::string& what() const;
            
        protected:
            Response();
            explicit Response(std::string error);
            
            std::string m_error;
    };
    
    template <typename T>
    class Result final : private Response {
        public:
            template <typename ...Ts>
            static Result<T> OK(Ts&&... args);
            static Result<T> NOT_OK(const std::string& error);
            ~Result();
            
            [[nodiscard]] bool ok() const;
            [[nodiscard]] const std::string& what() const;
            
            [[nodiscard]] T& operator->() const;
            
        private:
            template <typename ...Ts>
            explicit Result(Ts&&... args);
            
            std::optional<T> m_result;
    };
    
}

#include "utils/internal/result.tpp"

#endif // UTILS_RESULT_HPP
