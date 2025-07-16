#ifndef CATCH_HPP
#define CATCH_HPP
#include <vector>
#include <functional>
#include <string>
#include <iostream>
#include <stdexcept>

struct CatchTestCase {
    const char* name;
    std::function<void()> func;
};

inline std::vector<CatchTestCase>& catchTests() {
    static std::vector<CatchTestCase> tests;
    return tests;
}

struct CatchAutoReg {
    CatchAutoReg(const char* name, std::function<void()> func) {
        catchTests().push_back({name, func});
    }
};

#define CATCH_INTERNAL_TEST2(counter, name) \
    static void test_func_##counter(); \
    static CatchAutoReg auto_reg_##counter(name, test_func_##counter); \
    static void test_func_##counter()

#define CATCH_INTERNAL_TEST(counter, name) CATCH_INTERNAL_TEST2(counter, name)

#define TEST_CASE(name) CATCH_INTERNAL_TEST(__COUNTER__, name)

#define REQUIRE(cond) \
    do { if(!(cond)) { \
        std::cerr << "FAILED: " << #cond << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        throw std::runtime_error("test failed"); \
    } } while(0)

#ifdef CATCH_CONFIG_MAIN
int main() {
    int failures = 0;
    for(auto& t : catchTests()) {
        try {
            t.func();
            std::cout << "PASSED: " << t.name << std::endl;
        } catch(const std::exception& e) {
            std::cerr << "FAILED: " << t.name << " - " << e.what() << std::endl;
            failures++;
        }
    }
    return failures;
}
#endif

#endif // CATCH_HPP
