
#ifndef TUPLE_TPP
#define TUPLE_TPP

#include "utils/format.hpp"

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
        
        template <typename Fn, typename T, std::size_t Is>
        concept invocable_with_compile_time_index = requires(Fn fn, T value) {
            { fn.template operator()<T, Is>(value) };
        };
        
        // For invoking callback 'fn' on each element of the tuple
        template <typename Fn, typename ...Ts, std::size_t ...Is>
        inline void apply(Fn&& fn, const std::tuple<Ts...>& tuple, std::index_sequence<Is...>) requires (invocable_with_value<Fn, typename std::decay<decltype(std::get<Is>(tuple))>::type> && ...) {
            (fn.template operator()<typename std::decay<Ts>::type>(std::get<Is>(tuple)), ...);
        }
        
        // For invoking callback 'fn' on each element of the tuple, passing the index of the element as an additional parameter
        template <typename Fn, typename ...Ts, std::size_t ...Is>
        inline void apply(Fn&& fn, const std::tuple<Ts...>& tuple, std::index_sequence<Is...>) requires (invocable_with_runtime_index<Fn, typename std::decay<decltype(std::get<Is>(tuple))>::type> && ...) {
            (fn.template operator()<typename std::decay<Ts>::type>(std::get<Is>(tuple), Is), ...);
        }
        
        // For invoking callback 'fn' on one element of the tuple at runtime index 'index'
        template <typename Fn, typename ...Ts, std::size_t ...Is>
        inline void apply(Fn&& fn, const std::tuple<Ts...>& tuple, std::size_t index, std::index_sequence<Is...>) requires (invocable_with_value<Fn, typename std::decay<decltype(std::get<Is>(tuple))>::type> && ...)  {
            ((index == Is && (fn.template operator()<typename std::decay<Ts>::type>(std::get<Is>(tuple))), true), ...);
        }
        
        // For invoking callback 'fn' on one element of the tuple at runtime index 'index', passing 'index' as an additional parameter
        template <typename Fn, typename ...Ts, std::size_t ...Is>
        inline void apply(Fn&& fn, const std::tuple<Ts...>& tuple, std::size_t index, std::index_sequence<Is...>) requires (invocable_with_runtime_index<Fn, typename std::decay<decltype(std::get<Is>(tuple))>::type> && ...)  {
            ((index == Is && (fn.template operator()<typename std::decay<Ts>::type>(std::get<Is>(tuple), Is), true)), ...);
        }
        
        // For invoking callback 'fn' on each element of the tuple, passing the index of the element as an additional template parameter
        template <typename Fn, typename... Ts, std::size_t ...Is>
        inline void apply(Fn&& fn, const std::tuple<Ts...>& tuple, std::index_sequence<Is...>) requires (invocable_with_compile_time_index<Fn, typename std::decay<decltype(std::get<Is>(tuple))>::type, Is> && ...) {
            ((fn.template operator()<typename std::decay<Ts>::type, Is>(std::get<Is>(tuple))), ...);
        }
        
        // For invoking callback 'fn' on one element of the tuple at runtime index 'index', passing 'index' as an additional template parameter
        template <typename Fn, typename ...Ts, std::size_t ...Is>
        inline void apply(Fn&& fn, const std::tuple<Ts...>& tuple, std::size_t index, std::index_sequence<Is...>) requires (invocable_with_compile_time_index<Fn, typename std::decay<decltype(std::get<Is>(tuple))>::type, Is> && ...) {
            ((index == Is && ((fn.template operator()<typename std::decay<Ts>::type, Is>(std::get<Is>(tuple))), true)), ...);
        }
        
        
        template <typename Fn, typename ...Ts, std::size_t ...Is>
        inline void apply_for(Fn&& fn, const std::tuple<Ts...>& tuple, std::size_t start, std::size_t end, std::index_sequence<Is...>) requires (invocable_with_value<Fn, typename std::decay<decltype(std::get<Is>(tuple))>::type> && ...)  {
            (((Is >= start && Is <= end) && (fn.template operator()<typename std::decay<Ts>::type>(std::get<Is>(tuple))), true), ...);
        }
        
        
        template <typename Fn, typename ...Ts, std::size_t ...Is>
        inline void apply_for(Fn&& fn, const std::tuple<Ts...>& tuple, std::size_t start, std::size_t end, std::index_sequence<Is...>) requires (invocable_with_runtime_index<Fn, typename std::decay<decltype(std::get<Is>(tuple))>::type> && ...)  {
            (((Is >= start && Is <= end) && (fn.template operator()<typename std::decay<Ts>::type>(std::get<Is>(tuple), Is), true)), ...);
        }
        
        template <typename Fn, typename ...Ts, std::size_t ...Is>
        inline void apply_for(Fn&& fn, const std::tuple<Ts...>& tuple, std::size_t start, std::size_t end, std::index_sequence<Is...>) requires (invocable_with_compile_time_index<Fn, typename std::decay<decltype(std::get<Is>(tuple))>::type, Is> && ...) {
            (((Is >= start && Is <= end) && ((fn.template operator()<typename std::decay<Ts>::type, Is>(std::get<Is>(tuple))), true)), ...);
        }
        
    }
    
    template <typename Fn, typename ...Ts>
    void apply(Fn&& fn, const std::tuple<Ts...>& tuple) {
        detail::apply(fn, tuple, std::index_sequence_for<Ts...> { });
    }
    
    template <typename Fn, typename ...Ts>
    void apply(Fn&& fn, const std::tuple<Ts...>& tuple, std::size_t index) {
        detail::apply(fn, tuple, index, std::index_sequence_for<Ts...> { });
    }
    
    template <typename Fn, typename ...Ts>
    void apply_for(Fn&& fn, const std::tuple<Ts...>& tuple, std::size_t start, std::size_t end) {
        detail::apply_for(fn, tuple, start, end, std::index_sequence_for<Ts...> { });
    }
    
}

#endif // TUPLE_TPP
