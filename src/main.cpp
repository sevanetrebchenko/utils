
#include "utils/result.hpp"
#include "utils/string.hpp"
#include "utils/constexpr.hpp"
#include <iostream>
#include <vector>

struct MyContainer {
    std::vector<int> raw;

    std::vector<int>::const_iterator begin() const;
    std::vector<int>::const_iterator end() const;
};

int main() {
    using namespace utils;
    
    std::vector<int> b { 1, 2, 3, 4, 5 };
    int* c = nullptr;
    
    std::pair<bool, char> pair = std::make_pair(true, 'a');
    std::tuple<std::string, int, float, int> tup = std::make_tuple("test string", 56, 9.8f, 1);
    
    auto ocur = count_occurrences<std::string>(tup);
    
    try {
        std::string a = format("test {{ as }} df {0} {hello}{hello} {{}} adf ", 9, arg("hello", true));
        std::cout << a << std::endl;
    }
    catch(std::runtime_error& e) {
        std::cout << e.what() << std::endl;
    }
    
//    std::string a = format("test ", arg("test", 2), b, pair, nullptr, c, tup);
    
    return 0;
}