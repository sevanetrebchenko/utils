
#ifndef DATETIME_TPP
#define DATETIME_TPP

namespace utils {

    template <typename T>
    Duration::Duration(const std::chrono::duration<T>& duration) : Duration(std::chrono::duration_cast<std::chrono::milliseconds>(duration)) {
    }
    
    template <typename T>
    Duration::operator std::chrono::duration<T>() const {
        using namespace std::chrono;
        
        std::chrono::duration<T> total = std::chrono::duration_cast<std::chrono::duration<T>>(days * 24h);
        total += std::chrono::duration_cast<std::chrono::duration<T>>(hours * 1h);
        total += std::chrono::duration_cast<std::chrono::duration<T>>(minutes * 1min);
        total += std::chrono::duration_cast<std::chrono::duration<T>>(seconds * 1s);
        total += std::chrono::duration_cast<std::chrono::duration<T>>(milliseconds * 1ms);

        return total;
    }

}

#endif // DATETIME_TPP