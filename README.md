# Snuk

[![CI](https://github.com/kshku/snuk/actions/workflows/ci.yml/badge.svg?branch=dev)](https://github.com/kshku/snuk/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![Version](https://img.shields.io/github/v/tag/kshku/snuk?label=version&sort=semver)](https://github.com/kshku/snuk/releases)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey)]()

Snuk is an interpreted scripting language written in C.

---

## Table of Contents

- [Building](#building)
- [Usage](#usage)
- [Language Overview](#language-overview)
- [Contributing](#contributing)

---

## Building

### Prerequisites

- C compiler — GCC or Clang (Linux, macOS), MSVC (Windows)
- CMake 3.20+
- Git

### Steps

```bash
git clone --recurse-submodules https://github.com/kshku/snuk.git
cd snuk
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The binary is produced at `build/snuk` (Linux, macOS) or `build/Release/snuk.exe` (Windows).

---

## Usage

### REPL

```bash
./build/snuk
```

### Run a file

```bash
./build/snuk myfile.snuk
```

### Run a command

```bash
./build/snuk -c "print 1 + 2"
```

---

## Language Overview

Snuk is dynamically typed. Almost everything is an expression —
variables, functions, types, control flow. Semicolons are optional.

The value of a block, function, if/else, or loop is the last
executed item. `return` and `break` can be used for early exits.

**Separator rule:** `()` uses `,` — `{}` uses `;` or newlines.

### Variables and typing

Snuk uses gradual typing. Without a type annotation a variable has
type `any` — it accepts any value at any time. With an annotation
the type is enforced at runtime. No implicit conversions between types.

```snuk
// untyped — type is "any", accepts anything
var a = 10
a = "hello"       // valid — a is any

// typed — enforced at runtime
var b: int = 10
// b = "hello"    // runtime error

// "any" explicit — identical to no annotation
var c: any = 10
c = true          // valid

// type annotations
var speed: float = 2.5
var name: str = "snuk"
const MAX = 100

// "type" keyword optional in annotations — both identical
var x1: int = 10
var x2: type int = 10

// user-defined type annotation
var p: Point        // shorthand
var p2: type Point  // identical
```

### Blocks as expressions

```snuk
var result = {
    var temp = x * 2
    temp + 10          // last item — block's value
}

// break exits early with a value
var x = 10
var early = {
    if x > 5 { break x * 2 }
    x + 1
}
```

### Control flow

```snuk
// if/else — value is last item of taken branch
var label = if x > 10 { "big" } else { "small" }

var wc = 0
while wc < 10 { wc = wc + 1 }

for var i = 0; i < 10; i = i + 1 { print i }

// infinite loop — break exits with optional value
var fv = 0
var result = for {
    fv = fv + 1
    if fv >= 100 { break fv }
}

do { wc = wc - 1 } while wc > 0
```

### Functions

Functions are values. Last item is the return value.
`return` can be used for early exits.

```snuk
// statement form
fn add(a, b) { a + b }

// expression form
var add = fn(a, b) { a + b }

// early return
fn withdraw(balance, amount) {
    if amount > balance { return false }
    balance - amount
    // can also use return here
    // return balance - amount
}

// typed parameters — enforced at runtime
fn multiply(a: int, b: int) -> int { a * b }

// function type annotation — () uses comma
var transform: fn(float) -> float

// default parameters, higher order functions, closures
fn make_adder(n) { fn(x) { x + n } }
var add5 = make_adder(5)
print add5(3)    // 8
```

### Types

Types are structs with methods. No inheritance. Duck typing.
`{}` uses `;` or newlines as separators.

Inside methods, name lookup follows this order:
1. local scope — parameters and variables declared in the method
2. instance scope — fields of the instance
3. closure scope — variables captured at type definition
4. outer scopes

Fields are accessible directly by name. `self` is always available
as an explicit reference to the instance — useful when a local
variable shadows a field name.

```snuk
// definition
type Point {
    var x: float = 0.0
    var y: float = 0.0

    fn to_string() {
        "(" + x + ", " + y + ")"    // fields accessed directly
    }

    fn set_x(x) {
        // parameter x shadows field x
        self.x = x    // self disambiguates
    }
}

// instantiation — four forms
// only form 1 and type-first forms create typed variables
// form 2 (var = type Name {}) creates an untyped (any) variable
var s1: type Square = type Square { width: 10; height: 10 }  // typed
var s2 = type Square { width: 10; height: 10 }               // untyped (any)
type Square s3 = { width: 10; height: 10 }                   // typed, type-first
type Square s4 { width: 10; height: 10 }                     // typed, compact

// newlines as separators
var p = type Point {
    x: 3.0
    y: 4.0
}

// nested types
type SquareWithPos {
    var top_left: type Point = type Point { x: 0.0; y: 0.0 }
    var width: int = 0
    var height: int = 0
    fn area() { width * height }    // direct field access
}

var sq = type SquareWithPos {
    top_left: type Point { x: 10.0; y: 20.0 }
    width: 100
    height: 50
}

// type factory — closure scope lower priority than instance scope
fn make_type(color) {
    type {
        var color = color       // instance field
        fn to_string() { color }    // resolves to instance field
    }
}

p.x = 10.0
print p.to_string()
print sq.top_left.x    // 10.0
```

### Duck typing

```snuk
// any type with area() works — no declaration needed
fn print_area(shape) { print shape.area() }

type Rectangle {
    var width = 0.0
    var height = 0.0
    fn area() { width * height }    // direct field access
}

type Circle {
    var radius = 0.0
    fn area() { 3.14159 * radius * radius }
}

print_area(type Rectangle { width: 10.0; height: 5.0 })
print_area(type Circle { radius: 7.0 })
```

### Built-in types

| Type | Example |
|------|---------|
| `int` | `42`, `-7` |
| `float` | `3.14`, `-0.5` |
| `bool` | `true`, `false` |
| `str` | `"hello"`, `'world'` |
| `null` | `null` |
| `fn` | `fn(a, b) { }` |
| `type` | `type { }` |
| `list` | `[1, 2, 3]` *(not yet implemented)* |

### Built-in methods

All primitive types have conversion methods callable directly on literals.
Falsy values: `0`, `0.0`, `false`, `""`, `null` — everything else is truthy.

```snuk
100.to_float()          // 100.0
100.to_bool()           // true
100.to_str()            // "100"
"500".to_int()          // 500
"abc".to_int()          // null — parse failure

"hello".length()              // 5
"hello".get()                 // "hello"
"hello".get(1)                // "ello"
"hello".get(0, 2)             // "he"   (start=0, len=2)
"hello".get(start=1, len=3)   // "ell"
"hello".get(len=3)            // "hel"
// returns null on out of bounds
```

### Operators

| Category | Operators |
|----------|-----------|
| Arithmetic | `+` `-` `*` `/` `%` |
| Comparison | `==` `!=` `<` `>` `<=` `>=` |
| Logical | `and` `or` `not` / `&&` `\|\|` `!` |
| Bitwise | `&` `\|` `^` `~` `<<` `>>` |
| Assignment | `=` `+=` `-=` `*=` `/=` `%=` `&=` `\|=` `^=` `<<=` `>>=` |

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for branching strategy, commit
conventions, and pull request guidelines.

Planned features and ideas that are out of scope for the current
version are tracked in [FUTURE.md](FUTURE.md).

---

## License

Apache 2.0 — see [LICENSE](LICENSE) for details.
