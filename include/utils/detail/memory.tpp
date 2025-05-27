#ifndef MEMORY_TPP
#define MEMORY_TPP

namespace utils {
    
    template <typename T>
    constexpr Destructor get_destructor() {
        return +[](void* obj) -> void {
            static_cast<T*>(obj)->~T();
        };
    }
    
    template <typename T>
    constexpr CopyConstructor get_copy_constructor() {
        return +[](void* dest, const void* src) {
            new (dest) T(*static_cast<const T*>(src));
        };
    }
    
    template <typename T>
    constexpr MoveConstructor get_move_constructor() {
        return +[](void* dest, void* src) {
            new (dest) T(std::move(*static_cast<T*>(src)));
        };
    }
    
}

#endif // MEMORY_TPP