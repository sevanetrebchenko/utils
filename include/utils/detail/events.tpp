
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
        
        // Public-facing EventHandler (purposefully) cannot be instantiated directly
        // Derived EventHandler class circumvents this restriction (for event system internal use)
        class EventHandler : public utils::EventHandler {
            public:
                static constexpr std::size_t INVALID_ID = -1;
                explicit EventHandler(std::size_t id);
                
                [[nodiscard]] std::size_t id() const;
        };
        
        
   
        class Callback {
            public:
                Callback(std::type_index event_type);
                virtual ~Callback() = default;
                
                [[nodiscard]] bool operator==(std::size_t id) const;
                
                [[nodiscard]] bool enabled() const;
                void enable();
                void disable();
                
                [[nodiscard]] bool tombstoned() const;
                void tombstone();
                
                [[nodiscard]] std::size_t id() const;
                [[nodiscard]] std::type_index event_type() const;
                
                [[nodiscard]] virtual bool invoke(const void* data) = 0;
    
            private:
                std::type_index m_event_type;
                std::size_t m_id;
                bool m_enabled;
                bool m_tombstoned;
        };
    
        
        
        template <typename E>
        class GlobalFunctionWrapper final : public Callback {
            public:
                using FunctionType = bool (*)(const E*);
        
                explicit GlobalFunctionWrapper(FunctionType function);
                ~GlobalFunctionWrapper() override = default;
        
                [[nodiscard]] bool invoke(const void* data) override;
    
            private:
                FunctionType m_function;
        };
        
        template <typename E>
        GlobalFunctionWrapper<E>::GlobalFunctionWrapper(FunctionType function) : Callback(typeid(E)),
                                                                                 m_function(std::move(function)) {
        }
        
        template <typename E>
        bool GlobalFunctionWrapper<E>::invoke(const void* data) {
            return m_function(reinterpret_cast<const E*>(data));
        }
        
        
        
        template <typename T, typename U, typename E>
        class MemberFunctionWrapper final : public Callback {
            public:
                using FunctionType = bool (U::*)(const E*);
        
                MemberFunctionWrapper(std::shared_ptr<T> instance, FunctionType function);
                MemberFunctionWrapper(T* instance, FunctionType function);
                ~MemberFunctionWrapper() override = default;
        
                [[nodiscard]] bool invoke(const void* data) override;
    
            private:
                FunctionType m_function;
                
                union {
                    std::weak_ptr<T> sp;
                    T* p;
                } m_object;
                bool m_managed;
        };
        
        template <typename T, typename U, typename E>
        MemberFunctionWrapper<T, U, E>::MemberFunctionWrapper(std::shared_ptr<T> instance, FunctionType function) : Callback(typeid(E)),
                                                                                                                    m_function(std::move(function)),
                                                                                                                    m_object(instance),
                                                                                                                    m_managed(true) {
        }
        
        template <typename T, typename U, typename E>
        MemberFunctionWrapper<T, U, E>::MemberFunctionWrapper(T* instance, FunctionType function) : Callback(typeid(E)),
                                                                                                    m_function(std::move(function)),
                                                                                                    m_object(instance),
                                                                                                    m_managed(false) {
        }
        
        template <typename T, typename U, typename E>
        bool MemberFunctionWrapper<T, U, E>::invoke(const void* data) {
            if (m_managed) {
                std::weak_ptr<T> obj = m_object.sp;
                return obj.expired() || std::invoke(obj.lock, m_function, reinterpret_cast<const E*>(data));
            }
            else {
                return std::invoke(m_object.p, m_function, data);
            }
        }
        
        
        
        template <typename E>
        class LambdaWrapper final : public Callback {
            public:
                using LambdaType = std::function<bool(const E*)>;
                
                explicit LambdaWrapper(LambdaType&& lambda);
                ~LambdaWrapper() override = default;
                
                [[nodiscard]] bool invoke(const void* data) override;
                
            private:
                LambdaType m_lambda;
        };
        
        template <typename E>
        LambdaWrapper<E>::LambdaWrapper(LambdaType&& lambda) : Callback(typeid(E)),
                                                               m_lambda(std::move(lambda)) {
        }
        
        template <typename E>
        bool LambdaWrapper<E>::invoke(const void* data) {
            return m_lambda(reinterpret_cast<const E*>(data));
        }
        
        
        
        template <typename T>
        struct is_lambda : std::false_type { };
        
        template <typename E>
        struct is_lambda<std::function<bool(const E*)>> : std::true_type {
            using EventType = E;
        };

        
        
        struct GlobalFunction {
            GlobalFunction(std::shared_ptr<Callback> callback);
            ~GlobalFunction() = default;

            [[nodiscard]] bool operator==(const GlobalFunction& other) const;
            
            std::shared_ptr<Callback> callback;
            std::uintptr_t address;
        };
        
        struct GlobalFunctionHash {
            using is_transparent = void; // Allow heterogeneous lookup
            std::size_t operator()(const GlobalFunction& function) const;
            std::size_t operator()(std::uintptr_t address) const;
        };
        
        struct GlobalFunctionComparator {
            using is_transparent = void; // Allow heterogeneous lookup
            bool operator()(const GlobalFunction& a, const GlobalFunction& b) const;
            bool operator()(const GlobalFunction& a, std::uintptr_t b) const;
            bool operator()(std::uintptr_t a, const GlobalFunction& b) const;
        };
        
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
        
        struct EventHandlerRegistration {
            EventHandlerRegistration(std::type_index event_type, std::size_t index);
            
            std::type_index event_type;
            std::size_t index;
        };
        
        struct Event {
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
                        
                        Event operator*() const;
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
        
        std::shared_ptr<Callback> get_callback(std::size_t id);
        
        void register_new_callback(const std::shared_ptr<Callback>& callback);
        void remove_expired_callbacks();
        
        IdGenerator id_generator;
        std::unordered_map<std::size_t, EventHandlerRegistration> id_to_event_handler_map;
        std::unordered_map<std::type_index, std::vector<std::weak_ptr<Callback>>> callbacks;
        std::unordered_set<GlobalFunction, GlobalFunctionHash, GlobalFunctionComparator> global_function_registrations;
        
        // Using std::vector here instead of std::unordered_map<std::type_index, std::shared_ptr<Callback>> as the number of callbacks per object is expected to be relatively low
        // such that performing a linear scan of the callbacks per object will be more efficient than a hash + lookup operation
        std::unordered_map<std::uintptr_t, std::vector<std::shared_ptr<Callback>>> member_function_registrations;
        
        EventQueue event_queue;
    }
    
    template <typename T, typename U, typename E>
    EventHandler register_event_handler(std::shared_ptr<T> instance, bool (U::*function)(const E*)) {
        // Always use the underlying object pointer
        return register_event_handler(instance.get(), function);
    }

    template <typename T, typename U, typename E>
    EventHandler register_event_handler(T* instance, bool (U::*function)(const E*)) {
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

        if (!instance) {
            return detail::EventHandler(detail::EventHandler::INVALID_ID);
        }
        if (!function) {
            return detail::EventHandler(detail::EventHandler::INVALID_ID);
        }

        std::shared_ptr<detail::Callback> callback;
        
        // This operation will either register a new member function callback or be a noop (if the callback already exists)
        // Either way, using std::unordered_map::operator[] will not result in unnecessary memory overhead
        std::vector<std::shared_ptr<detail::Callback>>& callbacks = detail::member_function_registrations[instance];
        for (const std::shared_ptr<detail::Callback>& c : callbacks) {
            if (c == function) {
                // Found existing registration in the system for this function
                callback = c;
                break;
            }
        }

        if (!callback) {
            // Existing registration was not found, create a new one
            callback = callbacks.emplace_back(std::make_shared<detail::MemberFunctionWrapper>(instance, function));
            detail::register_new_callback(callback);
        }
        
        return detail::EventHandler(callback->id());
    }

    template <typename E>
    EventHandler register_event_handler(bool (*function)(const E*)) {
        if (!function) {
            return detail::EventHandler(detail::EventHandler::INVALID_ID);
        }

        std::shared_ptr<detail::Callback> callback;
        
        auto it = detail::global_function_registrations.find(function);
        if (it != detail::global_function_registrations.end()) {
            // Found existing registration in the system for this function
            callback = it->second.function;
        }
        else {
            callback = detail::global_function_registrations.emplace(std::make_shared<detail::GlobalFunctionWrapper<E>>(function));
            detail::register_new_callback(callback);
        }
        
        return detail::EventHandler(callback->id());
    }
    
    template <typename Fn>
    EventHandler register_event_handler(Fn&& function) {
        using namespace detail;
        std::size_t id = detail::EventHandler::INVALID_ID;
        
        // There aren't many safeguards here...
        if constexpr (is_lambda<Fn>::value) {
            using E = is_lambda<Fn>::EventType;
            std::shared_ptr<Callback> callback = global_function_registrations.emplace(std::make_shared<LambdaWrapper<E>>(function));
            register_new_callback(callback);
            id = callback->id();
        }
        
        return detail::EventHandler(id);
    }

    template <typename T, typename U, typename E>
    void deregister_event_handler(std::shared_ptr<T> instance, bool (U::*function)(const E*)) {
        // Always use the underlying object pointer
        deregister_event_handler(instance.get(), function);
    }

    template <typename T>
    void deregister_event_handler(std::shared_ptr<T> instance) {
        // Always use the underlying object pointer
        deregister_event_handler(instance.get());
    }

    template <typename T, typename U, typename E>
    void deregister_event_handler(T* instance, bool (U::*function)(const E*)) {
        if (!instance) {
            return;
        }
        if (!function) {
            return;
        }
        
        auto it = detail::member_function_registrations.find(instance);
        if (it == detail::member_function_registrations.end()) {
            return;
        }
        
        std::vector<std::shared_ptr<detail::Callback>>& callbacks = it->second;
        for (std::size_t i = 0; i < callbacks.size(); ++i) {
            if (callbacks[i] == function) {
                callbacks.erase(callbacks.begin() + i);
                break;
            }
        }
    }
    
    template <typename T>
    void deregister_event_handler(T* instance) {
        using namespace detail;
        if (!instance) {
            return;
        }
        
        auto it = member_function_registrations.find(instance);
        if (it == member_function_registrations.end()) {
            return;
        }
        
        member_function_registrations.erase(it);
    }
    
    template <typename E>
    void deregister_event_handler(bool (*function)(const E*)) {
        using namespace detail;
        if (!function) {
            return;
        }
        
        auto it = global_function_registrations.find(function);
        if (it == global_function_registrations.end()) {
            return;
        }
        
        global_function_registrations.erase(it);
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