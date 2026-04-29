#include "logger.h"
#include "memory.h"
#include "io.h"
#include "runtime.h"
#include "snuk_string.h"

#define PROMPT_STR ">>> "
#define LINE_BUFFER_SIZE 1024

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
static void print_version(void);

static char *program_name;

int main(int argc, char *argv[]) {
    snuk_logger_init();
#ifdef SNUK_DEBUG
    log_trace("DEBUG MODE", NULL);
#endif
    if (!snuk_memory_init(GIB(1))) return -1;

    char *data;
    OpMode mode = parse_args(argc, argv, &data);
    log_info("Hello, World!, Program name: %s", program_name);

    switch (mode) {
        case OP_MODE_FILE:
            log_info("Running the file: %s", data);
            run_file(data);
            break;

        case OP_MODE_REPL:
            log_info("Running in REPL mode", NULL);
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
    return snuk_string_equal(opt, short_opt) || snuk_string_equal(opt, long_opt);
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
        } else if (is_option(argv[i], "-v", "--version")) {
            print_version();
            return OP_MODE_QUIT;
        } else {
            *data = argv[i];
            return OP_MODE_FILE;
        }
    }

    return OP_MODE_QUIT;
}

void run_repl(void) {
    char *line_buffer = (char *)snuk_alloc(LINE_BUFFER_SIZE, alignof(char));
    Runtime rt = snuk_runtime_init();

    const char *line;
    do {
        snuk_print(PROMPT_STR);
        line = snuk_read_line(line_buffer, LINE_BUFFER_SIZE);
    } while (!snuk_runtime_execute(&rt, line));

    snuk_runtime_deinit(&rt);

    snuk_println("Bye!");
}

void run_file(const char *path) {
    const char *content = snuk_read_file(path);

    Runtime rt = snuk_runtime_init();

    snuk_runtime_execute(&rt, content);

    snuk_runtime_deinit(&rt);

    snuk_free((void *)content);
}

static void run_command(const char *command) {
    Runtime rt = snuk_runtime_init();
    snuk_runtime_execute(&rt, command);
    snuk_runtime_deinit(&rt);
}

static void print_help(void) {
    snuk_println(
            "snuk - snuk interpreter\n"
            "VERSION: %d.%d.%d\n"
            "USAGE:\n"
            "snuk               luanch as REPL\n"
            "snuk file.snuk     runs the file\n"
            "if multiple files are given, they will be ignored. Only first file gets executed."
            "\n"
            "ARGS:\n"
            "-v | --version                 print the version\n"
            "-h | --help                    print this help message and exit\n"
            "-c | --command \"COMMAND\"     executes the given command and exits\n",
            SNUK_VERSION_MAJOR, SNUK_VERSION_MINOR, SNUK_VERSION_PATCH
    );
}

static void print_version(void) {
    snuk_println("Snuk version: %d.%d.%d", SNUK_VERSION_MAJOR, SNUK_VERSION_MINOR, SNUK_VERSION_PATCH);
}
