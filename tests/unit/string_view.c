#include "test_framework.h"

#include <memory.h>
#include <snuk_string.h>
#include <string_view.h>

ADD_TEST(test_string_view_create_with_len) {
    SnukStringView view = snuk_string_view_create_with_len("hello world", 5);

    ASSERT_EQ(view.len, 5);
    ASSERT(view.str != NULL);
    ASSERT(snuk_string_n_equal(view.str, "hello", 5));

    TEST_PASSED;
}

ADD_TEST(test_string_view_create) {
    SnukStringView view = snuk_string_view_create("hello");

    ASSERT_EQ(view.len, 5);
    ASSERT(view.str != NULL);
    ASSERT(snuk_string_equal(view.str, "hello"));

    TEST_PASSED;
}

ADD_TEST(test_string_view_get_cstr) {
    SnukStringView view = snuk_string_view_create("hello world");
    char* str = snuk_string_view_get_cstr(view);

    ASSERT(snuk_string_equal(str, "hello world"));

    snuk_free(str);

    TEST_PASSED;
}

ADD_TEST(test_string_view_copy) {
    SnukStringView view = snuk_string_view_create("hello world");
    SnukStringView copy = snuk_string_view_copy(view);

    ASSERT_EQ(view.len, copy.len);
    ASSERT(copy.str != view.str);
    ASSERT(snuk_string_equal(view.str, copy.str));

    snuk_free((void *)copy.str);

    TEST_PASSED;
}

ADD_TEST(test_string_view_concat) {
    SnukStringView a = snuk_string_view_create("hello");
    SnukStringView b = snuk_string_view_create(" world");
    SnukStringView result = snuk_string_view_concat(a, b);

    ASSERT_EQ(result.len, 11);
    ASSERT(snuk_string_n_equal(result.str, "hello world", result.len));

    snuk_free((void *)result.str);
  
    TEST_PASSED;
}

ADD_TEST(test_string_view_equal) {
    SnukStringView a = snuk_string_view_create("hello");
    SnukStringView b = snuk_string_view_create("hello");
    SnukStringView c = snuk_string_view_create("world");
    SnukStringView d = snuk_string_view_create("hell");
    SnukStringView e = snuk_string_view_create("");
    SnukStringView f = snuk_string_view_create("");

    ASSERT(snuk_string_view_equal(a, b));
    ASSERT(!snuk_string_view_equal(a, c));
    ASSERT(!snuk_string_view_equal(a, d));
    ASSERT(snuk_string_view_equal(e, f));

    TEST_PASSED;
}

RUN_ALL_TESTS();