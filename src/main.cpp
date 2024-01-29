
#include "utils/result.hpp"
#include "utils/string.hpp"
#include "utils/constexpr.hpp"
#include <iostream>

int main() {
    using namespace utils;
    
    std::vector<int> b { 1, 2, 3, 4, 5 };
    int* c = nullptr;
    
    std::pair<int, char> pair = std::make_pair(1, 'a');
    std::tuple<std::string, int, float> tup = std::make_tuple("test string", 56, 9.8f);
    
    std::string a = format("test", arg("test", 2), b, pair, nullptr, c, tup);
    
    return 0;
}