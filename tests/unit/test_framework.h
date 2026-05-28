#pragma once

#include <snuk/logger.h>
#include <snuk/memory.h>
#include <stdbool.h>
#include <string.h>

typedef bool (*snuk_test_fn)(void);

typedef struct SnukTest {
    const char *name;
    snuk_test_fn fn;
} SnukTest;

#if defined(__APPLE__) && defined(__MACH__)
    #include <mach-o/dyld.h>
    #include <mach-o/getsect.h>

    #define SNUK_TEST_SECTION __attribute__((used, section("__DATA,snuk_tests")))

static inline SnukTest **snuk_macos_section_begin(size_t *count) {
    unsigned long size = 0;

    const struct mach_header_64 *header = (const struct mach_header_64 *)_dyld_get_image_header(0);

    SnukTest **data = (SnukTest **)getsectiondata(header, "__DATA", "snuk_tests", &size);

    *count = size / sizeof(SnukTest);
    return data;
}

    #define SNUK_TEST_BEGIN_COUNT(count) snuk_macos_section_begin(&(count))
#elif defined(__GNUC__) || defined(__clang__)
    #if defined(__APPLE__) && defined(__MACH__)
        #define SNUK_TEST_SECTION __attribute__((used, section("__DATA,snuk_tests")))
    #else
        #define SNUK_TEST_SECTION __attribute__((used, section("snuk_tests")))
    #endif

extern SnukTest *__start_snuk_tests;
extern SnukTest *__stop_snuk_tests;

    #define SNUK_TEST_BEGIN() (&__start_snuk_tests)
    #define SNUK_TEST_END() (&__stop_snuk_tests)
#elif defined(_MSC_VER)
    #pragma section("snuk_tests$a", read)
    #pragma section("snuk_tests$m", read)
    #pragma section("snuk_tests$z", read)

__declspec(allocate("snuk_tests$a")) static SnukTest *__snuk_tests_start = NULL;
__declspec(allocate("snuk_tests$z")) static SnukTest *__snuk_tests_end = NULL;

    #define SNUK_TEST_SECTION __declspec(allocate("snuk_tests$m"))

    #define SNUK_TEST_BEGIN() (&__snuk_tests_start + 1)
    #define SNUK_TEST_END() (&__snuk_tests_end)
#else
    #error "Unsupported compiler :("
#endif

#define ADD_TEST(fn)                                                                \
    static bool fn(void);                                                           \
    static SnukTest snuk_test_struct_##fn = {#fn, fn};                              \
    static SNUK_TEST_SECTION SnukTest *snuk_test_ptr_##fn = &snuk_test_struct_##fn; \
    static bool fn(void)

static inline bool snuk_run_all_tests(void) {
    bool failed = false;

#if defined(__APPLE__) && defined(__MACH__)
    size_t count = 0;
    SnukTest **tests = SNUK_TEST_BEGIN_COUNT(count);

    for (size_t i = 0; i < count; i++) {
        SnukTest *it = tests[i];
#else
    SnukTest **begin = SNUK_TEST_BEGIN();
    SnukTest **end = SNUK_TEST_END();

    for (SnukTest **p = begin; p < end; ++p) {
        SnukTest *it = *p;
#endif
        if (!it) continue;

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
    int main(void) {                          \
        snuk_logger_init();                   \
        snuk_memory_init(mem_size);           \
        bool failed = snuk_run_all_tests();   \
        snuk_memory_deinit();                 \
        snuk_logger_deinit();                 \
        return (int)failed;                   \
    }

#define RUN_ALL_TESTS() RUN_ALL_TESTS_WITH_MEM_SIZE(MIB(1))

#define TEST_PASSED return true
#define TEST_FAILED return false

#define ASSERT(x)                                                                              \
    do {                                                                                       \
        if (!(x)) {                                                                            \
            log_error("Assertion failed: %s (%s:%d) in %s", #x, __FILE__, __LINE__, __func__); \
            TEST_FAILED;                                                                       \
        }                                                                                      \
    } while (0)

#define ASSERT_EQ(a, b)                                                                                  \
    do {                                                                                                 \
        if ((a) != (b)) {                                                                                \
            log_error("Assertion failed: %s == %s (%s:%d) in %s", #a, #b, __FILE__, __LINE__, __func__); \
            TEST_FAILED;                                                                                 \
        }                                                                                                \
    } while (0)

#define ASSERT_NE(a, b)                                                                                  \
    do {                                                                                                 \
        if ((a) == (b)) {                                                                                \
            log_error("Assertion failed: %s != %s (%s:%d) in %s", #a, #b, __FILE__, __LINE__, __func__); \
            TEST_FAILED;                                                                                 \
        }                                                                                                \
    } while (0)

#define ASSERT_NULL(x)                                                                                             \
    do {                                                                                                           \
        if ((x) != NULL) {                                                                                         \
            log_error("Assertion failed: expected %s to be NULL (%s:%d) in %s", #x, __FILE__, __LINE__, __func__); \
            TEST_FAILED;                                                                                           \
        }                                                                                                          \
    } while (0)

#define ASSERT_NOT_NULL(x)                                                                        \
    do {                                                                                          \
        if ((x) == NULL) {                                                                        \
            log_error("Assertion failed: expected %s to be non-NULL (%s:%d) in %s", #x, __FILE__, \
                      __LINE__, __func__);                                                        \
            TEST_FAILED;                                                                          \
        }                                                                                         \
    } while (0)

#define ASSERT_PTR_EQ(a, b)                                                                                       \
    do {                                                                                                          \
        if ((void *)(a) != (void *)(b)) {                                                                         \
            log_error("Assertion failed: pointers %s == %s (%s:%d) in %s", #a, #b, __FILE__, __LINE__, __func__); \
            TEST_FAILED;                                                                                          \
        }                                                                                                         \
    } while (0)

#define ASSERT_PTR_NE(a, b)                                                                                       \
    do {                                                                                                          \
        if ((void *)(a) == (void *)(b)) {                                                                         \
            log_error("Assertion failed: pointers %s != %s (%s:%d) in %s", #a, #b, __FILE__, __LINE__, __func__); \
            TEST_FAILED;                                                                                          \
        }                                                                                                         \
    } while (0)

#define ASSERT_STR_EQ(a, b)                                                                         \
    do {                                                                                            \
        const char *snuk_str_a = (a);                                                               \
        const char *snuk_str_b = (b);                                                               \
                                                                                                    \
        bool snuk_equal                                                                             \
            = (snuk_str_a == NULL && snuk_str_b == NULL)                                            \
              || (snuk_str_a != NULL && snuk_str_b != NULL && strcmp(snuk_str_a, snuk_str_b) == 0); \
                                                                                                    \
        if (!snuk_equal) {                                                                          \
            log_error("Assertion failed: strings %s == %s (%s:%d) in %s\n"                          \
                      "  left : \"%s\"\n"                                                           \
                      "  right: \"%s\"",                                                            \
                      #a, #b, __FILE__, __LINE__, __func__, snuk_str_a ? snuk_str_a : "(null)",     \
                      snuk_str_b ? snuk_str_b : "(null)");                                          \
            TEST_FAILED;                                                                            \
        }                                                                                           \
    } while (0)

#define ASSERT_STR_NE(a, b)                                                                         \
    do {                                                                                            \
        const char *snuk_str_a = (a);                                                               \
        const char *snuk_str_b = (b);                                                               \
                                                                                                    \
        bool snuk_equal                                                                             \
            = (snuk_str_a == NULL && snuk_str_b == NULL)                                            \
              || (snuk_str_a != NULL && snuk_str_b != NULL && strcmp(snuk_str_a, snuk_str_b) == 0); \
                                                                                                    \
        if (snuk_equal) {                                                                           \
            log_error("Assertion failed: strings %s != %s (%s:%d) in %s\n"                          \
                      "  both: \"%s\"",                                                             \
                      #a, #b, __FILE__, __LINE__, __func__, snuk_str_a ? snuk_str_a : "(null)");    \
            TEST_FAILED;                                                                            \
        }                                                                                           \
    } while (0)

#define ASSERT_STR_N_EQ(a, b, n)                                                                 \
    do {                                                                                         \
        const char *snuk_str_a = (a);                                                            \
        const char *snuk_str_b = (b);                                                            \
        size_t snuk_str_n = (n);                                                                 \
                                                                                                 \
        bool snuk_equal = (snuk_str_a == NULL && snuk_str_b == NULL)                             \
                          || (snuk_str_a != NULL && snuk_str_b != NULL                           \
                              && strncmp(snuk_str_a, snuk_str_b, snuk_str_n) == 0);              \
                                                                                                 \
        if (!snuk_equal) {                                                                       \
            log_error("Assertion failed: first %zu chars of %s == %s (%s:%d) in %s", snuk_str_n, \
                      #a, #b, __FILE__, __LINE__, __func__);                                     \
            TEST_FAILED;                                                                         \
        }                                                                                        \
    } while (0)

#define ASSERT_CHAR_EQ(a, b)                                                                                     \
    do {                                                                                                         \
        char snuk_char_a = (a);                                                                                  \
        char snuk_char_b = (b);                                                                                  \
                                                                                                                 \
        if (snuk_char_a != snuk_char_b) {                                                                        \
            log_error("Assertion failed: chars %s == %s (%s:%d) in %s\n"                                         \
                      "  left : '%c' (%d)\n"                                                                     \
                      "  right: '%c' (%d)",                                                                      \
                      #a, #b, __FILE__, __LINE__, __func__, snuk_char_a, snuk_char_a, snuk_char_b, snuk_char_b); \
            TEST_FAILED;                                                                                         \
        }                                                                                                        \
    } while (0)

#define ASSERT_MEM_EQ(a, b, size)                                                   \
    do {                                                                            \
        const void *snuk_mem_a = (a);                                               \
        const void *snuk_mem_b = (b);                                               \
        size_t snuk_mem_size = (size);                                              \
                                                                                    \
        if (memcmp(snuk_mem_a, snuk_mem_b, snuk_mem_size) != 0) {                   \
            log_error("Assertion failed: memory %s == %s with size %zu (%s:%d) in " \
                      "%s",                                                         \
                      #a, #b, snuk_mem_size, __FILE__, __LINE__, __func__);         \
            TEST_FAILED;                                                            \
        }                                                                           \
    } while (0)
