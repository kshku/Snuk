#include "test_framework.h"

#include <snuk_string.h>

ADD_TEST(test_string_length) {
    ASSERT_EQ(snuk_string_length("Hello"), 5);
    ASSERT_EQ(snuk_string_length(""), 0);

    TEST_PASSED;
}

ADD_TEST(test_string_equal) {
    ASSERT(snuk_string_equal("Hi", "Hi"));
    ASSERT(snuk_string_equal("", ""));
    ASSERT(!snuk_string_equal("Hello", "Hi"));

    TEST_PASSED;
}

ADD_TEST(test_string_concat) {
    const char *a = "Hello, ";
    const char *b = "World!";

    char *c = snuk_string_concat(a, 0, b, 0);
    ASSERT(snuk_string_equal(c, "Hello, World!"));
    snuk_free(c);

    c = snuk_string_concat(a, 3, b, 2);
    ASSERT(snuk_string_equal(c, "HelWo"));
    snuk_free(c);

    c = snuk_string_concat(a, 6, b, 6);
    ASSERT(snuk_string_equal(c, "Hello,World!"));
    snuk_free(c);

    c = snuk_string_concat(a, 7, b, 6);
    ASSERT(snuk_string_equal(c, "Hello, World!"));
    snuk_free(c);

    TEST_PASSED;
}

RUN_ALL_TESTS();
