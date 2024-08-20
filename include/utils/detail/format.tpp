
#pragma once

#ifndef FORMAT_TPP
#define FORMAT_TPP

#include "utils/logging/logging.hpp"
#include "utils/constexpr.hpp"
#include "utils/string.hpp"
#include "utils/tuple.hpp"
#include "utils/assert.hpp"

#include <charconv> // std::to_chars
#include <queue> // std::priority_queue
#include <limits> // std::numeric_limits
#include <optional> // std::optional

namespace utils {

    namespace detail {
        

        
        template <typename T>
        concept is_formattable = requires(Formatter<typename std::decay<T>::type> formatter, const FormatString::Specification& spec, const T& value) {
            { formatter.parse(spec) };
            { formatter.format(value) } -> std::same_as<std::string>;
        };
        
        template <typename T>
        concept is_formattable_to = requires(Formatter<typename std::decay<T>::type> formatter, const T& value, FormattingContext context) {
            { formatter.reserve(value) } -> std::same_as<std::size_t>;
            { formatter.format_to(value, context) };
        };
        
        template <typename T>
        struct PlaceholderFormatter : public Formatter<typename std::decay<T>::type> {
            PlaceholderFormatter() : length(0u),
                                     specification_index(0u),
                                     start(std::numeric_limits<std::size_t>::max()) {
            }
            
            PlaceholderFormatter(std::size_t spec_id) : length(0u),
                                                        specification_index(spec_id),
                                                        start(std::numeric_limits<std::size_t>::max()) {
            }
            
            bool initialized() const {
                return start != std::numeric_limits<std::size_t>::max();
            }
            
            std::size_t length;
            std::size_t specification_index;
            std::size_t start; // start + length is the formatted value
        };
        
        struct PlaceholderIndices {
            std::size_t argument_index;
            std::size_t formatter_index;
        };
        
        int round_up_to_multiple(int value, int multiple);
        

        
    }
    

    
    // Specification implementation
    
    template <typename T, typename ...Ts>
    bool FormatString::Specification::has_specifier(const T& first, const Ts& ...rest) const {
        if (std::holds_alternative<FormattingGroupList>(m_spec)) {
            // TODO: calling has_group on a formatting group list makes no sense, throw exception?
            return false;
        }
        
        auto tuple = detail::make_string_view_tuple(first, rest...);
        bool has_specifier = false;
        
        utils::apply([this, &has_specifier](std::string_view name, std::size_t index) {
            for (const Specifier& specifier : std::get<SpecifierList>(m_spec)) {
                if (casecmp(specifier.name, name)) {
                    has_specifier = true;
                }
            }
        }, tuple);
        
        return has_specifier;
    }
    
    template <typename T>
    NamedArgument<T>::NamedArgument(std::string name, const T& value) : name(std::move(name)),
                                                                        value(value) {
    }

    template <typename T>
    NamedArgument<T>::~NamedArgument() = default;
    
    template <typename ...Ts>
    FormatString format(FormatString fmt, const Ts&... args) {
        return std::move(fmt.format(args...));
    }
    
    template <typename ...Ts>
    FormattedError::FormattedError(FormatString fmt, const Ts&... args) : std::runtime_error(fmt.format(args...)) {
    }
    
    // IntegerFormatter implementation
    

    
}


#endif // FORMAT_TPP