# Error Reporting Design

## Goals

- Library produces structured error data; the consumer (REPL/embedder) handles formatting and display
- Both parser and interpreter produce errors through the same type
- Errors carry source location (line, col) for user-friendly messages
- No formatting, printing, or I/O in the library — pure data

## Data Types

```c
typedef struct {
    const char *file;     // filename or NULL (unknown source)
    uint32_t line;        // 1-indexed
    uint32_t col;         // 1-indexed
} SnukSrcLoc;

typedef enum {
    SNUK_ERROR_KIND_PARSE,
    SNUK_ERROR_KIND_RUNTIME,
} SnukErrorKind;

typedef struct {
    SnukErrorKind kind;
    uint32_t code;        // error code (e.g., TYPE_MISMATCH, PARAM_COUNT)
    const char *msg;      // human-readable description, no newlines
    SnukSrcLoc loc;
} SnukError;
```

`SnukError` is POD — no allocation, returned by value, copied freely.

## Error Codes

Replace the current `SnukErrorCode` enum with separate enums for parse vs runtime errors:

```c
// Parse errors
typedef enum {
    SNUK_PARSE_ERR_NONE = 0,
    SNUK_PARSE_ERR_UNEXPECTED_TOKEN,
    SNUK_PARSE_ERR_EXPECTED_IDENTIFIER,
    SNUK_PARSE_ERR_EXPECTED_SEMICOLON,
    SNUK_PARSE_ERR_EXPECTED_CLOSE_BRACE,
    // ...
} SnukParseErrorCode;

// Runtime errors
typedef enum {
    SNUK_RUNTIME_ERR_NONE = 0,
    SNUK_RUNTIME_ERR_TYPE_MISMATCH,
    SNUK_RUNTIME_ERR_NON_FUNCTION,
    SNUK_RUNTIME_ERR_PARAM_COUNT,
    SNUK_RUNTIME_ERR_UNDEFINED_VARIABLE,
    SNUK_RUNTIME_ERR_DUPLICATE_VARIABLE,
    // ...
} SnukRuntimeErrorCode;
```

## Parser Flow

- `parser_error(parser, kind, msg)` → builds `SnukError` from the current token's line/col, stores it in the parser
- Parser continues producing `SNUK_ITEM_ERROR` items, but the error item carries a full `SnukError` instead of a raw string
- Consumer reads `snuk_parser_clear_error(parser)` after each `next_item()` call

## Interpreter Flow

- The interpreter struct gets a `SnukError err` field (one-shot latch, same pattern as current `err_code`)
- `interpreter_error(intpret, kind, msg, loc)` → sets `intpret->err` if no error is already latched
- Location tracking:
  - The interpreter has a single `SnukSrcLoc cur_loc` field, updated before each expression is evaluated
  - Each `execute_*` function calls `interpreter_set_loc(intpret, line, col)` before evaluating
  - The parser already computes line/col for each token; pass this through to the interpreter
  - NO changes to AST nodes needed — loc is tracked in the interpreter, not on expressions
  - The loc is captured at the point of error via `interpreter_error()`
- After `exec_item()`, consumer reads `snuk_interpreter_clear_error(intpret)` to get the error
- Consumer also calls `snuk_value_get_error(val)` for per-value errors

## Consumer (REPL) Flow

```c
SnukValue val = snuk_interpreter_exec_item(&rt->interpreter, item);
SnukError err = snuk_interpreter_clear_error(&rt->interpreter);

if (err.code != SNUK_ERROR_NONE) {
    // REPL formats however it wants:
    //   "Error[E12] at line 5, col 12: type mismatch"
    snuk_eprintln("Error[E%d] at line %u, col %u: %s",
        err.code, err.loc.line, err.loc.col, err.msg);
}
```

No formatting logic in the library — just data.

## Implementation Plan

### Phase 1: Core Types and Parser

1. Create `include/snuk/snuk_error.h` — define `SnukSrcLoc`, `SnukErrorKind`, `SnukError`, parse/runtime error code enums
2. Update `SnukParser` struct — store `SnukError` instead of `err_msg`/`err_token`
3. Update `parser_error()` and `parser_sync()` — work with `SnukError`
4. Update `SNUK_ITEM_ERROR` to carry `SnukError` instead of raw string
5. Update `snuk_parser_next_item()` — clear error each iteration
6. Remove old `error_code.h` and related files (or keep for compatibility)

### Phase 2: Interpreter Integration

7. Add `SnukError` + `SnukSrcLoc cur_loc` to `SnukInterpreter` struct, replace `err_code` field
8. Update `interpreter_error(intpret, kind, code, msg)` — captures current `cur_loc` into the error
9. Add `interpreter_set_loc(intpret, line, col)` — set `cur_loc` before each expression evaluation
10. Pass loc info through `snuk_interpreter_exec_item(intpret, item)` — execution starts with `cur_loc` set from the item's token position (token positions from parser are passed in)
11. Each `execute_*` function updates `cur_loc` at its entry point for error precision
12. Update `execute_call_expr()` error guard to use new error type
13. Update `snuk_interpreter_exec_item()` to transfer errors to returned values

### Phase 3: Consumer

13. Update `snuk_runtime_execute()` — read structured errors, format for display
14. Remove all `log_error()` calls from interpreter (library produces data, not output)
15. Verify REPL and file mode work identically

## Key Decisions

- **Library vs consumer boundary**: The library NEVER prints errors. It returns structured data. The consumer decides format and output.
- **Location tracking**: Single `SnukSrcLoc` on the interpreter (updated before each expression) rather than adding loc to every AST node. Simpler to implement, adequate precision.
- **Error codes**: Separate enums for parse vs runtime, both with NONE = 0 for zero-init safety.
- **Error latching**: Same "first error wins" pattern as current implementation.
- **Backward compatibility**: The old `SnukErrorCode` enum and `error_code.h` are removed. The new `SnukError` type replaces both the old `err_code` field in `SnukInterpreter` and the old `err_msg`/`err_token` fields in `SnukParser`.
