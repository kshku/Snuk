#include "test_framework.h"

#include <string_view.h>

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