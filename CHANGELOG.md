# Changelog

All notable changes to Snuk will be documented here.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).
Snuk uses [Semantic Versioning](https://semver.org/).

---

## [Unreleased]

### Language

- Variables and constants (`var`, `const`)
- Gradual typing — optional type annotations enforced at runtime
- Built-in types: `int`, `float`, `bool`, `str`, `fn`, `type`
- All built-in types are Snuk types — can be extended with `extend`
- `null` — special value, not a type, cannot be extended
- `any` type — no enforcement, default when unannotated
- `type` as annotation — accepts any type value, rejects instances
- Arithmetic, bitwise, logical, comparison, and compound assignment operators
- String concatenation with `+`
- Almost everything is an expression — blocks, if/else, loops, fn, type
- Block expressions — value is last executed item
- `if/else` expressions
- `while` and `do/while` loops
- C-style `for` loop with optional condition (infinite loop via `for {}`)
- `break` with optional value — exits nearest enclosing block or loop
- `continue` — skips to next loop iteration, caught by nearest loop
- `return` with optional value — exits nearest enclosing function
- Signal propagation — `break` → block/loop, `continue` → loop, `return` → function
- Functions as first class values — `fn` is an expression
- `fn name()` as syntax sugar for `var name = fn()`
- Default parameter values, named parameter passing
- Optional typed parameters and return type annotations
- Closures — functions capture enclosing scope
- Types as first class values — `type` is an expression
- `type Name {}` as syntax sugar for `var Name = type {}`
- Four type instantiation forms
- Type annotation using type name directly — `var x: Point`
- Annotating with a type name accepts the type itself OR its instances
- Nested type instantiation
- Methods with direct field access via instance scope resolution
- `self` available in methods for explicit instance reference
- Scope resolution order inside methods: local → instance → closure → outer
- Duck typing — unannotated parameters accept any type
- Type factories — types capture enclosing scope as closures
- Functions stored as type fields (event handler pattern)
- Interfaces — named member contracts, satisfied implicitly
  - Named interfaces: `interface Name { var member: fn() -> type }`
  - Inline interface annotation: `var x: interface { var area: fn() -> float }`
  - Runtime checked when used as annotation
- `extend` — add members to existing types without modifying definition
  - Works on built-in types (`int`, `float`, `bool`, `str`) and user types
  - Takes same declarations as `type {}` body
- Comment trivia — leading and trailing comments attached to tokens
- Optional semicolons — newlines work as separators inside `{}`

### Built-in methods

All primitive types (`int`, `float`, `bool`, `str`) and `null` have:

- `.to_int()` — convert to int, returns `null` on failure
- `.to_float()` — convert to float, returns `null` on failure
- `.to_bool()` — convert to bool (falsy: `0`, `0.0`, `false`, `""`, `null`)
- `.to_str()` — convert to string representation

`str` additionally has:

- `.length()` — number of characters
- `.get(start=0, len=null)` — substring extraction, returns `null` on out of bounds

### Interpreter

- Tree-walk interpreter
- Lexically scoped environment with scope chain
- Control flow signals for `return`, `break`, `continue`
- Runtime type enforcement for annotated variables and parameters
- Interface satisfaction checked at runtime
- Instance scope inserted between local and closure scope on method calls
- Reference counting for memory management

### Infrastructure

- Lexer with full token support including comment trivia
- Pratt parser generating AST
- Memory abstraction layer over SnMemory (linear, stack, freelist allocators)
- Logger abstraction layer over SnLogger
- I/O abstraction layer over SnFile
- Runtime struct encapsulating parser, interpreter, and frame allocator
- CTest integration with unit and integration test labels
- Minimal test framework (`test_framework.h`) for C unit tests
- CI on Linux, macOS, Windows via GitHub Actions
- Conventional commits, branch protection, squash merge workflow

---

<!-- ## [0.1.0] - YYYY-MM-DD -->
