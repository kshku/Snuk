#include "test_framework.h"

#include <snuk/snuk_string.h>

ADD_TEST(test_is_alpha) {
    // lowercase and uppercase letters
    ASSERT(snuk_is_alpha('a'));
    ASSERT(snuk_is_alpha('z'));
    ASSERT(snuk_is_alpha('A'));
    ASSERT(snuk_is_alpha('Z'));

    // non-alphabetic characters
    ASSERT(!snuk_is_alpha('0'));
    ASSERT(!snuk_is_alpha('9'));
    ASSERT(!snuk_is_alpha('_'));
    ASSERT(!snuk_is_alpha('@'));

    TEST_PASSED;
}

ADD_TEST(test_is_alpha_numeric) {
    // alphabetic characters
    ASSERT(snuk_is_alpha_numeric('a'));
    ASSERT(snuk_is_alpha_numeric('Z'));

    // numeric characters
    ASSERT(snuk_is_alpha_numeric('0'));
    ASSERT(snuk_is_alpha_numeric('9'));

    // non-alphanumeric characters
    ASSERT(!snuk_is_alpha_numeric('_'));
    ASSERT(!snuk_is_alpha_numeric('@'));

    TEST_PASSED;
}

ADD_TEST(test_is_digit) {
    // valid digits
    ASSERT(snuk_is_digit('0'));
    ASSERT(snuk_is_digit('9'));

    // non-digit characters
    ASSERT(!snuk_is_digit('a'));
    ASSERT(!snuk_is_digit('Z'));
    ASSERT(!snuk_is_digit('/'));
    ASSERT(!snuk_is_digit(':'));

    TEST_PASSED;
}

ADD_TEST(test_is_binary_digit) {
    // valid binary digits
    ASSERT(snuk_is_binary_digit('0'));
    ASSERT(snuk_is_binary_digit('1'));

    // invalid binary digits
    ASSERT(!snuk_is_binary_digit('2'));
    ASSERT(!snuk_is_binary_digit('a'));
    ASSERT(!snuk_is_binary_digit(' '));

    TEST_PASSED;
}

ADD_TEST(test_is_octal_digit) {
    // valid octal digits
    ASSERT(snuk_is_octal_digit('0'));
    ASSERT(snuk_is_octal_digit('7'));

    // invalid octal digits
    ASSERT(!snuk_is_octal_digit('8'));
    ASSERT(!snuk_is_octal_digit('9'));
    ASSERT(!snuk_is_octal_digit('a'));

    TEST_PASSED;
}

ADD_TEST(test_is_hex_digit) {
    // numeric digits
    ASSERT(snuk_is_hex_digit('0'));
    ASSERT(snuk_is_hex_digit('9'));

    // lowercase hex letters
    ASSERT(snuk_is_hex_digit('a'));
    ASSERT(snuk_is_hex_digit('f'));

    // uppercase hex letters
    ASSERT(snuk_is_hex_digit('A'));
    ASSERT(snuk_is_hex_digit('F'));

    // invalid hex characters
    ASSERT(!snuk_is_hex_digit('g'));
    ASSERT(!snuk_is_hex_digit('G'));
    ASSERT(!snuk_is_hex_digit('/'));

    TEST_PASSED;
}

ADD_TEST(test_string_length) {
    // basic cases
    ASSERT_EQ(snuk_string_length("Hello"), 5);
    ASSERT_EQ(snuk_string_length(""), 0);

    // string with spaces and symbols
    ASSERT_EQ(snuk_string_length("a b c"), 5);
    ASSERT_EQ(snuk_string_length("!@#"), 3);

    // NULL input
    ASSERT_EQ(snuk_string_length(NULL), 0);

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

ADD_TEST(test_string_n_equal_ignore_case) {
    // basic cases
    ASSERT(snuk_string_n_equal_ignore_case("hello", "hello", 5));
    ASSERT(snuk_string_n_equal_ignore_case("Hello", "hello", 5));
    ASSERT(snuk_string_n_equal_ignore_case("HELLO", "hello", 5));
    ASSERT(snuk_string_n_equal_ignore_case("HeLlO", "hElLo", 5));

    // within n
    ASSERT(snuk_string_n_equal_ignore_case("helloX", "helloY", 5));
    ASSERT(!snuk_string_n_equal_ignore_case("helloX", "heLLoY", 6));

    // empty strings
    ASSERT(snuk_string_n_equal_ignore_case("", "", 0));
    ASSERT(!snuk_string_n_equal_ignore_case("", "", 1));

    // non-alphabetic characters must match exactly
    ASSERT(snuk_string_n_equal_ignore_case("hello123", "HELLO123", 8));
    ASSERT(!snuk_string_n_equal_ignore_case("hello123", "HELLO124", 8));
    ASSERT(snuk_string_n_equal_ignore_case("!@#", "!@#", 3));
    ASSERT(!snuk_string_n_equal_ignore_case("!@#", "!@$", 3));

    // n larger than string length
    ASSERT(!snuk_string_n_equal_ignore_case("hi", "hi", 5));

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
    ASSERT_NOT_NULL(c);
    ASSERT_STR_EQ(c, "Hello, World!");
    snuk_free(c);

    c = snuk_string_concat(a, 3, b, 2);
    ASSERT_NOT_NULL(c);
    ASSERT_STR_EQ(c, "HelWo");
    snuk_free(c);

    c = snuk_string_concat(a, 6, b, 6);
    ASSERT_NOT_NULL(c);
    ASSERT_STR_EQ(c, "Hello,World!");
    snuk_free(c);

    c = snuk_string_concat(a, 7, b, 6);
    ASSERT_NOT_NULL(c);
    ASSERT_STR_EQ(c, "Hello, World!");
    snuk_free(c);

    TEST_PASSED;
}

RUN_ALL_TESTS();
