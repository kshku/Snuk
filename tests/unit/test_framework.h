#pragma once

#include <stdbool.h>

#include <logger.h>
#include <memory.h>

typedef bool (*snuk_test_fn)(void);

typedef struct SnukTest {
    const char *name;
    snuk_test_fn fn;
} SnukTest;

#if defined(__GNUC__) || defined(__clang__)
    #define SNUK_TEST_SECTION __attribute__((used, section("snuk_tests")))

    extern SnukTest __start_snuk_tests;
    extern SnukTest __stop_snuk_tests;

    #define SNUK_TEST_BEGIN() (&__start_snuk_tests)
    #define SNUK_TEST_END() (&__stop_snuk_tests)
#elif defined(_MSC_VER)
    #pragma section("snuk_tests$a", read)
    #pragma section("snuk_tests$m", read)
    #pragma section("snuk_tests$z", read)

    __declspec(allocate("snuk_tests$a")) static SnukTest __snuk_tests_start = {0};
    __declspec(allocate("snuk_tests$z")) static SnukTest __snuk_tests_end = {0};

    #pragma comment(linker, "/include:__snuk_tests_start")
    #pragma comment(linker, "/include:__snuk_tests_end")

    #define SNUK_TEST_SECTION __declspec(allocate("snuk_tests$m"))

    #define SNUK_TEST_BEGIN() (&__snuk_tests_start + 1)
    #define SNUK_TEST_END()   (&__snuk_tests_end)
#else
    #error "Unsupported compiler :("
#endif

#define ADD_TEST(fn) \
    static bool fn(void); \
    static const SnukTest snuk_test_##fn SNUK_TEST_SECTION = {#fn, fn}; \
    static bool fn(void)

static inline bool snuk_run_all_tests(void) {
    bool failed = false;

    SnukTest *begin = SNUK_TEST_BEGIN();
    SnukTest *end = SNUK_TEST_END();

    for (SnukTest *it = begin; it < end; ++it) {
        // Skip MSVC sentinels */
        if (!it->fn) continue;

        log_info("Running test: %s", it->name);

        if (!it->fn()) {
            log_error("Test failed: %s", it->name);
            failed = true;
        } else {
            log_info("Test passed: %s", it->name);
        }
    }

    return failed;
}

#define RUN_ALL_TESTS_WITH_MEM_SIZE(mem_size) \
    int main(void) { \
        snuk_logger_init(); \
        snuk_memory_init(mem_size); \
        bool failed = snuk_run_all_tests(); \
        snuk_memory_deinit(); \
        snuk_logger_deinit(); \
        return (int)failed; \
    }

#define RUN_ALL_TESTS() RUN_ALL_TESTS_WITH_MEM_SIZE(MIB(1))

#define SNUK_TEST_ASSERT(x) \
    do { \
        if (!(x)) { \
            log_error("Assertion failed: %s (%s:%d) in %s", #x, __FILE__, __LINE__, __func__); \
            return false; \
        } \
    } while (0)

#define SNUK_TEST_ASSERT_EQ(a, b) \
    do { \
        if ((a) != (b)) { \
            log_error("Assertion failed: %s == %s (%s:%d) in %s", #a, #b, __FILE__, __LINE__, __func__); \
            return false; \
        } \
    } while (0)
