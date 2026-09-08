#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <sstream>
#include <stdexcept>

namespace pixelforge::test {

struct TestCase {
    std::string suite;
    std::string name;
    std::function<void()> func;
};

class TestRegistry {
public:
    static TestRegistry& instance() {
        static TestRegistry reg;
        return reg;
    }

    void add_test(std::string suite, std::string name, std::function<void()> func) {
        tests_.push_back({std::move(suite), std::move(name), std::move(func)});
    }

    int run_all() {
        int passed = 0;
        int failed = 0;

        std::cout << "\n=======================================================\n";
        std::cout << "          PIXELFORGE AI - UNIT TEST SUITE             \n";
        std::cout << "=======================================================\n\n";

        for (const auto& t : tests_) {
            std::cout << "[ RUN      ] " << t.suite << "." << t.name << "\n";
            try {
                t.func();
                std::cout << "\033[32m[       OK ]\033[0m " << t.suite << "." << t.name << "\n";
                passed++;
            } catch (const std::exception& e) {
                std::cout << "\033[31m[  FAILED  ]\033[0m " << t.suite << "." << t.name << "\n"
                          << "             Exception: " << e.what() << "\n";
                failed++;
            } catch (...) {
                std::cout << "\033[31m[  FAILED  ]\033[0m " << t.suite << "." << t.name << "\n"
                          << "             Unknown exception caught\n";
                failed++;
            }
        }

        std::cout << "\n-------------------------------------------------------\n";
        std::cout << "TOTAL: " << tests_.size()
                  << " | \033[32mPASSED: " << passed << "\033[0m"
                  << " | " << (failed > 0 ? "\033[31m" : "\033[32m")
                  << "FAILED: " << failed << "\033[0m\n";
        std::cout << "=======================================================\n\n";

        return (failed == 0) ? 0 : 1;
    }

private:
    std::vector<TestCase> tests_;
};

struct TestRegistrar {
    TestRegistrar(std::string suite, std::string name, std::function<void()> func) {
        TestRegistry::instance().add_test(std::move(suite), std::move(name), std::move(func));
    }
};

#define TEST(suite, name) \
    static void test_##suite##_##name(); \
    static ::pixelforge::test::TestRegistrar reg_##suite##_##name(#suite, #name, test_##suite##_##name); \
    static void test_##suite##_##name()

#define ASSERT_TRUE(cond) \
    do { \
        if (!(cond)) { \
            std::ostringstream ss; \
            ss << "Assertion failed: (" #cond ") at " << __FILE__ << ":" << __LINE__; \
            throw std::runtime_error(ss.str()); \
        } \
    } while (0)

#define ASSERT_FALSE(cond) ASSERT_TRUE(!(cond))

#define ASSERT_EQ(a, b) \
    do { \
        if ((a) != (b)) { \
            std::ostringstream ss; \
            ss << "Assertion failed: (" #a " == " #b ") [" << (a) << " != " << (b) << "] at " << __FILE__ << ":" << __LINE__; \
            throw std::runtime_error(ss.str()); \
        } \
    } while (0)

#define ASSERT_NE(a, b) \
    do { \
        if ((a) == (b)) { \
            std::ostringstream ss; \
            ss << "Assertion failed: (" #a " != " #b ") at " << __FILE__ << ":" << __LINE__; \
            throw std::runtime_error(ss.str()); \
        } \
    } while (0)

} // namespace pixelforge::test
