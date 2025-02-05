#include "log.h"

#include <core/logger.h>

u8 log_all(void) {
    sFatal("This is a fatal");
    sError("This is a error");
    sWarn("This is a warn");
    sInfo("This is a info");
    sDebug("This is a debug");
    sTrace("This is a trace");
    return PASS;
}

Test *log_register_tests(Test *tests) {
    testManagerRegister(tests, log_all, "log_all");
    return tests;
}
