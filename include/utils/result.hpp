
#pragma once

#ifndef RESULT_HPP
#define RESULT_HPP

#include <string> // std::string
#include <string_view> // std::string_view
#include <optional> // std::optional

namespace utils {

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

    // Specialization for functions that don't need to return a value on success
    template <typename E>
    class Result<void, E> {
        public:
            static Result<void, E> OK();

            template <typename ...Ts>
            static Result<void, E> NOT_OK(Ts&&... args);

            [[nodiscard]] bool ok() const;
            [[nodiscard]] const E& error() const;

        protected:
            Result();

            std::optional<E> m_error;
    };

}

#include "utils/detail/result.tpp"

#endif // RESULT_HPP

