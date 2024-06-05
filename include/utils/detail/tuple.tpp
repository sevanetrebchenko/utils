
#ifndef TUPLE_TPP
#define TUPLE_TPP

#include "utils/format.hpp"

#define GENERATE_APPLY_DEFINITIONS_FOR(TUPLE)                                                                                                                                                \
    /* For invoking callback 'fn' on each element of the tuple */                                                                                                                            \
    template <typename Fn, typename ...Ts, std::size_t ...Is>                                                                                                                                \
    inline void apply(Fn&& fn, TUPLE tuple, std::index_sequence<Is...>) requires (invocable_with_value<Fn, Ts>, ...) {                                                                       \
        (fn(std::get<Is>(tuple)), ...);                                                                                                                                                      \
    }                                                                                                                                                                                        \
                                                                                                                                                                                             \
    /* For invoking callback 'fn' on each element of the tuple, passing the index of the element as an additional parameter */                                                               \
    template <typename Fn, typename ...Ts, std::size_t ...Is>                                                                                                                                \
    inline void apply(Fn&& fn, TUPLE tuple, std::index_sequence<Is...>) requires (invocable_with_runtime_index<Fn, Ts>, ...) {                                                               \
        (fn(std::get<Is>(tuple), Is), ...);                                                                                                                                                  \
    }                                                                                                                                                                                        \
                                                                                                                                                                                             \
    /* For invoking callback 'fn' on each element of the tuple, passing both the type and the index of the element as template parameters */                                                 \
    /* This is useful for templated lambda functions where the type of any given element is not the same / easily known */                                                                   \
    template <typename Fn, typename ...Ts, std::size_t ...Is>                                                                                                                                \
    inline void apply(Fn&& fn, TUPLE tuple, std::index_sequence<Is...>) requires (invocable_with_compile_time_index<Fn, Ts, Is>, ...) {                                                      \
        (fn.template operator()<typename std::decay<Ts>::type, Is>(std::get<Is>(tuple)), ...);                                                                                               \
    }                                                                                                                                                                                        \
                                                                                                                                                                                             \
    /* For invoking callback 'fn' on each element of the tuple, passing the index of the element as the only template parameter */                                                           \
    template <typename Fn, typename... Ts, std::size_t ...Is>                                                                                                                                \
    inline void apply(Fn&& fn, TUPLE tuple, std::index_sequence<Is...>) requires (implicitly_invocable_with_compile_time_index<Fn, Ts, Is>, ...) {                                           \
        (fn.template operator()<Is>(std::get<Is>(tuple)), ...);                                                                                                                              \
    }                                                                                                                                                                                        \
                                                                                                                                                                                             \
                                                                                                                                                                                             \
                                                                                                                                                                                             \
    /* For invoking callback 'fn' on the tuple element at index 'index' */                                                                                                                   \
    template <typename Fn, typename ...Ts, std::size_t ...Is>                                                                                                                                \
    inline void apply(Fn&& fn, TUPLE tuple, std::size_t index, std::index_sequence<Is...>) requires (invocable_with_value<Fn, Ts> && ...) {                                                  \
        ((index == Is && (fn(std::get<Is>(tuple)), true)), ...);                                                                                                                              \
    }                                                                                                                                                                                        \
                                                                                                                                                                                             \
    /* For invoking callback 'fn' on the tuple element at index 'index', passing 'index' as an additional parameter */                                                                       \
    template <typename Fn, typename ...Ts, std::size_t ...Is>                                                                                                                                \
    inline void apply(Fn&& fn, TUPLE tuple, std::size_t index, std::index_sequence<Is...>) requires (invocable_with_runtime_index<Fn, Ts> && ...) {                                          \
        ((index == Is && (fn(std::get<Is>(tuple), Is), true)), ...);                                                                                                                          \
    }                                                                                                                                                                                        \
                                                                                                                                                                                             \
    /* For invoking callback 'fn' on the tuple element at index 'index', passing both the type and index of the element as template parameters */                                            \
    /* This is useful for templated lambda functions where the type of any given element is not the same / easily known */                                                                   \
    template <typename Fn, typename ...Ts, std::size_t ...Is>                                                                                                                                \
    inline void apply(Fn&& fn, TUPLE tuple, std::size_t index, std::index_sequence<Is...>) requires (invocable_with_compile_time_index<Fn, Ts, Is> && ...) {                                 \
        ((index == Is && (fn.template operator()<typename std::decay<Ts>::type, Is>(std::get<Is>(tuple)), true)), ...);                                                                       \
    }                                                                                                                                                                                        \
                                                                                                                                                                                             \
    /* For invoking callback 'fn' on the tuple element at index 'index', passing 'index' of the element as the only template parameter */                                                    \
    template <typename Fn, typename ...Ts, std::size_t ...Is>                                                                                                                                \
    inline void apply(Fn&& fn, TUPLE tuple, std::size_t index, std::index_sequence<Is...>) requires (implicitly_invocable_with_compile_time_index<Fn, Ts, Is> && ...) {                      \
        ((index == Is && (fn.template operator()<Is>(std::get<Is>(tuple)), true)), ...);                                                                                                       \
    }                                                                                                                                                                                        \
                                                                                                                                                                                             \
                                                                                                                                                                                             \
                                                                                                                                                                                             \
    /* For invoking callback 'fn' on the tuple elements contained within the range [start, end) */                                                                                           \
    template <typename Fn, typename ...Ts, std::size_t ...Is>                                                                                                                                \
    inline void apply_for(Fn&& fn, TUPLE tuple, std::size_t start, std::size_t end, std::index_sequence<Is...>) requires (invocable_with_value<Fn, Ts> && ...) {                             \
        (((Is >= start && Is < end) && (fn(std::get<Is>(tuple)), true)), ...);                                                                                                                 \
    }                                                                                                                                                                                        \
                                                                                                                                                                                             \
    /* For invoking callback 'fn' on the tuple elements contained within the range [start, end), passing the index of each element as an additional parameter */                             \
    template <typename Fn, typename ...Ts, std::size_t ...Is>                                                                                                                                \
    inline void apply_for(Fn&& fn, TUPLE tuple, std::size_t start, std::size_t end, std::index_sequence<Is...>) requires (invocable_with_runtime_index<Fn, Ts> && ...) {                     \
        (((Is >= start && Is < end) && (fn(std::get<Is>(tuple), Is), true)), ...);                                                                                                           \
    }                                                                                                                                                                                        \
                                                                                                                                                                                             \
    /* For invoking callback 'fn' on the tuple elements contained within the range [start, end), passing both the type and the index of each element as template parameters */               \
    /* This is useful for templated lambda functions where the type of any given element is not the same / easily known */                                                                   \
    template <typename Fn, typename ...Ts, std::size_t ...Is>                                                                                                                                \
    inline void apply_for(Fn&& fn, TUPLE tuple, std::size_t start, std::size_t end, std::index_sequence<Is...>) requires (invocable_with_compile_time_index<Fn, Ts, Is> && ...) {            \
        (((Is >= start && Is < end) && (fn.template operator()<typename std::decay<Ts>::type, Is>(std::get<Is>(tuple)), true)), ...);                                                          \
    }                                                                                                                                                                                        \
                                                                                                                                                                                             \
    /* For invoking callback 'fn' on the tuple elements contained within the range [start, end), passing both the type and the index of each element as template parameters */               \
    template <typename Fn, typename ...Ts, std::size_t ...Is>                                                                                                                                \
    inline void apply_for(Fn&& fn, TUPLE tuple, std::size_t start, std::size_t end, std::index_sequence<Is...>) requires (implicitly_invocable_with_compile_time_index<Fn, Ts, Is> && ...) { \
        (((Is >= start && Is < end) && (fn.template operator()<Is>(std::get<Is>(tuple)), true)), ...);                                                                                         \
    }

namespace utils {

    namespace detail {
        
        template <typename Fn, typename T>
        concept invocable_with_value = requires(Fn fn, T value) {
            { fn(value) };
        };
        
        template <typename Fn, typename T>
        concept invocable_with_runtime_index = requires(Fn fn, T value, std::size_t index) {
            { fn(value, index) };
        };
        
        template <typename Fn, typename T, std::size_t I>
        concept implicitly_invocable_with_compile_time_index = requires(Fn fn, T value) {
            { fn.template operator()<I>(value) };
        };
        
        template <typename Fn, typename T, std::size_t I>
        concept invocable_with_compile_time_index = requires(Fn fn, T value) {
            { fn.template operator()<T, I>(value) };
        };

        GENERATE_APPLY_DEFINITIONS_FOR(const std::tuple<Ts...>&);
        GENERATE_APPLY_DEFINITIONS_FOR(std::tuple<Ts...>&);
        GENERATE_APPLY_DEFINITIONS_FOR(std::tuple<Ts...>&&);
        
    }
    
    template <typename Fn, typename ...Ts>
    void apply(Fn&& fn, std::tuple<Ts...>&& tuple) {
        detail::apply(std::forward<Fn>(fn), std::forward<std::tuple<Ts...>>(tuple), std::index_sequence_for<Ts...> { });
    }
    
    template <typename Fn, typename ...Ts>
    void apply(Fn&& fn, std::tuple<Ts...>&& tuple, std::size_t index) {
        detail::apply(std::forward<Fn>(fn), std::forward<std::tuple<Ts...>>(tuple), index, std::index_sequence_for<Ts...> { });
    }
    
    template <typename Fn, typename ...Ts>
    void apply_for(Fn&& fn, std::tuple<Ts...>&& tuple, std::size_t start, std::size_t end) {
        detail::apply_for(std::forward<Fn>(fn), std::forward<std::tuple<Ts...>>(tuple), start, end, std::index_sequence_for<Ts...> { });
    }
    
    
    
    template <typename Fn, typename ...Ts>
    void apply(Fn&& fn, const std::tuple<Ts...>& tuple) {
        detail::apply(std::forward<Fn>(fn), tuple, std::index_sequence_for<Ts...> { });
    }
    
    template <typename Fn, typename ...Ts>
    void apply(Fn&& fn, const std::tuple<Ts...>& tuple, std::size_t index) {
        detail::apply(std::forward<Fn>(fn), tuple, index, std::index_sequence_for<Ts...> { });
    }
    
    template <typename Fn, typename ...Ts>
    void apply_for(Fn&& fn, const std::tuple<Ts...>& tuple, std::size_t start, std::size_t end) {
        detail::apply_for(std::forward<Fn>(fn), tuple, start, end, std::index_sequence_for<Ts...> { });
    }
    
}

#endif // TUPLE_TPP
