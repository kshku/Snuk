#pragma once

#include <stdint.h>

typedef struct {
    const char *file;  // filename or NULL
    uint32_t line;     // 1-indexed
    uint32_t col;      // 1-indexed
} SnukSrcLoc;

#define SNUK_SRC_LOC_NULL ((SnukSrcLoc){NULL, 0, 0})

typedef enum {
    SNUK_ERROR_KIND_NONE = 0,
    SNUK_ERROR_KIND_PARSE,
    SNUK_ERROR_KIND_INTERP,
} SnukErrorKind;

typedef enum {
    SNUK_INTERP_ERR_NONE = 0,
    SNUK_INTERP_ERR_TYPE,         // type system violations
    SNUK_INTERP_ERR_NAME,         // name resolution / declaration conflicts
    SNUK_INTERP_ERR_FUNCALL,      // function call errors
    SNUK_INTERP_ERR_ASSIGN,       // assignment / initialization failures
    SNUK_INTERP_ERR_CONTROL_FLOW, // break/continue/return outside valid scope
    SNUK_INTERP_ERR_INTERFACE,    // interface creation errors
    SNUK_INTERP_ERR_INTERNAL,     // unexpected/unreachable code paths
} SnukInterpError;

typedef enum {
    SNUK_PARSE_ERR_NONE = 0,
    SNUK_PARSE_ERR_UNEXPECTED_TOKEN,
    SNUK_PARSE_ERR_EXPECTED_IDENTIFIER,
    SNUK_PARSE_ERR_EXPECTED_TYPE_ANNOTATION,
    SNUK_PARSE_ERR_EXPECTED_SEMICOLON_OR_NEWLINE,
    SNUK_PARSE_ERR_EXPECTED_CLOSE_BRACE,
    SNUK_PARSE_ERR_EXPECTED_CLOSE_PAREN,
    SNUK_PARSE_ERR_EXPECTED_CLOSE_BRACKET,
    SNUK_PARSE_ERR_EXPECTED_KEYWORD,
    SNUK_PARSE_ERR_EXPECTED_EXPRESSION,
    SNUK_PARSE_ERR_EXPECTED_TYPE,
    SNUK_PARSE_ERR_EXPECTED_MEMBER,
    SNUK_PARSE_ERR_INVALID_LITERAL,
    SNUK_PARSE_ERR_UNTERMINATED_STRING,
    SNUK_PARSE_ERR_LEXER_ERROR,
    SNUK_PARSE_ERR_UNKNOWN,
} SnukParseError;

typedef struct {
    SnukErrorKind kind;
    uint32_t code;      // cast from SnukInterpError or SnukParseError
    const char *msg;    // human-readable description
    SnukSrcLoc loc;
} SnukError;

#define SNUK_ERROR_NONE ((SnukError){SNUK_ERROR_KIND_NONE, 0, NULL, SNUK_SRC_LOC_NULL})

static inline const char *snuk_interp_error_msg(SnukInterpError code) {
    switch (code) {
        case SNUK_INTERP_ERR_NONE: return "no error";
        case SNUK_INTERP_ERR_TYPE: return "type error";
        case SNUK_INTERP_ERR_NAME: return "name error";
        case SNUK_INTERP_ERR_FUNCALL: return "function call error";
        case SNUK_INTERP_ERR_ASSIGN: return "assignment error";
        case SNUK_INTERP_ERR_CONTROL_FLOW: return "control flow error";
        case SNUK_INTERP_ERR_INTERFACE: return "interface error";
        case SNUK_INTERP_ERR_INTERNAL: return "internal error";
    }
    return "unknown error";
}
