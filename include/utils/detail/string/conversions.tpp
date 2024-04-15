
#pragma once

#ifndef UTILS_CONVERSIONS_TPP
#define UTILS_CONVERSIONS_TPP

#include <sstream> // std::stringstream

namespace utils {

    template <typename T, typename S>
    std::string utils::to_string(const std::pair<T, S>& value, const Formatting& formatting) {
        return "[" + to_string(value.first, formatting) + ", " + to_string(value.second, formatting) + "]";
    }
    
    template <typename... Ts>
    std::string utils::to_string(const std::tuple<Ts...>& value, const Formatting& formatting) {
        std::string result = "[ ";

        // Use std::apply + fold expression to iterate over and format the elements of the tuple.
        std::apply([&result, &formatting](const auto&&... args) {
            ((result.append(to_string(args, formatting) + ", ")), ...);
        }, value);

        // Overwrite the trailing ", " with " ]".
        std::size_t length = result.length();
        result[length - 2u] = ' ';
        result[length - 1u] = ']';
        return std::move(result);
    }
    
    template <typename T>
    std::string utils::to_string(const T& value, const Formatting& formatting) requires is_const_iterable<T> {
        std::stringstream builder { };
        
        builder << "[ ";
        
        auto iter = value.begin();
        builder << to_string(*iter, formatting);
        
        for (++iter; iter != value.end(); ++iter) {
            builder << ", ";
            builder << to_string(*iter, formatting);
        }

        builder << " ]";
        
        if (formatting["representation"] == "binary") {
            builder << " to binary!";
        }
        
        return std::move(builder.str());
    }
    
    template <typename T>
    std::string utils::to_string(const T* value, const Formatting& formatting) {
        return std::string();
    }
    
    template <typename T>
    std::string utils::to_string(const NamedArgument<T>& value, const Formatting& formatting) {
        return to_string(value.value, formatting);
    }
    

}

#endif // UTILS_CONVERSIONS_TPP