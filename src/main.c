#include "logger.h"
#include "memory.h"
#include "io.h"
#include "string.h"

#define PROMPT_STR ">>> "

int main(void) {
    snuk_logger_init();
    if (!snuk_memory_init()) return -1;

    log_info("Hello, World!");

    while (true) {
        snuk_print(PROMPT_STR);
        char *line = snuk_read_line();

        if (snuk_string_n_equal("exit", line, 4)) {
            snuk_println("Bye!");
            break;
        }

        snuk_println(line);
    }

    // Reset the linear allocator.
    // Should we use frame allocator instead?
    snuk_free(SNUK_ALLOC_KIND_LINEAR, NULL);

    snuk_memory_deinit();
    snuk_logger_deinit();

    return 0;
}
