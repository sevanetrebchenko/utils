
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
    class Result final : public Response {
        public:
            template <typename ...Args>
            static Result<T> NOT_OK(const std::string& format_string, const Args&... args);
            
            template <typename ...Ts>
            explicit Result(Ts&&... args);
            ~Result();
            
            [[nodiscard]] T& get() const;
            [[nodiscard]] T* operator->();
            
        private:
            std::optional<T> m_result;
    };
    
}

#include "utils/internal/result.tpp"

#endif // UTILS_RESULT_HPP
