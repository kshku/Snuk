#include "logger.h"
#include "memory.h"
#include "io.h"
#include "string.h"

#define PROMPT_STR ">>> "

typedef enum OpMode {
    OP_MODE_FILE,
    OP_MODE_REPL,
    OP_MODE_COMMAND,
    OP_MODE_QUIT
} OpMode;

static OpMode parse_args(int argc, char *argv[], char **data);

static void run_repl(void);
static void run_file(const char *path);
static void run_command(const char *command);

static void print_help(void);

static char *program_name;

int main(int argc, char *argv[]) {
    snuk_logger_init();
    if (!snuk_memory_init()) return -1;

    char *data;
    OpMode mode = parse_args(argc, argv, &data);
    log_info("Hello, World!, Program name: %s", program_name);
    switch (mode) {
        case OP_MODE_FILE:
            log_info("Running the file: %s", data);
            run_file(data);
            break;

        case OP_MODE_REPL:
            log_info("Running in REPL mode");
            run_repl();
            break;

        case OP_MODE_COMMAND:
            log_info("Running the command: %s", data);
            run_command(data);
            break;

        case OP_MODE_QUIT:
        default:
            break;
    }


    snuk_memory_deinit();
    snuk_logger_deinit();

    return 0;
}

static inline bool is_option(const char *opt, const char *short_opt, const char *long_opt) {
    return string_equal(opt, short_opt) || string_equal(opt, long_opt);
}

OpMode parse_args(int argc, char *argv[], char **data) {
    for (int i = 0; i < argc; ++i) log_info("args[%d] = %s", i, argv[i]);

    program_name = argv[0];

    if (argc == 1) return OP_MODE_REPL;

    for (int i = 1; i < argc; ++i) {
        if (is_option(argv[i], "-h", "--help")) {
            print_help();
            return OP_MODE_QUIT;
        } else if (is_option(argv[i], "-c", "--command")) {
            i++;
            if (i >= argc) snuk_eprintln("COMMAND is not given");
            *data = argv[i];
            return OP_MODE_COMMAND;
        } else {
            *data = argv[i];
            return OP_MODE_FILE;
        }
    }

    return OP_MODE_QUIT;
}

void run_repl(void) {
    while (true) {
        snuk_print(PROMPT_STR);
        char *line = snuk_read_line();

        if (string_equal("exit\n", line)) {
            snuk_println("Bye!");
            break;
        }

        snuk_println(line);

        // Reset the linear allocator.
        // Should we use frame allocator instead?
        snuk_free(SNUK_ALLOC_KIND_LINEAR, NULL);
    }

}

void run_file(const char *path) {
    const char *content = snuk_read_file(path);
    snuk_println("The file content:");
    snuk_println(content);
    snuk_free(SNUK_ALLOC_KIND_LINEAR, NULL);
}

static void run_command(const char *command) {
    snuk_println("The command: %s", command);
    snuk_free(SNUK_ALLOC_KIND_LINEAR, NULL);
}

static void print_help(void) {
    snuk_println(
            "snuk - snuk interpreter\n"
            "USAGE:\n"
            "snuk               luanch as REPL\n"
            "snuk file.snuk     runs the file\n"
            "if multiple files are given, they will be ignored. Only first file gets executed."
            "\n"
            "ARGS:\n"
            "-h | --help                print this help message and exit\n"
            "-c | --command \"COMMAND\"     executes the given command and exits\n"
    );
}
