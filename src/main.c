#include "logger.h"
#include "memory.h"

int main(void) {
    snuk_logger_init();
    if (!snuk_memory_init()) return -1;

    log_info("Hello, World!");

    snuk_memory_deinit();
    snuk_logger_deinit();

    return 0;
}
