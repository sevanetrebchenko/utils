
#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <memory> // std::shared_ptr

namespace utils {
    
    class EventHandler {
        public:
            ~EventHandler() = default;
            
            void enable() const;
            void disable() const;
            
            [[nodiscard]] bool enabled() const;
            
            void deregister() const;
            
        protected:
            EventHandler(std::size_t id);
            std::size_t m_id;
    };
    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(std::shared_ptr<T> instance, bool (U::*function)(const E*));

    template <typename T, typename U, typename E>
    EventHandler register_event_handler(T* instance, bool (U::*function)(const E*));

    template <typename E>
    EventHandler register_event_handler(bool (*function)(const E*));
    
    template <typename Fn>
    EventHandler register_event_handler(Fn&& function);

    
    template <typename T, typename U, typename E>
    void deregister_event_handler(std::shared_ptr<T> instance, bool (U::*function)(const E*));

    template <typename T>
    void deregister_event_handler(std::shared_ptr<T> instance);

    template <typename T, typename U, typename E>
    void deregister_event_handler(T* instance, bool (U::*function)(const E*));
    
    template <typename T>
    void deregister_event_handler(T* instance);
    
    template <typename E>
    void deregister_event_handler(bool (*function)(const E*));
    
    
    template <typename E>
    void dispatch_event(E&& event);
    
}

#include "utils/detail/events.tpp"

#endif // EVENTS_HPP
