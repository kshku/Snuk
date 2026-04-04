#include "logger.h"
#include "memory.h"

int main(void) {
    snuk_logger_init();
    if (!snuk_memory_init()) return -1;

    log_info("Hello, World!");

    // Reset the linear allocator.
    // Should we use frame allocator instead?
    snuk_free(SNUK_ALLOC_KIND_LINEAR, NULL);

    snuk_memory_deinit();
    snuk_logger_deinit();

    return 0;
}
