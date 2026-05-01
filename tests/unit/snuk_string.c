#include "test_framework.h"

#include <snuk_string.h>

ADD_TEST(test_string_concat) {
    const char *a = "Hello, ";
    const char *b = "World!";

    char *c = snuk_string_concat(a, 0, b, 0);
    snuk_free(c);
    log_trace("%s", c);

    c = snuk_string_concat(a, 3, b, 2);
    log_trace("%s", c);
    snuk_free(c);

    c = snuk_string_concat(a, 6, b, 6);
    log_trace("%s", c);
    snuk_free(c);

    c = snuk_string_concat(a, 7, b, 6);
    log_trace("%s", c);
    snuk_free(c);

    return true;
}

RUN_ALL_TESTS();
