
#include "utils/result.hpp"
#include "utils/constexpr.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include "utils/logging/adapter.hpp"
#include <source_location>

struct MyContainer {
    std::vector<int> raw;

    std::vector<int>::const_iterator begin() const;
    std::vector<int>::const_iterator end() const;
};

namespace mynamespace {
    
    struct MyType {
    
    
    };
    
    [[nodiscard]] std::string to_string(const MyType& t) {
        return "custom type no formatting";
    }
    
}

int main() {
    using namespace utils;
//    int* c = nullptr;
//
//    if constexpr (detail::is_deconstructible<std::source_location>) {
//        std::cout << "Type is deconstructible." << std::endl;
//    } else {
//        std::cout << "Type is not deconstructible." << std::endl;
//    }
//
//    utils::push_format_override<std::source_location>("this is a custom formatter {filepath}");
//
//    std::pair<bool, char> pair = std::make_pair(true, 'a');
//    std::tuple<std::string, int, float, int> tup = std::make_tuple("test string", 56, 9.8f, 1);
//
//    try {
////        std::string a = format("test {{ as }} df {0} {hello}{hello} {{}} adf  '", 9);
////        std::string a = format("test {{ as }} df {0} {1}{3} {{}} adf  '", 9, arg("asdf", 9), arg("asdf", 9), 9);
//
//        auto start = std::chrono::high_resolution_clock::now();
////            std::string f = std::format("this is a test {}", 5);
//            std::string f = utils::format("this is a test {}", 5);
//            // std::string a = utils::format("testing value: {0} {0|representation=[binary]}", v);
//        auto end = std::chrono::high_resolution_clock::now();
//
//        std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << std::endl;
//
//        std::cout << f << std::endl;
//
//        std::string c = utils::format("testing value: {0|width=[10]}", mynamespace::MyType { });
//
//    }
//    catch (FormattedError& e) {
//        std::cout << e.what() << std::endl;
//    }
//
//    logging::info("asdf", 1);
//
//    utils::Formatting formatting { };
//
//
//    formatting["a"] = "c";
//    formatting["b"] = "56";
//    formatting["c"] = "5.2";
//    formatting["d"] = "this is a string";
    
//    auto list = to_placeholder_list(std::source_location::current());
    
    // std::cout << from_string<unsigned>("12344123123123123123") << std::endl;

    // std::cout << utils::format("example format string '{:representation=[scientific],precision=[1]}'", std::numeric_limits<double>::max() - 1) << std::endl;
    
//    std::string str = "this is a string";
//    std::string_view strv = "this is a string view";
//    const char* ccp = "ccp";
//    char ca[6] = "hello";
//    const char cca[6] = "hello";
//    char c = 'a';
//
//    std::unordered_map<int, std::string> map = {
//        { 1234, "one" },
//        { 2, "two two" },
//        { 3, "three three three" },
//    };
////
//    Formatter<std::unordered_map<int, std::string>> formatter { };
//    FormatString::Specification spec { };
//    spec[0]["use_base_prefix"] = "1";
//    spec[1]["width"] = "10";
//    formatter.parse(spec);
//    std::cout << formatter.format(map) << std::endl;
//
//     std::cout << utils::format("example format string '{}'", map) << std::endl;

    Formatter<int> formatter { };
    std::string r = formatter.format(42);
    std::cout << r << std::endl;
    
//    utils::format("asdf {}", "adsf");

    std::unordered_map<int, float> a {
        { 1, 1.1f },
        { 2, 2.2f },
        { 3, 3.3f },
        { 4, 4.4f },
    };
    
    std::cout << utils::format("'this is a format string {{ {adf::precision=[2]}' {ad} {{ ", a) << std::endl;
    
//    std::cout << utils::format("{:representation=[decimal],use_separator_character=[1],sign=[both]}", std::numeric_limits<long double>::max()) << std::endl;
    
//    std::cout << utils::format("example format string '{string:width=[15]}'", NamedArgument("string", std::source_location::current())) << std::endl;
//    std::cout << utils::format("example format string '{}'", strv) << std::endl;
//    std::cout << utils::format("example format string '{}'", ccp) << std::endl;
//    std::cout << utils::format("example format string '{}'", ca) << std::endl;
//    std::cout << utils::format("example format string '{}'", cca) << std::endl;
//    std::cout << utils::format("example format string '{}'", c) << std::endl;
    
//    std::size_t amount = 1000000;
//
//    auto start = std::chrono::high_resolution_clock::now();
//    for (std::size_t i = 0u; i < amount; ++i) {
//        std::string r = utils::format("this is a test format string containing a vector: {:specifier=[value [[ ]]]:|representation=[hexadecimal],use_base_prefix=[true]:representation=[binary]|:}, called from: {}", std::source_location::current());
//    }
//    auto end = std::chrono::high_resolution_clock::now();
//    std::cout << "time to format " << amount << ": " << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << std::endl;
//
//    std::vector<std::size_t> durations;
//    durations.reserve(amount);
//
//    for (std::size_t i = 0u; i < amount; ++i) {
//        start = std::chrono::high_resolution_clock::now();
//        std::string r = utils::format("this is a test format string containing a vector: {:specifier=[value [[ ]]]:|representation=[hexadecimal],use_base_prefix=[true]:representation=[binary]|:}, called from: {}", std::source_location::current());
//        // std::string r = utils::format("this is a test format string containing a vector: {:specifier=[value [[ ]]]:|representation=[hexadecimal],use_base_prefix=[true]:representation=[binary]|:}, called from: {}", std::source_location::current());
//        end = std::chrono::high_resolution_clock::now();
//        durations.emplace_back(std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count());
//    }
//
//    std::size_t total = 0;
//    for (auto duration : durations) {
//        total += duration;
//    }
//    std::cout << "average time per format: " << total / amount << std::endl;
    
    return 0;
}
