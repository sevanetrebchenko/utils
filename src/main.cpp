
#include "utils/result.hpp"
#include "utils/string.hpp"
#include "utils/constexpr.hpp"
#include <iostream>
#include <vector>
#include <chrono>

struct MyContainer {
    std::vector<int> raw;

    std::vector<int>::const_iterator begin() const;
    std::vector<int>::const_iterator end() const;
};

int main() {
    using namespace utils;
    
    std::vector<int> v { 1, 2, 3, 4, 5 };
    int* c = nullptr;
    
    std::pair<bool, char> pair = std::make_pair(true, 'a');
    std::tuple<std::string, int, float, int> tup = std::make_tuple("test string", 56, 9.8f, 1);
    
    auto ocur = count_occurrences<std::string>(tup);
    
    try {
//        std::string a = format("test {{ as }} df {0} {hello}{hello} {{}} adf  '", 9);
//        std::string a = format("test {{ as }} df {0} {1}{3} {{}} adf  '", 9, arg("asdf", 9), arg("asdf", 9), 9);

        auto start = std::chrono::high_resolution_clock::now();
            std::string a = format("testing value: {: > #15,.9f}", 3123123412.14159265358979323846);
            std::string b = format("testing value: {: > #15,.9f}", -3435314123.14159265358979323846);
        auto end = std::chrono::high_resolution_clock::now();
        
        std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << std::endl;
        
        std::cout << a << std::endl;
        std::cout << b << std::endl;
    }
    catch (FormatError& e) {
        std::cout << e.what() << std::endl;
    }
    
//    std::string a = format("test ", arg("test", 2), b, pair, nullptr, c, tup);
    
    return 0;
}
