
#include "utils/events.hpp"
#include "utils/assert.hpp"
#include "utils/detail/events.tpp"

namespace utils {
    
    void EventHandler::enable() const {
        std::shared_ptr<detail::Callback> callback = detail::get_callback(m_id);
        if (callback) {
            callback->enable();
        }
    }
    
    void EventHandler::disable() const {
        std::shared_ptr<detail::Callback> callback = detail::get_callback(m_id);
        if (callback) {
            callback->enable();
        }
    }
    
    bool EventHandler::enabled() const {
        std::shared_ptr<detail::Callback> callback = detail::get_callback(m_id);
        if (callback) {
            return callback->enabled();
        }
        
        return false;
    }
    
    void EventHandler::deregister() const {
        std::shared_ptr<detail::Callback> callback = detail::get_callback(m_id);
        if (callback) {
            callback->tombstone();
        }
    }
    
    EventHandler::EventHandler(std::size_t id) : m_id(id) { }
    
    void process_events() {
        using namespace detail;
        for (const Event& event : event_queue) {
            auto it = callbacks.find(event.type);
            if (it == callbacks.end()) {
                continue;
            }
            
            for (const std::weak_ptr<Callback>& callback : it->second) {
                if (callback.expired()) {
                    continue;
                }
                
                std::shared_ptr<Callback> c = callback.lock();
                if (c->tombstoned()) {
                    continue;
                }
                
                if (!c->invoke(event.data)) {
                    // An EventHandler returns false to stop event propagation
                    break;
                }
            }
        }
        
        // Reset allocator for the next frame
        event_queue.reset();
    }
    
    namespace detail {
        
        Callback::Callback(std::type_index event_type) : m_event_type(event_type),
                                                         m_id(detail::id_generator.next()),
                                                         m_enabled(true),
                                                         m_tombstoned(false) {
        }
        
        bool Callback::operator==(std::uintptr_t id) const {
            return m_id == id;
        }
        
        bool Callback::enabled() const {
            return m_enabled;
        }
        
        void Callback::enable() {
            m_enabled = true;
        }
        
        void Callback::disable() {
            m_enabled = false;
        }
        
        bool Callback::tombstoned() const {
            return m_tombstoned;
        }
        
        void Callback::tombstone() {
            m_tombstoned = true;
        }
        
        std::size_t Callback::id() const {
            return m_id;
        }
        
        std::type_index Callback::event_type() const {
            return m_event_type;
        }
        
        std::shared_ptr<Callback> get_callback(std::size_t id) {
            auto it = detail::id_to_event_handler_map.find(id);
            if (it == detail::id_to_event_handler_map.end()) {
                return nullptr;
            }
            
            const detail::EventHandlerRegistration& handler = it->second;
            std::weak_ptr<detail::Callback> callback = detail::callbacks[handler.event_type][handler.index];
            if (callback.expired()) {
                return nullptr;
            }
            
            return callback.lock();
        }
        
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
        
        void remove_expired_callbacks() {
            // Remove expired callback registrations
            std::erase_if(global_function_registrations, [](const GlobalFunction& function) -> bool {
                return function.callback->tombstoned();
            });
            for (auto it = member_function_registrations.begin(); it != member_function_registrations.end(); ++it) {
                std::erase_if(it->second, [](const std::shared_ptr<Callback>& callback) -> bool {
                    return callback->tombstoned();
                });
            }
            
            for (auto& [event_type, callback_list] : callbacks) {
                // Remove references to expired callbacks
                std::erase_if(callback_list, [](const std::weak_ptr<Callback>& callback) -> bool {
                    return callback.expired();
                });
                
                // Update valid callback indices
                for (std::size_t i = 0; i < callback_list.size(); ++i) {
                    std::shared_ptr<Callback> callback = callback_list[i].lock();
                    
                    auto it = id_to_event_handler_map.find(callback->id());
                    ASSERT(it != id_to_event_handler_map.end(), "invalid EventHandler mapping");
                    
                    it->second.index = i;
                }
            }
        }
        
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
            if (m_id == detail::EventHandler::INVALID_ID) {
                // std::size_t maximum value is reserved
                throw std::runtime_error("");
            }
            
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

        void register_new_callback(const std::shared_ptr<Callback>& callback) {
            std::size_t id = callback->id();
            std::type_index event_type = callback->event_type();
            std::vector<std::weak_ptr<Callback>> cb = callbacks[event_type];
            
            std::size_t index = cb.size();
            cb.emplace_back(callback);
            
            detail::id_to_event_handler_map.emplace(id, EventHandlerRegistration(event_type, index));
        }
        
        
        EventQueue::ForwardIterator::ForwardIterator(void* base) : m_base(base) {
        }
        
        Event EventQueue::ForwardIterator::operator*() const {
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
        
        GlobalFunction::GlobalFunction(std::shared_ptr<Callback> callback) : callback(std::move(callback)),
                                                                             address(reinterpret_cast<uintptr_t>(this->callback.get())) {
            
        }
        
        bool GlobalFunction::operator==(const GlobalFunction& other) const {
            return address == other.address;
        }
        
        std::size_t GlobalFunctionHash::operator()(const GlobalFunction& function) const {
            return std::hash<std::uintptr_t>{ }(function.address);
        }
        
        std::size_t GlobalFunctionHash::operator()(std::uintptr_t address) const {
            return std::hash<std::uintptr_t>{ }(address);
        }
        
        bool GlobalFunctionComparator::operator()(const GlobalFunction& a, const GlobalFunction& b) const {
            return a.address < b.address;
        }
        
        bool GlobalFunctionComparator::operator()(const GlobalFunction& a, std::uintptr_t b) const {
            return a.address < b;
        }
        
        bool GlobalFunctionComparator::operator()(std::uintptr_t a, const GlobalFunction& b) const {
            return a < b.address;
        }
        
    }
    
}