
#ifndef EVENTS_HPP
#define EVENTS_HPP

#include <memory> // std::shared_ptr

namespace utils {
    
    class EventHandler {
        public:
            EventHandler();
            EventHandler(std::uintptr_t address, std::size_t id);
            ~EventHandler() = default;
            
            void enable() const;
            void disable() const;
            
            [[nodiscard]] bool enabled() const;
            
            void deregister() const;
            
        private:
            std::uintptr_t m_address;
            std::size_t m_id;
    };
    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(std::shared_ptr<T> object, bool (U::*function)(const E&));
    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(std::shared_ptr<T> object, bool (U::*function)(const E&) const);
    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(std::shared_ptr<T> object, bool (U::*function)(E));
    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(std::shared_ptr<T> object, bool (U::*function)(E) const);

    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(T* object, bool (U::*function)(const E&));
    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(T* object, bool (U::*function)(const E&) const);
    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(T* object, bool (U::*function)(E));
    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(T* object, bool (U::*function)(E) const);

    
    template <typename E>
    EventHandler register_event_handler(bool (*function)(const E&));
    
    template <typename E>
    EventHandler register_event_handler(bool (*function)(E));
    
    
    // Lambdas can only be deregistered through the EventHandler
    template <typename Fn>
    EventHandler register_event_handler(Fn&& function);

    
    template <typename T, typename U, typename E>
    void deregister_event_handler(std::shared_ptr<T> object, bool (U::*function)(const E&));
    
    template <typename T, typename U, typename E>
    void deregister_event_handler(std::shared_ptr<T> object, bool (U::*function)(const E&) const);
    
    template <typename T, typename U, typename E>
    void deregister_event_handler(std::shared_ptr<T> object, bool (U::*function)(E));
    
    template <typename T, typename U, typename E>
    void deregister_event_handler(std::shared_ptr<T> object, bool (U::*function)(E) const);

    template <typename T>
    void deregister_event_handler(std::shared_ptr<T> object);
    
    
    template <typename T, typename U, typename E>
    void deregister_event_handler(T* object, bool (U::*function)(const E&));
    
    template <typename T, typename U, typename E>
    void deregister_event_handler(T* object, bool (U::*function)(const E&) const);
    
    template <typename T, typename U, typename E>
    void deregister_event_handler(T* object, bool (U::*function)(E));
    
    template <typename T, typename U, typename E>
    void deregister_event_handler(T* object, bool (U::*function)(E) const);

    template <typename T>
    void deregister_event_handler(T* object);
    
    
    template <typename E>
    void dispatch_event(E&& event);
    
}

#include "utils/detail/events.tpp"

#endif // EVENTS_HPP
