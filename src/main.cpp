
#include "utils/result.hpp"
#include "utils/string.hpp"
#include "utils/constexpr.hpp"
#include <iostream>
#include <vector>
#include <chrono>
#include "utils/logging/logging.hpp"
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
    
    std::vector<int> v { 1, 2, 3, 4, 5 };
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
    
    using namespace detail;
    
//    Formatting f { };
//    f["representation"] = "fixed";
//    f["sign"] = "aligned";
//    f["group_size"] = "6";
//    f["use_base_prefix"] = "true";
//    f["width"] = "20";
//    f["justification"] = "center";
//    f["use_separator"] = "true";
//    // f["precision"] = "30";
//

    std::cout << detail::is_formattable_to<std::source_location> << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
//        std::string r = utils::format("this is a test format string containing a vector: {:specifier=[value]:|representation=[hexadecimal],use_base_prefix=[true]:representation=[binary]|:}, called from: {}", v, std::source_location::current());
        std::string r = utils::format("{{ {{ asdf hehe {asdf} {{ {0:specifier=[value]} {0:specifier=[different]} k", std::source_location::current(), NamedArgument("asdf", std::source_location::current()));
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count() << std::endl;

    std::cout << r << std::endl;
    
//
//    std::shared_ptr<logging::Adapter> adapter = logging::get_adapter("stdout");
//    adapter->info("asdf", 4);
    
//    std::string a = format("test ", arg("test", 2), b, pair, nullptr, c, tup);
    
    return 0;
}
