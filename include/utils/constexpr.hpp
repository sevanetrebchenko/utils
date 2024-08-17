
#ifndef UTILS_CONSTEXPR_HPP
#define UTILS_CONSTEXPR_HPP

#include <array>
#include <forward_list>
#include <list>
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <type_traits> // std::true_type, std::false_type
#include <string> // std::string
#include <string_view> // std::string_view

namespace utils {
    
    // Standard containers.
    
    // std::array
    
    template <typename T>
    struct is_array : std::false_type { };
    
    template <typename T, std::size_t N>
    struct is_array<std::array<T, N>> : std::true_type { };
    
    // std::deque
    
    template <typename T>
    struct is_deque : std::false_type { };
    
    template <typename T, typename A>
    struct is_deque<std::deque<T, A>> : std::true_type { };
    
    // std::forward_list
    
    template <typename T>
    struct is_forward_list : std::false_type { };
    
    template <typename T, typename A>
    struct is_forward_list<std::forward_list<T, A>> : std::true_type { };
    
    // std::list
    
    template <typename T>
    struct is_list : std::false_type { };
    
    template <typename T, typename A>
    struct is_list<std::list<T, A>> : std::true_type { };
    
    // std::map
    
    template <typename T>
    struct is_map : std::false_type { };
    
    template <typename K, typename T, typename C, typename A>
    struct is_map<std::map<K, T, C, A>> : std::true_type { };
    
    // std::multimap
    
    template <typename T>
    struct is_multimap : std::false_type { };
    
    template <typename K, typename T, typename C, typename A>
    struct is_multimap<std::multimap<K, T, C, A>> : std::true_type { };
    
    // std::queue
    
    template <typename T>
    struct is_queue : std::false_type { };
    
    template <typename T, typename C>
    struct is_queue<std::queue<T, C>> : std::true_type { };
    
    // std::priority_queue
    
    template <typename T>
    struct is_priority_queue : std::false_type { };
    
    template <typename T, typename C, typename P>
    struct is_priority_queue<std::priority_queue<T, C, P>> : std::true_type { };
    
    // std::set
    
    template <typename T>
    struct is_set : std::false_type { };
    
    template <typename T, typename C, typename A>
    struct is_set<std::set<T, C, A>> : std::true_type { };
    
    // std::multiset
    
    template <typename T>
    struct is_multiset : std::false_type { };
    
    template <typename T, typename C, typename A>
    struct is_multiset<std::multiset<T, C, A>> : std::true_type { };
    
    // std::stack
    
    template <typename T>
    struct is_stack : std::false_type { };
    
    template <typename T, typename C>
    struct is_stack<std::stack<T, C>> : std::true_type { };
    
    // std::unordered_map
    
    template <typename T>
    struct is_unordered_map : std::false_type { };
    
    template <typename K, typename T, typename H, typename P, typename A>
    struct is_unordered_map<std::unordered_map<K, T, H, P, A>> : std::true_type { };
    
    // std::unordered_multimap
    
    template <typename T>
    struct is_unordered_multimap : std::false_type { };
    
    template <typename K, typename T, typename H, typename P, typename A>
    struct is_unordered_multimap<std::unordered_multimap<K, T, H, P, A>> : std::true_type { };
    
    // std::unordered_set
    
    template <typename T>
    struct is_unordered_set : std::false_type { };
    
    template <typename K, typename H, typename P, typename A>
    struct is_unordered_set<std::unordered_set<K, H, P, A>> : std::true_type { };
    
    // std::unordered_multiset
    
    template <typename T>
    struct is_unordered_multiset : std::false_type { };
    
    template <typename K, typename H, typename P, typename A>
    struct is_unordered_multiset<std::unordered_multiset<K, H, P, A>> : std::true_type { };
    
    // std::vector
    
    template <typename T>
    struct is_vector : std::false_type { };
    
    template <typename T, typename A>
    struct is_vector<std::vector<T, A>> : std::true_type { };
    
    template <typename T>
    struct is_standard_container {
        static constexpr bool value = is_array<T>::value || is_vector<T>::value ||
                                      is_deque<T>::value ||
                                      is_forward_list<T>::value || is_list<T>::value ||
                                      is_map<T>::value || is_multimap<T>::value ||
                                      is_queue<T>::value || is_priority_queue<T>::value ||
                                      is_set<T>::value || is_multiset<T>::value ||
                                      is_stack<T>::value ||
                                      is_unordered_map<T>::value || is_unordered_multimap<T>::value ||
                                      is_unordered_set<T>::value || is_unordered_multiset<T>::value;
    };
    
    // std::initializer_list
    
    template <typename T>
    struct is_initializer_list : std::false_type { };
    
    template <typename T>
    struct is_initializer_list<std::initializer_list<T>> : std::true_type { };
    
    // Standard types (non-exhaustive list).
    
    // std::pair
    
    template <typename T>
    struct is_pair : std::false_type { };
    
    template <typename T, typename U>
    struct is_pair<std::pair<T, U>> : std::true_type { };
    
    // std::tuple
    
    template <typename T>
    struct is_tuple : std::false_type { };
    
    template <typename ...Ts>
    struct is_tuple<std::tuple<Ts...>> : std::true_type { };
    
    // fundamental type categories
    
    template <typename T>
    struct is_integer_type {
        using Type = std::decay<T>::type;
        static constexpr bool value = std::is_same<Type, unsigned char>::value ||
                                      std::is_same<Type, short>::value || std::is_same<Type, unsigned short>::value ||
                                      std::is_same<Type, int>::value || std::is_same<Type, unsigned int>::value ||
                                      std::is_same<Type, long>::value || std::is_same<Type, unsigned long>::value ||
                                      std::is_same<Type, long long>::value || std::is_same<Type, unsigned long long>::value;
    };
    
    template <typename T>
    struct is_floating_point_type {
        using Type = std::decay<T>::type;
        static constexpr bool value = std::is_same<Type, float>::value || std::is_same<Type, double>::value || std::is_same<Type, long double>::value;
    };
    
    // TODO: handle volatile?
    
    template <typename T>
    struct is_string_type : std::false_type {
    };
    
    // Specializations of is_string_type for string types
    
    // char*
    template <>
    struct is_string_type<char*> : std::true_type {
    };
    
    // const char*
    template <>
    struct is_string_type<const char*> : std::true_type {
    };
    
    // char[]
    template <std::size_t N>
    struct is_string_type<char[N]> : std::true_type {
    };
    
    // const char[]
    template <std::size_t N>
    struct is_string_type<const char[N]> : std::true_type {
    };
    
    // std::string
    template <>
    struct is_string_type<std::string> : std::true_type {
    };
    
    template <>
    struct is_string_type<std::string_view> : std::true_type {
    };
    
    // Handle references to string types
    template <typename T>
    struct is_string_type<T&> : is_string_type<T> {
    };
    
    template <typename T>
    struct is_string_type<const T&> : is_string_type<T> {
    };
 
    template <typename T>
    inline constexpr bool is_string_type_v = is_string_type<T>::value;
    
}


#endif // UTILS_CONSTEXPR_HPP
