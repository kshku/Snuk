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
executed item. `return` and `break` are for early exits only.

### Variables

```snuk
var x = 10
var name = "snuk"
const MAX = 100

// type annotations — optional, enforced at runtime when present
var speed: float = 2.5
```

### Blocks as expressions

Every block returns the value of its last executed item.
`break` exits a block or loop early with an optional value.

```snuk
var result = {
    var temp = x * 2
    temp + 10          // last item — block's value
}

// break exits early
var early = {
    if x > 5 { break x * 2 }
    x + 1
}
```

### Control flow

```snuk
// if/else — value is last item of taken branch
var label = if x > 10 { "big" } else { "small" }

// while
while x > 0 { x = x - 1 }

// for — C-style
for var i = 0; i < 10; i = i + 1 { print i }

// for without condition — infinite loop
// break exits with optional value
var result = for {
    x = x + 1
    if x >= 100 { break x }
}

// do/while
do { x = x - 1 } while x > 0
```

### Functions

Functions are values. Both forms are identical in semantics.
Last item is the return value. `return` is for early exits only.

```snuk
// statement form
fn add(a, b) { a + b }

// expression form
var add = fn(a, b) { a + b }

// early return
fn withdraw(balance, amount) {
    if amount > balance { return false }
    balance - amount
}

// default parameters
fn greet(name, greeting = "hello") {
    print greeting + ", " + name
}

// pass functions as arguments
fn apply(value, func) { func(value) }
var doubled = apply(5, fn(x) { x * 2 })

// return functions from functions
fn make_adder(n) { fn(x) { x + n } }
var add5 = make_adder(5)
print add5(3)    // 8
```

### Types

Types are structs with methods. No inheritance. Duck typing.

```snuk
// statement form
type Point {
    var x = 0.0
    var y = 0.0

    fn to_string() {
        "(" + self.x + ", " + self.y + ")"
    }
}

// expression form — types are values
var Point = type { var x = 0.0; var y = 0.0 }

// type factory
fn make_type(default_color) {
    type {
        var color = default_color
        fn to_string() { self.color }
    }
}

var p = Point { x: 3.0, y: 4.0 }
print p.to_string()
p.x = 10.0
```

### Lists

```snuk
var items = [1, 2, 3, 4, 5]
items[0] = 99
items.add(6)
items.remove(0)
print items.length

for item in items { print item }
```

### Duck typing

```snuk
// any type with area() works — no declaration needed
fn print_area(shape) { print shape.area() }

type Rectangle {
    var width = 0.0
    var height = 0.0
    fn area() { self.width * self.height }
}

type Circle {
    var radius = 0.0
    fn area() { 3.14159 * self.radius * self.radius }
}

print_area(Rectangle { width: 10.0, height: 5.0 })
print_area(Circle { radius: 7.0 })
```

### Built-in types

| Type | Example |
|------|---------|
| `int` | `42`, `-7` |
| `float` | `3.14`, `-0.5` |
| `bool` | `true`, `false` |
| `string` | `"hello"`, `'world'` |
| `null` | `null` |
| `fn` | `fn(a, b) { }` |
| `type` | `type { }` |
| `list` | `[1, 2, 3]` |

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
