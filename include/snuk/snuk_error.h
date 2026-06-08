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
    SNUK_INTERP_ERR_SHOULD_NOT_REACH_HERE,
    SNUK_INTERP_ERR_SOMETHING_WENT_WRONG,
    SNUK_INTERP_ERR_CONTROL_FLOW,
    SNUK_INTERP_ERR_EXISTS,
    SNUK_INTERP_ERR_NON_TYPE,
    SNUK_INTERP_ERR_EXPECT_ASSIGN,
    SNUK_INTERP_ERR_BUILTIN_INVALID_VALUE,
    SNUK_INTERP_ERR_MEMBER_INITIALIZE,
    SNUK_INTERP_ERR_SELF_CREATION,
    SNUK_INTERP_ERR_PARAM_CREATION,
    SNUK_INTERP_ERR_NON_FN,
    SNUK_INTERP_ERR_PARAM_COUNT,
    SNUK_INTERP_ERR_NO_PARAM,
    SNUK_INTERP_ERR_PARAM_MIXED,
    SNUK_INTERP_ERR_PARAM_REQUIRED,
    SNUK_INTERP_ERR_SELF,
    SNUK_INTERP_ERR_SET_ENV_FAIL,
    SNUK_INTERP_ERR_MEMBER,
    SNUK_INTERP_ERR_INTERFACE,
    SNUK_INTERP_ERR_TYPE_MISMATCH,
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
        case SNUK_INTERP_ERR_SHOULD_NOT_REACH_HERE: return "shouldn't reach here";
        case SNUK_INTERP_ERR_SOMETHING_WENT_WRONG: return "something went wrong";
        case SNUK_INTERP_ERR_CONTROL_FLOW: return "control flow item outside scope";
        case SNUK_INTERP_ERR_EXISTS: return "variable already exists";
        case SNUK_INTERP_ERR_NON_TYPE: return "expected a type";
        case SNUK_INTERP_ERR_EXPECT_ASSIGN: return "expected assignment expression";
        case SNUK_INTERP_ERR_BUILTIN_INVALID_VALUE: return "invalid value for builtin type member";
        case SNUK_INTERP_ERR_MEMBER_INITIALIZE: return "failed to initialize member";
        case SNUK_INTERP_ERR_SELF_CREATION: return "failed to create self";
        case SNUK_INTERP_ERR_PARAM_CREATION: return "failed to create parameter";
        case SNUK_INTERP_ERR_NON_FN: return "call expression on non-function";
        case SNUK_INTERP_ERR_PARAM_COUNT: return "parameter count mismatch";
        case SNUK_INTERP_ERR_NO_PARAM: return "parameter doesn't exist";
        case SNUK_INTERP_ERR_PARAM_MIXED: return "mixed positional and named parameters";
        case SNUK_INTERP_ERR_PARAM_REQUIRED: return "required parameter missing";
        case SNUK_INTERP_ERR_SELF: return "failed to get self";
        case SNUK_INTERP_ERR_SET_ENV_FAIL: return "failed to set env value";
        case SNUK_INTERP_ERR_MEMBER: return "couldn't find the member";
        case SNUK_INTERP_ERR_INTERFACE: return "failed to create interface";
        case SNUK_INTERP_ERR_TYPE_MISMATCH: return "types are not the same";
    }
    return "unknown error";
}
