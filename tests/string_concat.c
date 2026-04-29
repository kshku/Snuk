#include <logger.h>
#include <snuk_string.h>
#include <memory.h>

int main(void) {
    snuk_logger_init();
    snuk_memory_init(KIB(1));

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

    snuk_memory_deinit();
    snuk_logger_deinit();
}
