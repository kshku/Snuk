#include "logger.h"

int main(void) {
    snuk_logger_init();

    log_info("Hello, World!");

    snuk_logger_deinit();
}
