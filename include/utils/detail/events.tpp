
#ifndef EVENTS_TPP
#define EVENTS_TPP

#include "utils/hash.hpp"
#include "utils/assert.hpp"
#include "utils/memory.hpp"
#include "utils/datetime.hpp"

#include <unordered_map> // std::unordered_map
#include <unordered_set> // std::unordered_set
#include <memory> // std::unique_ptr, std::weak_ptr
#include <vector> // std::vector
#include <typeindex> // std::type_index
#include <functional> // std::function

namespace utils {
    namespace detail {
        
        struct EventData;
        
        class Callback {
            public:
                // Type-erased event handler callback
                using FunctionType = std::function<bool(const EventData&)>;
                
                template <typename Fn>
                explicit Callback(Fn fn);
                
                // Wrappers around public-facing events API to avoid incurring overhead with copying std::function objects
                template <typename T, typename U, typename E>
                Callback(const std::shared_ptr<T>& object, bool (U::*function)(const E&));
                
                template <typename T, typename U, typename E>
                Callback(const std::shared_ptr<T>& object, bool (U::*function)(const E&) const);
                
                template <typename T, typename U, typename E>
                Callback(const std::shared_ptr<T>& object, bool (U::*function)(E));
                
                template <typename T, typename U, typename E>
                Callback(const std::shared_ptr<T>& object, bool (U::*function)(E) const);
                
                template <typename T, typename U, typename E>
                Callback(T* object, bool (U::*function)(const E&));
                
                template <typename T, typename U, typename E>
                Callback(T* object, bool (U::*function)(const E&) const);
                
                template <typename T, typename U, typename E>
                Callback(T* object, bool (U::*function)(E));
                
                template <typename T, typename U, typename E>
                Callback(T* object, bool (U::*function)(E) const);
                
                template <typename E>
                Callback(bool (*function)(const E&));
                
                template <typename E>
                Callback(bool (*function)(E));
                
                ~Callback();
                
                [[nodiscard]] inline bool invoke(const EventData& data);
                
                [[nodiscard]] bool expired() const;
                [[nodiscard]] bool enabled() const;
                
                // Marks this callback for deletion
                // Will be deleted before the next call to process_events
                void deregister();

                void enable();
                void disable();
                
                [[nodiscard]] std::size_t id() const;
                [[nodiscard]] std::type_index type() const;
                
            private:
                static constexpr std::size_t ENABLED_BIT = 1 << (std::numeric_limits<std::uint8_t>::digits - 1); // 0b10000000
                static constexpr std::size_t TOMBSTONED_BIT = 1 << (std::numeric_limits<std::uint8_t>::digits - 2); // 0b01000000
                
                [[nodiscard]] bool deregistered() const;
            
                // Remains uninitialized for lambdas / global functions
                std::weak_ptr<void> m_object;
                FunctionType m_function;
                
                std::type_index m_type;
                std::size_t m_id;
                
                std::uint8_t m_flags;
        };
        
        using CallbackHandle = std::shared_ptr<Callback>;
        
        class IdGenerator {
            public:
                IdGenerator();
                ~IdGenerator() = default;
                
                // Returns an old ID to the list for later reuse
                void recycle(std::size_t id);
                
                // Retrieves (or generates) the next available ID
                [[nodiscard]] std::size_t next();
                
            private:
                // Structure maintains an internal list of intervals for a compact representation
                struct Interval {
                    std::size_t end;
                    std::size_t start;
                };
                
                // Merges 'it' with any neighboring intervals
                void merge(std::vector<Interval>::iterator it);
                
                std::vector<Interval> m_intervals;
                std::size_t m_id;
        };
        
        struct EventData {
            void* data;
            std::type_index type;
        };
        
        // EventQueue acts as a linear allocator for a given frame of events
        class EventQueue {
            public:
                class ForwardIterator {
                    public:
                        ForwardIterator(void* base);
                        ~ForwardIterator() = default;
                        
                        EventData operator*() const;
                        ForwardIterator& operator++();
                        bool operator!=(const ForwardIterator& other) const;
                        
                    private:
                        void* m_base;
                };
                
                explicit EventQueue(std::size_t size = kilobytes(256));
                ~EventQueue() = default;
                
                template <typename E>
                void push(E&& event);
                
                [[nodiscard]] ForwardIterator begin();
                [[nodiscard]] ForwardIterator end();
                
                // Random element access would be an O(N) operation and is not supported
                // const Event& operator[](std::size_t);
                
                void reset();
                
            private:
                // Metadata about an event allocation
                struct Allocation {
                    union {
                        CopyConstructor copy;
                        MoveConstructor move;
                    } copy_constructor;
                    Destructor destructor;
                    unsigned int size : 30;
                    enum class CopyConstructorType {
                        None = 0,
                        Copy,
                        Move,
                    } copy_op : 2;
                    
                    std::type_index type;
                    // Timestamp timestamp;
                };
                
                void reallocate();
                
                void* m_data;
                std::size_t m_offset;
                
                std::size_t m_capacity; // Capacity, in bytes
                std::size_t m_allocation_count;
        };
        
        template <typename T>
        struct callback_traits : callback_traits<decltype(&T::operator())> { };
        
        template <typename T, typename E>
        struct callback_traits<bool (T::*)(const E&) const> {
            using ClassType = T;
            using EventType = E;
        };
        
        template <typename T, typename E>
        struct callback_traits<bool (T::*)(const E&)> {
            using ClassType = T;
            using EventType = E;
        };
        
        template <typename T, typename E>
        struct callback_traits<bool (T::*)(E) const> {
            using ClassType = T;
            using EventType = E;
        };
        
        template <typename T, typename E>
        struct callback_traits<bool (T::*)(E)> {
            using ClassType = T;
            using EventType = E;
        };
        
        template <typename E>
        struct callback_traits<bool(*)(const E&)> {
            using EventType = E;
        };
        
        template <typename E>
        struct callback_traits<bool(*)(E)> {
            using EventType = E;
        };
        
        template <typename T, typename Fn>
        inline EventHandler register_event_handler(T* object, Fn function);
        
        template <typename Fn>
        inline EventHandler register_event_handler(Fn function);
        
        template <typename T, typename Fn>
        inline void deregister_event_handler(T* object, Fn function);
        
        template <typename Fn>
        inline void deregister_event_handler(Fn function);
        
        // Specialization for deregistering all callbacks for a given object or global function
        template <>
        inline void deregister_event_handler(std::uintptr_t address);
        
        
        // Indices change when expired callbacks are removed, so lookup is done by callback id instead
        [[nodiscard]] CallbackHandle get_callback(std::size_t address, std::size_t id);
        
        IdGenerator id_generator;
        EventQueue event_queue;
        std::unordered_map<std::type_index, std::vector<std::weak_ptr<Callback>>> dispatch_map;
        
        using CallbackRegistration = std::variant<std::monostate, CallbackHandle, std::vector<CallbackHandle>>;
        std::unordered_map<std::uintptr_t, CallbackRegistration> callback_registrations;
        
        // Note for Callback constructors: object validity is checked before the callback is constructed
        
        template <typename Fn>
        Callback::Callback(Fn fn)
            : m_object(),
            m_function([fn = std::move(fn)](const EventData& event) -> bool {
                using E = callback_traits<Fn>::EventType;
                return fn(*static_cast<E*>(event.data));
            }),
            m_type(typeid(typename callback_traits<Fn>::EventType)),
            m_id(id_generator.next()),
            m_flags(ENABLED_BIT) {
        }
        
        template <typename T, typename U, typename E>
        Callback::Callback(const std::shared_ptr<T>& object, bool (U::*function)(const E&))
            : m_object(object),
              m_function([object = std::weak_ptr<T>(object), fn = std::move(function)](const EventData& event) -> bool {
                  return (object.lock()->*fn)(*static_cast<E*>(event.data));
              }),
              m_type(typeid(E)),
              m_id(id_generator.next()),
              m_flags(ENABLED_BIT) {
        }
        
        template <typename T, typename U, typename E>
        Callback::Callback(const std::shared_ptr<T>& object, bool (U::*function)(E))
            : m_object(object),
              m_function([object = std::weak_ptr<T>(object), fn = std::move(function)](const EventData& event) -> bool {
                  return (object.lock()->*fn)(*static_cast<E*>(event.data));
              }),
              m_type(typeid(E)),
              m_id(id_generator.next()),
              m_flags(ENABLED_BIT) {
        }
        
        template <typename T, typename U, typename E>
        Callback::Callback(const std::shared_ptr<T>& object, bool (U::*function)(E) const)
            : m_object(object),
              m_function([object = std::weak_ptr<T>(object), fn = std::move(function)](const EventData& event) -> bool {
                  return (object.lock()->*fn)(*static_cast<E*>(event.data));
              }),
              m_type(typeid(E)),
              m_id(id_generator.next()),
              m_flags(ENABLED_BIT) {
        }
        
        template <typename T, typename U, typename E>
        Callback::Callback(T* object, bool (U::*function)(const E&))
            : m_object(),
              m_function([object, fn = std::move(function)](const EventData& event) -> bool {
                  return (object->*fn)(*static_cast<E*>(event.data));
              }),
              m_type(typeid(E)),
              m_id(id_generator.next()),
              m_flags(ENABLED_BIT) {
        }
        
        template <typename T, typename U, typename E>
        Callback::Callback(T* object, bool (U::*function)(const E&) const)
            : m_object(),
              m_function([object, fn = std::move(function)](const EventData& event) -> bool {
                  return (object->*fn)(*static_cast<E*>(event.data));
              }),
              m_type(typeid(E)),
              m_id(id_generator.next()),
              m_flags(ENABLED_BIT) {
        }
        
        template <typename T, typename U, typename E>
        Callback::Callback(T* object, bool (U::*function)(E))
            : m_object(),
              m_function([object, fn = std::move(function)](const EventData& event) -> bool {
                  return (object->*fn)(*static_cast<E*>(event.data));
              }),
              m_type(typeid(E)),
              m_id(id_generator.next()),
              m_flags(ENABLED_BIT) {
        }
        
        template <typename T, typename U, typename E>
        Callback::Callback(T* object, bool (U::*function)(E) const)
            : m_object(),
              m_function([object, fn = std::move(function)](const EventData& event) -> bool {
                  return (object->*fn)(*static_cast<E*>(event.data));
              }),
              m_type(typeid(E)),
              m_id(id_generator.next()),
              m_flags(ENABLED_BIT) {
        }
        
        template <typename E>
        Callback::Callback(bool (*function)(const E&))
            : m_object(),
              m_function([fn = std::move(function)](const EventData& event) -> bool {
                  return fn(*static_cast<E*>(event.data));
              }),
              m_type(typeid(E)),
              m_id(id_generator.next()),
              m_flags(ENABLED_BIT) {
        }
        
        template <typename E>
        Callback::Callback(bool (*function)(E))
            : m_object(),
              m_function([fn = std::move(function)](const EventData& event) -> bool {
                  return fn(*static_cast<E*>(event.data));
              }),
              m_type(typeid(E)),
              m_id(id_generator.next()),
              m_flags(ENABLED_BIT) {
        }
        
        template <typename E>
        void EventQueue::push(E&& event) {
            std::size_t block_size = sizeof(Allocation) + sizeof(E);
            std::size_t available_size = m_capacity - m_offset;
            if (available_size < block_size) {
                // Allocator is out of memory, need to reallocate the internal data store
                m_capacity *= 2;
                reallocate();
            }
            
            std::byte* block = reinterpret_cast<std::byte*>(m_data) + m_offset;
            m_offset += block_size;
            
            // Initialize metadata block
            Allocation& allocation = *reinterpret_cast<Allocation*>(block);
            allocation.copy_op = Allocation::CopyConstructorType::None;
            allocation.type = typeid(E);
            allocation.size = block_size;
            
            if constexpr (!std::is_trivially_destructible<E>::value && std::is_destructible<E>::value) {
                // Hybrid allocator automatically stores destructors for non-trivially destructible types
                allocation.destructor = get_destructor<E>();
            }
            else {
                allocation.destructor = nullptr; // Clear underlying memory since Allocation objects are reused per frame
            }
            
            // Prefer move constructor over copy constructor, if available
            if constexpr (!std::is_trivially_copy_constructible<E>::value && std::is_copy_constructible<E>::value) {
                allocation.copy_constructor.copy = get_copy_constructor<E>();
                allocation.copy_op = Allocation::CopyConstructorType::Copy;
            }
            if constexpr (!std::is_trivially_move_constructible<E>::value && std::is_move_constructible<E>::value) {
                allocation.copy_constructor.move = get_move_constructor<E>();
                allocation.copy_op = Allocation::CopyConstructorType::Move;
            }
            
            void* data = block + sizeof(Allocation);
            
            // Migrate event data
            switch (allocation.copy_op) {
                case Allocation::CopyConstructorType::None:
                    // Event type is trivially copyable
                    std::memcpy(data, &event, sizeof(E));
                    break;
                case Allocation::CopyConstructorType::Copy:
                    allocation.copy_constructor.copy(data, &event);
                    break;
                case Allocation::CopyConstructorType::Move:
                    allocation.copy_constructor.move(data, &event);
                    break;
            }
        }
        
        template <typename T, typename Fn>
        EventHandler register_event_handler(T* object, Fn function) {
            using U = callback_traits<Fn>::ClassType;
            using E = callback_traits<Fn>::EventType;
            
            // Allow polymorphic class instances to register event listeners to functions of a base class type, but not the other way around:
            //
            // struct Base {
            //     bool handle_event(const int*);
            // };
            //
            // struct Derived : public Base {
            //     bool handle_event(const int*);
            // };
            //
            // Allow register_event_handler(std::shared_ptr<Derived>, &Base::handle_event), but not register_event_handler(std::shared_ptr<Base>, &Derived::handle_event)
            static_assert(std::is_convertible<U, T>::value);
            
            if (!object) {
                return { };
            }
            if (!function) {
                return { };
            }
    
            CallbackHandle callback;
            
            // This will create a new registration if one does not already exist
            // A new registration will contain a std::monostate object
            std::uintptr_t address = (std::uintptr_t) object;
            CallbackRegistration& registration = callback_registrations[address];
            
            if (std::holds_alternative<std::monostate>(registration)) {
                // This is the first registration for this address
                callback = std::make_shared<Callback>(object, function);
                registration = callback;
            }
            else if (std::holds_alternative<CallbackHandle>(registration)) {
                // The event system only incurs the overhead of a std::vector when more than one callback is registered to the same (object) address
                // This is to (more) efficiently support registrations of global functions (1:1 mapping) or objects with only one event handler
                
                const CallbackHandle& handle = std::get<CallbackHandle>(registration);
                if (handle->type() == typeid(E)) {
                    // An event handler for this event type has already been registered for this object
                    // The system supports only one callback per event type per object
                    callback = handle;
                }
                else {
                    callback = std::make_shared<Callback>(object, function);
                    
                    // Maintain the existing order of callback registration
                    registration = std::vector<CallbackHandle> {
                        handle,
                        callback
                    };
                }
            }
            else if (std::holds_alternative<std::vector<CallbackHandle>>(registration)) {
                std::vector<CallbackHandle>& callbacks = std::get<std::vector<CallbackHandle>>(registration);
                for (const CallbackHandle& handle : callbacks) {
                    if (handle->type() == typeid(E)) {
                        // An event handler for this event type has already been registered for this object
                        // The system supports only one callback per event type per object
                        callback = handle;
                        break;
                    }
                }
                
                if (!callback) {
                    callback = callbacks.emplace_back(std::make_shared<Callback>(object, function));
                }
            }
            
            return EventHandler(address, callback->id());
        }
        
        template <typename Fn>
        EventHandler register_event_handler(Fn function) {
            using E = callback_traits<Fn>::EventType;
            if (!function) {
                return { };
            }
    
            CallbackHandle callback;
            
            // Global function callbacks use the function address directly as the key
            std::uintptr_t address = function;
            
            // This will create a new registration if one does not already exist
            // A new registration will contain a std::monostate object
            CallbackRegistration& registration = callback_registrations[address];
            
            // Global functions can only ever refer to a single event handler
            ASSERT(!std::holds_alternative<std::vector<CallbackHandle>>(registration), "invalid event handler registration");
            
            if (std::holds_alternative<std::monostate>(registration)) {
                callback = std::make_shared<Callback>(function);
                registration = callback;
            }
            else {
                callback = std::get<CallbackHandle>(registration);
                ASSERT(callback->type() == typeid(E), "invalid event handler registration");
            }
            
            return EventHandler(address, callback->id());
        }
        
        template <typename T, typename Fn>
        void deregister_event_handler(T* object, Fn function) {
            using U = callback_traits<Fn>::ClassType;
            using E = callback_traits<Fn>::EventType;
            
            // Allow polymorphic class instances to deregister event listeners to functions of a base class type, but not the other way around:
            //
            // struct Base {
            //     bool handle_event(const int*);
            // };
            //
            // struct Derived : public Base {
            //     bool handle_event(const int*);
            // };
            //
            // Allow deregister_event_handler(std::shared_ptr<Derived>, &Base::handle_event), but not deregister_event_handler(std::shared_ptr<Base>, &Derived::handle_event)
            static_assert(std::is_convertible<U, T>::value);
            
            if (!object) {
                return;
            }
            if (!function) {
                return;
            }
            
            std::uintptr_t address = object;
            auto it = callback_registrations.find(address);
            if (it == callback_registrations.end()) {
                // No callback registrations exist for the given object
                return;
            }
            
            CallbackRegistration& registration = it->second;
            
            if (std::holds_alternative<std::monostate>(registration)) {
                // No callback registrations exist for the given object
                // This will be cleaned up by a call to
                return;
            }
            else if (std::holds_alternative<CallbackHandle>(registration)) {
                const CallbackHandle& callback = std::get<CallbackHandle>(registration);
                if (callback->type() == typeid(E)) {
                    registration = std::monostate { };
                }
            }
            else { // if (std::holds_alternative<std::vector<CallbackHandle>>(registration))
                std::vector<CallbackHandle>& callbacks = std::get<std::vector<CallbackHandle>>(registration);
                std::erase_if(callbacks, [](const CallbackHandle& callback) -> bool {
                    return callback->type() == typeid(E);
                });
                
                if (callbacks.empty()) {
                    // Is it better to not deallocate vector memory so that further registrations are quicker?
                    registration = std::monostate { };
                }
            }
        }
        
        template <typename Fn>
        void deregister_event_handler(Fn function) {
            using E = callback_traits<Fn>::EventType;
            if (!function) {
                return;
            }
        }
        
    } // namespace detail
    
    // Public API implementation
    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(std::shared_ptr<T> object, bool (U::*function)(const E&)) {
        return detail::register_event_handler(object.get(), function);
    }

    template <typename T, typename U, typename E>
    EventHandler register_event_handler(std::shared_ptr<T> object, bool (U::*function)(const E&) const) {
        return detail::register_event_handler(object.get(), function);
    }
    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(std::shared_ptr<T> object, bool (U::*function)(E)) {
        return detail::register_event_handler(object.get(), function);
    }
    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(std::shared_ptr<T> object, bool (U::*function)(E) const) {
        return detail::register_event_handler(object.get(), function);
    }
    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(T* object, bool (U::*function)(const E&)) {
        return detail::register_event_handler(object, function);
    }
    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(T* object, bool (U::*function)(const E&) const) {
        return detail::register_event_handler(object, function);
    }

    template <typename T, typename U, typename E>
    EventHandler register_event_handler(T* object, bool (U::*function)(E)) {
        return detail::register_event_handler(object, function);
    }
    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(T* object, bool (U::*function)(E) const) {
        return detail::register_event_handler(object, function);
    }
    
    template <typename E>
    EventHandler register_event_handler(bool (*function)(const E*)) {
        return detail::register_event_handler(function);
    }
    
    template <typename Fn>
    EventHandler register_event_handler(Fn&& function) {
        using namespace detail;
        
        // Lambda callbacks are always considered unique (there is no way to easily determine if two lambdas are equal)
        CallbackHandle callback = std::make_shared<Callback>(function);
        
        std::uintptr_t address = 0;  // Lambdas use reserved memory address 0
        CallbackRegistration& registration = callback_registrations[address];
        
        if (std::holds_alternative<std::monostate>(registration)) {
            registration = callback;
        }
        else if (std::holds_alternative<CallbackHandle>(registration)) {
            const CallbackHandle& handle = std::get<CallbackHandle>(registration);
            registration = std::vector<CallbackHandle> {
                handle,
                callback
            };
        }
        else if (std::holds_alternative<std::vector<CallbackHandle>>(registration)) {
            std::vector<CallbackHandle>& callbacks = std::get<std::vector<CallbackHandle>>(registration);
            callbacks.emplace_back(callback);
        }
        
        return EventHandler(address, callback->id());
    }

    template <typename T, typename U, typename E>
    void deregister_event_handler(std::shared_ptr<T> object, bool (U::*function)(const E&)) {
        detail::deregister_event_handler(object.get(), function);
    }
    
    template <typename T, typename U, typename E>
    void deregister_event_handler(std::shared_ptr<T> object, bool (U::*function)(const E&) const) {
        detail::deregister_event_handler(object.get(), function);
    }
    
    template <typename T, typename U, typename E>
    void deregister_event_handler(std::shared_ptr<T> object, bool (U::*function)(E)) {
        detail::deregister_event_handler(object.get(), function);
    }
    
    template <typename T, typename U, typename E>
    void deregister_event_handler(std::shared_ptr<T> object, bool (U::*function)(E) const) {
        detail::deregister_event_handler(object.get(), function);
    }

    template <typename T>
    void deregister_event_handler(std::shared_ptr<T> object) {
        detail::deregister_event_handler((std::uintptr_t) object.get());
    }
    
    template <typename T, typename U, typename E>
    void deregister_event_handler(T* object, bool (U::*function)(const E&)) {
        detail::deregister_event_handler(object, function);
    }
    
    template <typename T, typename U, typename E>
    void deregister_event_handler(T* object, bool (U::*function)(const E&) const) {
        detail::deregister_event_handler(object, function);
    }
    
    template <typename T, typename U, typename E>
    void deregister_event_handler(T* object, bool (U::*function)(E)) {
        detail::deregister_event_handler(object, function);
    }
    
    template <typename T, typename U, typename E>
    void deregister_event_handler(T* object, bool (U::*function)(E) const) {
        detail::deregister_event_handler(object, function);
    }

    template <typename T>
    void deregister_event_handler(T* object) {
        detail::deregister_event_handler((std::uintptr_t) object);
    }
    
    template <typename E>
    void deregister_event_handler(bool (*function)(const E&)) {
        detail::deregister_event_handler((std::uintptr_t) function);
    }
    
    template <typename E>
    void deregister_event_handler(bool (*function)(E)) {
        detail::deregister_event_handler((std::uintptr_t) function);
    }
    
    template <typename E>
    void dispatch_event(E&& event) {
        using namespace detail;
        event_queue.push(event);
    }
    
    // To be called by the implementation
    // Processes all events from the last frame
    void process_events();
    
}

#endif // EVENTS_TPP