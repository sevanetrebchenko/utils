
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
    std::tuple<std::string, int, float> tup = std::make_tuple("test string", 56, 9.8f);
    
//    std::string a = format("test", arg("test", true));
    std::string a = format("test {format specifier:}", arg("test", 2), b, pair, nullptr, c, tup);
    
    std::vector<std::string> cont {
        "test",
        "main"
    };
    
    std::vector<std::string> comps = split(":adf", ":");
    
    arg t = arg("name", 6);
    std::string s = std::string(t);
    std::cout << std::is_convertible<decltype(t), std::string>::value << std::endl;

    return 0;
}