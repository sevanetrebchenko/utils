
#include "utils/string/formatting.hpp"

namespace utils {
    
    Formatting::Formatting() : justification(Justification::Left),
                               sign(Sign::NegativeOnly),
                               representation(Representation::Decimal),
                               precision(0),
                               width(0),
                               fill(0),
                               separator(0),
                               use_base_prefix(false),
                               group_size(0),
                               next(nullptr) {
    }
    
}