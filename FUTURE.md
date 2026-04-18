# Future Plans

Ideas and features that are intentionally out of scope for the current version.
Nothing here is guaranteed — this is a thinking space, not a roadmap.

---

## Language

- Arbitrary precision integers (bignum) ?
- Arrays and maps as built-in types
- Pattern matching (`match` / `case`)
- Error handling (result type or try/catch)
- Modules and imports
- Inheritance or interfaces for types
- Closures and anonymous functions
- Generator functions
- String interpolation (`"hello {name}"`)
- Bitwise operator sugar (`<<`, `>>`, `~`, `^`, `&`, `|`)
- Static typing mode (opt-in strict types)

## Standard Library

- Math functions (`sin`, `cos`, `sqrt`, `abs`, ...)
- String functions (`split`, `trim`, `replace`, ...)
- Collections (`list`, `map`, `set`)
- File I/O from Snuk code ?
- Random number generation

## Memory

- Growable arenas for larger programs (or argument specifying the size) ?

## Tooling

- `snuk fmt` — code formatter ?
- `snuk check` — static analysis without running ?
- `snuk doc` — documentation generator from comments ?
- Language server protocol (LSP) support ?
- Syntax highlighting definitions (VSCode, Neovim, etc.) ?

## Platform

- Windows, Linux, macOS build verification
- Pre-built binaries for major platforms
- Package manager integration (brew, scoop, apt)
