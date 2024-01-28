
#ifndef UTILS_RESULT_HPP
#define UTILS_RESULT_HPP

#include <string> // std::string
#include <optional> // std::optional

namespace utils {
    
    template <typename T>
    class Result {
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
            
            std::string m_error;
            std::optional<T> m_result;
    };
    
}

#include "utils/internal/result.tpp"

#endif // UTILS_RESULT_HPP
