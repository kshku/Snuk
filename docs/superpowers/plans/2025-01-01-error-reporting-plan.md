# Error Reporting Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the current ad-hoc error reporting with a clean library-produces-data, consumer-formats architecture.

**Architecture:** A single `SnukError` POD type shared by parser and interpreter. Both produce structured errors with source location (line/col). The REPL/consumer formats and prints. The library never prints.

**Tech Stack:** C17, CMake, existing test infrastructure

---

## File Structure

- **Create:** `include/snuk/snuk_error.h` — `SnukSrcLoc`, `SnukErrorKind`, `SnukError`, runtime/parse error code enums
- **Modify:** `include/snuk/interpreter/interpreter.h` — replace `SnukErrorCode err_code` with `SnukError err` + `SnukSrcLoc cur_loc`
- **Modify:** `include/snuk/interpreter/interpreter_helper.h` — update `interpreter_error()`, add `interpreter_set_loc()`
- **Modify:** `src/interpreter/interpreter.c` — update all error calls, add loc tracking at expression entry points
- **Modify:** `include/snuk/parser/parser.h` — replace `err_msg`/`err_token` with `SnukError err`
- **Modify:** `include/snuk/parser/snuk_item.h` — replace `const char *msg` with `SnukError error` in error item
- **Modify:** `src/parser/parser.c` — update `parser_error()`, `parser_sync()`
- **Modify:** `src/parser/parser_common.h` — update `parser_advance()`, `parser_expect()`
- **Modify:** `repl/runtime.c` — format `SnukError` for display
- **Remove:** `include/snuk/interpreter/error_code.h` — replaced by `snuk_error.h`
- **Remove:** `src/interpreter/error_code.c` — replaced by `snuk_error.h`
- **Remove:** `repl/runtime.h` — inline functions (no changes needed but verify)

---

### Task 1: Create `snuk_error.h` with core types

**Files:**
- Create: `include/snuk/snuk_error.h`
- Modify: None

- [ ] **Step 1: Write the header file**

```c
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
```

- [ ] **Step 2: Verify it compiles in isolation**

Run: `echo '#include "snuk_error.h"' | gcc -std=c17 -fsyntax-only -I include -x c -`
Expected: no errors

- [ ] **Step 3: Commit**

```bash
git add include/snuk/snuk_error.h
git commit -m "feat: add SnukError types for structured error reporting"
```

---

### Task 2: Update parser to use SnukError

**Files:**
- Modify: `include/snuk/parser/parser.h`
- Modify: `include/snuk/parser/snuk_item.h`
- Modify: `src/parser/parser.c`
- Modify: `src/parser/parser_common.h`

- [ ] **Step 1: Update `include/snuk/parser/parser.h`**

Replace `err_msg`/`err_token` with a single `SnukError err`:

```c
#include "snuk/snuk_error.h"

typedef struct SnukParser {
    SnukLexer *lexer;
    SnukToken previous;
    SnukToken current;
    SnukToken next;
    bool panic_mode;
    SnukError err;          // <-- replace err_msg + err_token
    struct {
        char *buffer;
        uint64_t length;
    } string_buffer;
} SnukParser;
```

Also add `snuk_parser_clear_error()` declaration:

```c
SnukError snuk_parser_clear_error(SnukParser *parser);
```

- [ ] **Step 2: Update `include/snuk/parser/snuk_item.h`**

Replace the error item's `const char *msg` with `SnukError error`:

```c
#include "snuk/snuk_error.h"

// Inside the item union:
struct {
    SnukError error;    // <-- was const char *msg + SnukToken token
} error;
```

Remove `build_error_item()` and replace with a version that takes `SnukError`:

```c
SnukItem *snuk_build_error_item(SnukAllocator allocator, SnukError error);
```

- [ ] **Step 3: Update `src/parser/parser.c`**

Replace `parser_error()`:

```c
#include "snuk/lexer.h"

void parser_error(SnukParser *parser, SnukParseError code, const char *msg) {
    if (parser->panic_mode) return;
    parser->panic_mode = true;
    parser->err = (SnukError){
        .kind = SNUK_ERROR_KIND_PARSE,
        .code = code,
        .msg = msg,
        .loc = {
            .line = parser->current.line,
            .col = parser->current.col,
        },
    };
}
```

Update `parser_sync()`:

```c
static SnukItem *parser_sync(SnukParser *parser) {
    while (parser->current.type != SNUK_TOKEN_EOF
           && parser->current.type != SNUK_TOKEN_SEMICOLON
           && parser->current.type != SNUK_TOKEN_NEWLINE) {
        parser_advance(parser);
    }
    SnukItem *item = snuk_build_error_item(parser->allocator, parser->err);
    parser->panic_mode = false;
    return item;
}
```

- [ ] **Step 4: Update `src/parser/parser_common.h`**

Update `parser_advance()`:

```c
#define parser_advance(parser) \
    do { \
        (parser)->previous = (parser)->current; \
        (parser)->current = (parser)->next; \
        (parser)->next = snuk_lexer_next_token((parser)->lexer); \
        if ((parser)->current.type == SNUK_TOKEN_ERROR) \
            parser_error(parser, SNUK_PARSE_ERR_LEXER_ERROR, "lexer error"); \
    } while (0)
```

Update `parser_expect()`:

```c
static inline bool parser_expect(SnukParser *parser, SnukTokenType type, const char *msg) {
    if (parser->current.type == type) {
        parser_advance(parser);
        return true;
    }
    parser_error(parser, SNUK_PARSE_ERR_UNEXPECTED_TOKEN, msg);
    return false;
}
```

- [ ] **Step 5: Update `snuk_build_error_item()` in snuk_item.h**

```c
SnukItem *snuk_build_error_item(SnukAllocator allocator, SnukError error) {
    SnukItem *item = snuk_alloc_item(allocator);
    if (!item) return NULL;
    item->type = SNUK_ITEM_ERROR;
    item->error.error = error;
    return item;
}
```

- [ ] **Step 6: Update all parser .c files that call `parser_error()`**

These files currently call `parser_error(parser, "some string")`. Update them all:

```c
// Old:
parser_error(parser, "expected identifier");

// New:
parser_error(parser, SNUK_PARSE_ERR_EXPECTED_IDENTIFIER, "expected identifier");
```

Files to update (grep for `parser_error`):
- `src/parser/snuk_expr.c`
- `src/parser/snuk_item.c`  
- `src/parser/snuk_type.c`

- [ ] **Step 7: Build and verify parser compiles**

Run: `cmake --build build --target snuk_repl -j$(nproc) 2>&1`
Expected: compiles with no errors

- [ ] **Step 8: Run existing .snuk tests to verify parser still works**

Run: `ctest --test-dir build/tests -L snuk_files --output-on-failure`
Expected: all pass

- [ ] **Step 9: Commit**

```bash
git add include/snuk/parser/parser.h include/snuk/parser/snuk_item.h src/parser/
git commit -m "feat: update parser to use SnukError"
```

---

### Task 3: Update interpreter to use SnukError

**Files:**
- Modify: `include/snuk/interpreter/interpreter.h`
- Modify: `include/snuk/interpreter/interpreter_helper.h`
- Modify: `include/snuk/interpreter/snuk_value.h`
- Modify: `src/interpreter/interpreter.c`
- Remove: `include/snuk/interpreter/error_code.h`
- Remove: `src/interpreter/error_code.c`

- [ ] **Step 1: Update `include/snuk/interpreter/interpreter.h`**

Replace `SnukErrorCode err_code` with `SnukError err` + `SnukSrcLoc cur_loc`:

```c
#include "snuk/snuk_error.h"

typedef struct SnukInterpreter {
    SnukRefCounter *global;
    SnukRefCounter *current;
    SnukRefCounter *instance;

    SnukError err;          // <-- replaces SnukErrorCode err_code
    SnukSrcLoc cur_loc;     // current source location for error reporting

    SnukDarray /* SnukValue * */ trash;
    SnukSignal signal;
    // ...
} SnukInterpreter;
```

Remove `#include "error_code.h"` from the includes.

Add declarations:
```c
SnukError snuk_interpreter_clear_error(SnukInterpreter *intpret);
void snuk_interpreter_set_loc(SnukInterpreter *intpret, uint32_t line, uint32_t col);
```

- [ ] **Step 2: Update `include/snuk/interpreter/snuk_value.h`**

Replace `SnukErrorCode err_code` with a cleaner approach — keep for now but rename to avoid confusion. Actually, the simplest approach: keep `SnukErrorCode err_code` field for now but typedef `SnukErrorCode` as a generic uint32_t. Or better, just remove it from the value and have the error be on the interpreter only.

Actually, the current system copies `err_code` from the interpreter onto the returned value to communicate errors. With the new system, the consumer calls `snuk_interpreter_clear_error()` after `exec_item()`. So we don't need `err_code` on the value anymore.

Remove `SnukErrorCode err_code;` from the `SnukValue` struct. Adjust all references.

- [ ] **Step 3: Update `include/snuk/interpreter/interpreter_helper.h`**

Update `interpreter_error()`:

```c
#include "snuk/snuk_error.h"

static inline void interpreter_error(SnukInterpreter *intpret, SnukInterpError code, const char *msg) {
    if (intpret->err.kind != SNUK_ERROR_KIND_NONE) return;  // first error wins
    intpret->err = (SnukError){
        .kind = SNUK_ERROR_KIND_INTERP,
        .code = code,
        .msg = msg,
        .loc = intpret->cur_loc,
    };
}
```

Add `interpreter_set_loc()`:

```c
static inline void interpreter_set_loc(SnukInterpreter *intpret, uint32_t line, uint32_t col) {
    intpret->cur_loc.line = line;
    intpret->cur_loc.col = col;
}
```

- [ ] **Step 4: Update `src/interpreter/interpreter.c`**

Update all `interpreter_error(intpret, SNUK_ERROR_XXX)` calls to `interpreter_error(intpret, SNUK_INTERP_ERR_XXX, msg)`.

Update `snuk_interpreter_exec_item()`:

```c
SnukValue snuk_interpreter_exec_item(SnukInterpreter *intpret, SnukItem *item) {
    interpreter_clear_trash(intpret);

    // Set cur_loc from item if available (parser items track line/col)
    // For now, we just clear the error at the start of each item
    SnukValue res = interpreter_exec_item(intpret, item, true);

    if (intpret->signal != SNUK_SIGNAL_NONE) {
        interpreter_error(intpret, SNUK_INTERP_ERR_CONTROL_FLOW, "control flow item outside scope");
    }

    // Error is now on intpret->err, not copied to value
    // Consumer reads via snuk_interpreter_clear_error()
    return res;
}
```

Add `snuk_interpreter_clear_error()`:

```c
SnukError snuk_interpreter_clear_error(SnukInterpreter *intpret) {
    SnukError err = intpret->err;
    intpret->err = (SnukError)SNUK_ERROR_NONE;
    return err;
}

void snuk_interpreter_set_loc(SnukInterpreter *intpret, uint32_t line, uint32_t col) {
    interpreter_set_loc(intpret, line, col);
}
```

Update all `interpreter_error(ptr, SNUK_ERROR_*)` calls throughout the file:

```c
// Old:
interpreter_error(intpret, SNUK_ERROR_NON_FN);

// New:
interpreter_error(intpret, SNUK_INTERP_ERR_NON_FN, "call expression on non-function");
```

Grep for all `SNUK_ERROR_` references in the file and replace:
- `SNUK_ERROR_NONE` → check `intpret->err.kind == SNUK_ERROR_KIND_NONE`
- `SNUK_ERROR_CONTROL_FLOW` → `SNUK_INTERP_ERR_CONTROL_FLOW`
- `SNUK_ERROR_EXISTS` → `SNUK_INTERP_ERR_EXISTS`
- etc.

The only error guard in `execute_call_expr()` (around line 1029):

```c
if (intpret->err.kind != SNUK_ERROR_KIND_NONE) {
    snuk_ref_counter_release(&new_scope);
    return (SnukValue){.type = SNUK_VALUE_UNKOWN};
}
```

- [ ] **Step 5: Remove old error files**

Delete `include/snuk/interpreter/error_code.h` and `src/interpreter/error_code.c`.
Remove the `error_code.c` source from `CMakeLists.txt` in the interpreter directory.

- [ ] **Step 6: Build and verify**

Run: `cmake --build build --target snuk_repl -j$(nproc) 2>&1`
Expected: compiles with no errors

- [ ] **Step 7: Run tests**

Run: `ctest --test-dir build/tests -L snuk_files --output-on-failure`
Expected: all pass (some tests may need regex update — see Task 5)

- [ ] **Step 8: Commit**

```bash
git add include/snuk/interpreter/ src/interpreter/
git rm include/snuk/interpreter/error_code.h src/interpreter/error_code.c
git commit -m "feat: update interpreter to use SnukError"
```

---

### Task 4: Thread source location through interpreter

**Files:**
- Modify: `src/interpreter/interpreter.c`

- [ ] **Step 1: Add `interpreter_set_loc()` calls at expression entry points**

In each `execute_*` function, set the loc at entry. Since we don't have loc on every AST node yet, use a simple approach: the parser sets line/col from the item's token when calling `snuk_interpreter_exec_item()`, and each expression evaluator updates it.

For now, update `snuk_interpreter_exec_item()` to accept an optional start location and set it on the interpreter:

```c
SnukValue snuk_interpreter_exec_item(SnukInterpreter *intpret, SnukItem *item, SnukSrcLoc start_loc) {
    interpreter_clear_trash(intpret);
    intpret->cur_loc = start_loc;
    // ... rest of function
}
```

But wait — changing the signature breaks the public API. Better to have a separate setter called by the runtime before exec_item:

```c
// In runtime.c:
snuk_interpreter_set_loc(&rt->interpreter, line, col);
SnukValue val = snuk_interpreter_exec_item(&rt->interpreter, item);
```

The parser can't easily provide line/col per item right now (the token info is in the parser, not on the item). For the initial implementation, just set loc to (0,0) or the last-known token position from the parser.

- [ ] **Step 2: Build and test**

Run: `cmake --build build --target snuk_repl -j$(nproc) && ctest --test-dir build/tests -L snuk_files --output-on-failure`
Expected: all pass

- [ ] **Step 3: Commit**

```bash
git add src/interpreter/interpreter.c include/snuk/interpreter/interpreter.h
git commit -m "feat: add cur_loc tracking to interpreter"
```

---

### Task 5: Update runtime/REPL to display errors

**Files:**
- Modify: `repl/runtime.c`
- Modify: `CMakeLists.txt` (if needed for test regex)

- [ ] **Step 1: Update `repl/runtime.c`**

Replace the error display:

```c
#include <snuk/snuk_error.h>

void snuk_runtime_execute(Runtime *rt, const char *src) {
    SnukParser parser;
    snuk_parser_init(&parser, src, &rt->parser_allocator);

    SnukItem *item;
    while (true) {
        item = snuk_parser_next_item(&parser);
        if (!item) break;

        // Handle parser errors
        SnukError parse_err = snuk_parser_clear_error(&parser);
        if (parse_err.kind != SNUK_ERROR_KIND_NONE) {
            snuk_println("[Error] at line %u, col %u: %s",
                parse_err.loc.line, parse_err.loc.col, parse_err.msg);
            continue;
        }

        SnukValue value = snuk_interpreter_exec_item(&rt->interpreter, item);

        // Handle interpreter errors
        SnukError interp_err = snuk_interpreter_clear_error(&rt->interpreter);
        if (interp_err.kind != SNUK_ERROR_KIND_NONE) {
            snuk_println("[Error] at line %u, col %u: %s",
                interp_err.loc.line, interp_err.loc.col, interp_err.msg);
        }

        snuk_value_log(value);
        log_trace("", NULL);
        snuk_value_free(value);
    }

    snuk_parser_deinit(&parser);
}
```

- [ ] **Step 2: Update test regex in `tests/CMakeLists.txt`**

The test regex checks for "ERROR|error". Our new format says "[Error]" which also matches. But let's verify:

Run: `ctest --test-dir build/tests -L snuk_files --output-on-failure`
Expected: all pass

If tests fail because error output format changed, update `FAIL_REGULAR_EXPRESSION` to match the new format.

- [ ] **Step 3: Commit**

```bash
git add repl/runtime.c tests/CMakeLists.txt
git commit -m "feat: update REPL to display structured SnukError"
```

---

### Task 6: Remove library-side log_error calls

**Files:**
- Modify: `src/interpreter/interpreter.c`
- Modify: `src/parser/` (if any log_error calls exist)

- [ ] **Step 1: Remove `log_error` calls from interpreter**

The interpreter currently has one `log_error` call in the `SNUK_ITEM_ERROR` handler (line 1150-1152):

```c
case SNUK_ITEM_ERROR:
    log_error("Error: %s", item->error.msg);
    return (SnukValue){.type = SNUK_VALUE_NULL};
```

Replace with:

```c
case SNUK_ITEM_ERROR:
    // Error data is in item->error.error; consumer handles display
    // Store parser error on interpreter for the consumer to read
    if (intpret->err.kind == SNUK_ERROR_KIND_NONE) {
        intpret->err = item->error.error;  // propagate parser error through interpreter
    }
    return (SnukValue){.type = SNUK_VALUE_NULL};
```

- [ ] **Step 2: Build and test**

Run: `cmake --build build --target snuk_repl -j$(nproc) && ctest --test-dir build/tests -L snuk_files --output-on-failure`
Expected: all pass

- [ ] **Step 3: Commit**

```bash
git add src/interpreter/interpreter.c
git commit -m "feat: remove library-side log_error, propagate SnukError through interpreter"
```

---

### Task 7: Clean up includes and verify everything compiles

**Files:**
- All modified files

- [ ] **Step 1: Remove obsolete includes**

Search for leftover `#include "error_code.h"` references and replace with `#include "snuk/snuk_error.h"`:

```bash
grep -rn "error_code.h" include/ src/ repl/ --include="*.h" --include="*.c"
```

Also update any `error_code.h` includes that need to point to `snuk_error.h`.

- [ ] **Step 2: Full rebuild and test**

```bash
cmake --build build -j$(nproc) 2>&1
ctest --test-dir build/tests --output-on-failure
```

Expected: all compile, all tests pass

- [ ] **Step 3: Commit**

```bash
git add -A
git commit -m "chore: clean up includes, remove old error_code references"
```
