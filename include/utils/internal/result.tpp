
#ifndef UTILS_RESULT_TPP
#define UTILS_RESULT_TPP

#include <stdexcept> // std::runtime_error

namespace utils {

    template <typename T>
    template <typename ...Ts>
    Result<T>::Result(Ts&& ...args) : m_result(std::move(args)...) {
    }
    
    template <typename T>
    Result<T>::~Result() = default;
    
    template <typename T>
    template <typename ...Ts>
    Result<T> Result<T>::OK(Ts&& ...args) {
        return Result<T>(std::move(args)...);
    }
    
    template <typename T>
    Result<T> Result<T>::NOT_OK(const std::string& error) {
        Result<T> Result { };
        Result.m_error = error;
        return Result;
    }
    
    template <typename T>
    bool Result<T>::ok() const {
        return m_result.has_value();
    }
    
    template <typename T>
    const std::string& Result<T>::what() const {
        return m_error;
    }
    
    template <typename T>
    T& Result<T>::operator->() const {
        if (!m_result.has_value()) {
            throw std::runtime_error("Result<T>::operator->() invoked on Result<T> instance that is not ok!");
        }
        return m_result;
    }

}


#endif // UTILS_RESULT_TPP
