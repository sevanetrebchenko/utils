
#ifndef RESULT_TPP
#define RESULT_TPP

#include <utility> // std::move
#include <stdexcept> // std::runtime_error

namespace utils {

    template <typename T, typename E>
    Result<T, E>::Result() = default;

    template <typename T, typename E>
    template <typename ...Ts>
    Result<T, E> Result<T, E>::OK(Ts&&... args) {
        Result<T, E> r { };
        r.m_result.emplace(std::move(args)...);
        return std::move(r);
    }

    template <typename T, typename E>
    template <typename ...Ts>
    Result<T, E> Result<T, E>::NOT_OK(Ts&&... args) {
        Result<T, E> e { };
        e.m_error.emplace(std::move(args)...);
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

    template <typename E>
    Result<void, E>::Result() = default;

    template <typename E>
    Result<void, E> Result<void, E>::OK() {
        return { };
    }

    template <typename E>
    template <typename ...Ts>
    Result<void, E> Result<void, E>::NOT_OK(Ts&&... args) {
        Result<void, E> e { };
        e.m_error.emplace(std::move(args)...);
        return std::move(e);
    }

    template <typename E>
    bool Result<void, E>::ok() const {
        return !m_error.has_value();
    }

    template <typename E>
    const E& Result<void, E>::error() const {
        if (!m_error.has_value()) {
            throw std::runtime_error("error() called on Result that is ok!");
        }

        return m_error.value();
    }

}

#endif // RESULT_TPP
