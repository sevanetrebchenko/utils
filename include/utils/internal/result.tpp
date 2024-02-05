
#ifndef UTILS_RESULT_TPP
#define UTILS_RESULT_TPP

#include "utils/string.hpp"
#include <stdexcept> // std::runtime_error
#include <utility> // std::move

namespace utils {

    template <typename T>
    template <typename ...Ts>
    Result<T>::Result(Ts&& ...args) : Response(),
                                      m_result(std::move(args)...)
                                      {
    }
    
    template <typename T>
    Result<T>::Result() : Response(),
                          m_result(T { })
                          {
    }
    
    template <typename T>
    Result<T>::~Result() = default;
    
    template <typename T>
    template <typename ...Args>
    Result<T> Result<T>::NOT_OK(const std::string& format_string, const Args&... args) {
        Result<T> result { };
        
        if constexpr (sizeof...(Args) > 0u) {
            result.m_error = format(format_string, args...);
        }
        else {
            result.m_error = format_string;
        }

        return result;
    }
    
    template <typename T>
    [[nodiscard]] T& Result<T>::operator*() {
        return m_result.value();
    }
    
    template <typename T>
    [[nodiscard]] T& Result<T>::get() {
        return m_result.value();
    }
    
    template <typename T>
    T* Result<T>::operator->() {
        return &m_result.value();
    }

}

#endif // UTILS_RESULT_TPP
