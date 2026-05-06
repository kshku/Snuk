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

ADD_TEST(test_string_equal_ignore_case) {
    // basic cases
    ASSERT(snuk_string_equal_ignore_case("hello", "hello"));
    ASSERT(snuk_string_equal_ignore_case("Hello", "hello"));
    ASSERT(snuk_string_equal_ignore_case("HELLO", "hello"));
    ASSERT(snuk_string_equal_ignore_case("HeLlO", "hElLo"));

    // empty strings
    ASSERT(snuk_string_equal_ignore_case("", ""));

    // non-alphabetic characters must match exactly
    ASSERT(snuk_string_equal_ignore_case("hello123", "HELLO123"));
    ASSERT(!snuk_string_equal_ignore_case("hello123", "HELLO124"));
    ASSERT(snuk_string_equal_ignore_case("!@#", "!@#"));
    ASSERT(!snuk_string_equal_ignore_case("!@#", "!@$"));

    TEST_PASSED;
}

ADD_TEST(test_string_n_equal) {
    // basic equality within n
    ASSERT(snuk_string_n_equal("hello", "hello", 5));
    ASSERT(snuk_string_n_equal("hello", "heLLo", 2));
    ASSERT(!snuk_string_n_equal("hello", "heLLo", 5));

    // within n
    ASSERT(snuk_string_n_equal("hello", "heXYZ", 2));
    ASSERT(!snuk_string_n_equal("hello", "hello", 10));

    // empty strings
    ASSERT(snuk_string_n_equal("", "", 0));
    ASSERT(!snuk_string_n_equal("", "", 1));

    TEST_PASSED;
}

ADD_TEST(test_char_in_string) {
    // basic cases
    ASSERT(snuk_char_in_string('a', "abc"));
    ASSERT(snuk_char_in_string('c', "abc"));
    ASSERT(!snuk_char_in_string('x', "abc"));

    // repeated characters
    ASSERT(snuk_char_in_string('a', "aaaa"));

    // empty string
    ASSERT(!snuk_char_in_string('a', ""));

    // special characters
    ASSERT(snuk_char_in_string('!', "!@#"));
    ASSERT(!snuk_char_in_string('$', "!@#"));

    // null string
    ASSERT(!snuk_char_in_string('a', NULL));

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
