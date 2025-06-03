
#include "utils/events.hpp"
#include "utils/assert.hpp"
#include "utils/detail/events.tpp"
#include <stdexcept> // std::out_of_range

namespace utils {
    namespace detail {
        
        template <typename T>
        bool is_uninitialized(const std::weak_ptr<T>& ptr) {
            return !ptr.owner_before(std::weak_ptr<T>{ }) && !std::weak_ptr<T>{ }.owner_before(ptr);
        }
        
        // Callback implementation
        Callback::~Callback() {
            id_generator.recycle(m_id);
        }
        
        bool Callback::invoke(const EventData& data) {
            return m_function(data);
        }
        
        bool Callback::expired() const {
            if (is_uninitialized(m_object)) {
                // If the object pointer was never initialized, the callback was either created with a raw object pointer (T*) or a global function pointer,
                // which expire unless manually deregistered from the system
                return deregistered();
            }
            
            return m_object.expired() || deregistered();
        }
        
        bool Callback::enabled() const {
            return (m_flags & ENABLED_BIT) != 0;
        }
        
        void Callback::deregister() {
            m_flags |= TOMBSTONED_BIT;
        }
        
        void Callback::enable() {
            m_flags |= ENABLED_BIT;
        }
        
        void Callback::disable() {
            m_flags &= ~ENABLED_BIT;
        }
        
        std::size_t Callback::id() const {
            return m_id;
        }
        
        std::type_index Callback::type() const {
            return m_type;
        }
        
        bool Callback::deregistered() const {
            return (m_flags & TOMBSTONED_BIT) != 0;
        }
        
        // IdGenerator implementation
        IdGenerator::IdGenerator() : m_id(0) { }
        
        void IdGenerator::recycle(std::size_t id) {
            for (auto it = m_intervals.begin(); it != m_intervals.end(); ++it) {
                if (id + 1 == it->start) {
                    it->start = id;
                    merge(it);
                    return;
                }
                
                if (id == it->end + 1) {
                    it->end = id;
                    merge(it);
                    return;
                }
                
                if (id >= it->start && id <= it->end) {
                    // Index is already contained within an existing interval
                    return;
                }
                
                if (id < it->start) {
                    m_intervals.insert(it, { id, id });
                    return;
                }
            }
            
            m_intervals.emplace_back(id, id);
        }
        
        std::size_t IdGenerator::next() {
            if (m_intervals.empty()) {
                return m_id++;
            }
            
            Interval& interval = m_intervals.front();
            std::size_t id = interval.start++;
            
            if (interval.start > interval.end) {
                // Interval consists of one element and should be removed entirely
                m_intervals.erase(m_intervals.begin());
            }
            
            return id;
        }
        
        void IdGenerator::merge(std::vector<Interval>::iterator it) {
            // Merge left
            if (it != m_intervals.begin()) {
                auto previous = std::prev(it);
                if (previous->end + 1 == it->start) {
                    previous->end = it->end;
                    m_intervals.erase(it);
                    it = previous;
                }
            }
            
            // Merge right
            auto next = std::next(it);
            if (next != m_intervals.end() && it->end + 1 == next->start) {
                it->end = next->end;
                m_intervals.erase(next);
            }
        }
        
        // EventQueue implementation
        EventQueue::EventQueue(std::size_t size) : m_data(malloc(size)),
                                                   m_offset(0),
                                                   m_capacity(size),
                                                   m_allocation_count(0) {
        }
        
        void EventQueue::reallocate() {
            // Increase allocator capacity
            void* new_data = malloc(m_capacity);
            
            std::byte* dst = reinterpret_cast<std::byte*>(new_data);
            std::byte* src = reinterpret_cast<std::byte*>(m_data);
            std::byte* last_copied_from = src;
            
            // Copy over elements + allocation metadata
            // Consecutive allocations that are trivially copyable are copied over in bulk
            for (std::size_t i = 0; i < m_allocation_count; ++i) {
                const Allocation& allocation = *reinterpret_cast<Allocation*>(src);
                std::size_t total_size = sizeof(Allocation) + allocation.size;
                
                bool is_trivial = allocation.copy_op == Allocation::CopyConstructorType::None;
                
                if (!is_trivial) {
                    // Bulk copy any trivially copyable elements before this allocation
                    if (src != last_copied_from) {
                        std::size_t block_size = src - last_copied_from;
                        std::memcpy(dst, last_copied_from, block_size);
                        dst += block_size;
                    }
                    
                    // Copy allocation header
                    std::memcpy(dst, src, sizeof(Allocation));
                    
                    // Copy block
                    std::size_t offset = sizeof(Allocation);
                    switch (allocation.copy_op) {
                        case Allocation::CopyConstructorType::Copy:
                            allocation.copy_constructor.copy(dst + offset, src + offset);
                            break;
                        case Allocation::CopyConstructorType::Move:
                            allocation.copy_constructor.move(dst + offset, src + offset);
                            break;
                    }
                    
                    dst += total_size;
                    last_copied_from = src + total_size;
                }
                // Simply step over trivially-copyable blocks, as batch memcpy will copy the data of this block later
                // else { ... }
                
                src += total_size;
            }
            
            // Copy any trailing trivially-copyable blocks
            if (src != last_copied_from) {
                std::size_t block_size = src - last_copied_from;
                std::memcpy(dst, last_copied_from, block_size);
                // dst += block_size;
            }
            
            free(m_data);
            m_data = new_data;
        }
        
        void EventQueue::reset() {
            std::byte* base = reinterpret_cast<std::byte*>(m_data);
            
            for (std::size_t i = 0; i < m_allocation_count; ++i) {
                const Allocation& allocation = *reinterpret_cast<Allocation*>(base);
                
                if (allocation.destructor) {
                    // Non-trivially destructible type, need to call destructor for this object before resetting allocator
                    allocation.destructor(base + sizeof(Allocation));
                }
                
                // Step to next block
                base += sizeof(base) + allocation.size;
            }
            
            // Reset allocator internals
            m_allocation_count = 0;
            m_offset = 0;
        }
        
        // Note: begin() and end() functions should return the same pointer for empty containers
        EventQueue::ForwardIterator EventQueue::begin() {
            return m_data;
        }
        EventQueue::ForwardIterator EventQueue::end() {
            return reinterpret_cast<std::byte*>(m_data) + m_offset; // Points to one past the last element
        }

        // EventQueue::ForwardIterator implementation
        EventQueue::ForwardIterator::ForwardIterator(void* base) : m_base(base) {
        }
        
        EventData EventQueue::ForwardIterator::operator*() const {
            const EventQueue::Allocation& allocation = *reinterpret_cast<EventQueue::Allocation*>(m_base);
            return {
                .data = reinterpret_cast<std::byte*>(m_base) + sizeof(Allocation), // Return a pointer to the data segment
                .type = allocation.type
            };
        }
        
        EventQueue::ForwardIterator& EventQueue::ForwardIterator::operator++() {
            const EventQueue::Allocation& allocation = *reinterpret_cast<EventQueue::Allocation*>(m_base);
            m_base = reinterpret_cast<std::byte*>(m_base) + sizeof(Allocation) + allocation.size;
            return *this;
        }
        
        bool EventQueue::ForwardIterator::operator!=(const EventQueue::ForwardIterator& other) const {
            return m_base == other.m_base;
        }

        CallbackHandle get_callback(std::size_t address, std::size_t id) {
            auto it = callback_registrations.find(address);
            
            // The assumption here is that the number of callbacks per object is relatively low, so performing a linear scan is faster than a hash + lookup operation
            if (it == callback_registrations.end()) {
                return nullptr;
            }
            
            CallbackRegistration& registration = it->second;
            
            if (std::holds_alternative<std::monostate>(registration)) {
                return nullptr;
            }
            else if (std::holds_alternative<CallbackHandle>(registration)) {
                const CallbackHandle& callback = std::get<CallbackHandle>(registration);
                return callback->id() == id ? callback : nullptr;
            }
            else { // if (std::holds_alternative<std::vector<CallbackHandle>>(registration))
                const std::vector<CallbackHandle>& callbacks = std::get<std::vector<CallbackHandle>>(registration);
                for (const CallbackHandle& callback : callbacks) {
                    if (callback->id() == id) {
                        return callback;
                    }
                }
                
                return nullptr;
            }
        }
        
        void remove_callback_registration(std::uintptr_t address) {
            auto it = callback_registrations.find(address);
            if (it == callback_registrations.end()) {
                // No callback registrations exist for the given object
                return;
            }
            
            callback_registrations.erase(it);
        }
        
    }
    
    // EventHandler implementation
    EventHandler::EventHandler(std::uintptr_t address, std::size_t id) : m_address(address),
                                                                         m_id(id) {
    }
    
    EventHandler::EventHandler() : m_address(-1),
                                   m_id(-1) {
    }
    
    void EventHandler::enable() const {
        using namespace detail;
        CallbackHandle callback = get_callback(m_address, m_id);
        if (callback) {
            callback->enable();
        }
    }
    
    void EventHandler::disable() const {
        using namespace detail;
        CallbackHandle callback = get_callback(m_address, m_id);
        if (callback) {
            callback->disable();
        }
    }
    
    bool EventHandler::enabled() const {
        using namespace detail;
        CallbackHandle callback = get_callback(m_address, m_id);
        if (callback) {
            return callback->enabled();
        }
        
        return false;
    }
    
    void EventHandler::deregister() const {
        using namespace detail;
        CallbackHandle callback = get_callback(m_address, m_id);
        if (callback) {
            callback->deregister();
        }
    }
    
    // Event system helper functions
    void remove_expired_callbacks(detail::CallbackRegistration& registration) {
        using namespace detail;
        
        if (std::holds_alternative<CallbackHandle>(registration)) {
            const CallbackHandle& callback = std::get<CallbackHandle>(registration);
            if (callback->expired()) {
                registration = std::monostate { };
            }
        }
        else if (std::holds_alternative<std::vector<CallbackHandle>>(registration)) {
            std::vector<CallbackHandle>& callbacks = std::get<std::vector<CallbackHandle>>(registration);
            std::erase_if(callbacks, [](const CallbackHandle& callback) -> bool {
                return callback->expired();
            });
        }
        // Nothing to do if variant holds std::monostate
        // else { ... }
    }
    
    void process_events() {
        using namespace detail;
        
        // Ensure that only valid callbacks are invoked with event data
        for (auto& [address, callbacks] : callback_registrations) {
            remove_expired_callbacks(callbacks);
        }
        
        // Remove references to expired callbacks
        for (auto& [type, callbacks] : dispatch_map) {
            std::erase_if(callbacks, [](const std::weak_ptr<Callback>& callback) -> bool {
                return callback.expired();
            });
        }
        
        // Dispatch enqueued events
        for (const EventData& event : event_queue) {
            auto it = dispatch_map.find(event.type);
            if (it == dispatch_map.end()) {
                continue;
            }
            
            for (const std::weak_ptr<Callback>& callback : it->second) {
                ASSERT(!callback.expired(), "invoking expired callback");
                CallbackHandle c = callback.lock();

                if (!c->enabled()) {
                    // Do not invoke disabled callbacks
                    continue;
                }
                
                if (!c->invoke(event)) {
                    // An EventHandler returns false to stop event propagation
                    break;
                }
            }
        }
        
        // Reset allocator for the next frame
        event_queue.reset();
    }
    
}