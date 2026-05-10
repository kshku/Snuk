#include "test_framework.h"

#include <memory.h>
#include <snuk_string.h>
#include <string_view.h>

ADD_TEST(test_string_view_create_with_len) {
    SnukStringView view = snuk_string_view_create_with_len("hello world", 5);

    ASSERT_EQ(view.len, 5);
    ASSERT_NOT_NULL(view.str);
    ASSERT_STR_N_EQ(view.str, "hello", 5);

    TEST_PASSED;
}

ADD_TEST(test_string_view_create) {
    SnukStringView view = snuk_string_view_create("hello");

    ASSERT_EQ(view.len, 5);
    ASSERT_NOT_NULL(view.str);
    ASSERT_STR_EQ(view.str, "hello");

    TEST_PASSED;
}

ADD_TEST(test_string_view_get_cstr) {
    SnukStringView view = snuk_string_view_create("hello world");
    char *str = snuk_string_view_get_cstr(view);

    ASSERT_NOT_NULL(str);
    ASSERT_STR_EQ(str, "hello world");

    snuk_free(str);

    TEST_PASSED;
}

ADD_TEST(test_string_view_copy) {
    SnukStringView view = snuk_string_view_create("hello world");
    SnukStringView copy = snuk_string_view_copy(view);

    ASSERT_EQ(view.len, copy.len);
    ASSERT_PTR_NE(copy.str, view.str);
    ASSERT_STR_EQ(view.str, copy.str);

    snuk_free((void *)copy.str);

    TEST_PASSED;
}

ADD_TEST(test_string_view_concat) {
    SnukStringView a = snuk_string_view_create("hello");
    SnukStringView b = snuk_string_view_create(" world");
    SnukStringView result = snuk_string_view_concat(a, b);

    ASSERT_EQ(result.len, 11);
    ASSERT_NOT_NULL(result.str);
    ASSERT_STR_N_EQ(result.str, "hello world", result.len);

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
