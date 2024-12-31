#include "sstring.h"

#include <core/sstring.h>

u8 noLength(void) {
    if (!sStringEqual("Hello", "Hello", 0)) return FAIL;
    if (sStringEqual("Hello", "Hi", 0)) return FAIL;
    if (sStringEqual("There", "The", 0)) return FAIL;
    return PASS;
}

u8 withLength(void) {
    if (!sStringEqual("There", "The", 3)) return FAIL;
    if (!sStringEqual("Hello", "Hi", 1)) return FAIL;
    if (sStringEqual("Hello", "Hi", 2)) return FAIL;
    return PASS;
}

Test *core_sstring_register_tests(Test *tests) {
    tests = testManagerRegister(tests, noLength, "Without length (len = 0)");
    tests =
        testManagerRegister(tests, withLength, "with length (len = length)");
    return tests;
}
