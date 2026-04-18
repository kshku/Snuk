# Snuk

[![CI](https://github.com/kshku/snuk/actions/workflows/ci.yml/badge.svg?branch=dev)](https://github.com/kshku/snuk/actions/workflows/ci.yml)
[![License](https://img.shields.io/badge/license-Apache%202.0-blue.svg)](LICENSE)
[![Version](https://img.shields.io/github/v/tag/kshku/snuk?label=version&sort=semver)](https://github.com/kshku/snuk/releases)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey)]()

Snuk is a lightweight, interpreted programming language written in C.

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
./build/snuk -c <command>
```

---

## Language Overview

Snuk is dynamically typed with optional type annotations. Semicolons are optional.

### Variables

```snuk
var x = 10
var name = "snuk"
var ratio: float = 3.14
const MAX = 100
```

### Control flow

```snuk
if x > 10 {
    print "big"
} else if x > 5 {
    print "medium"
} else {
    print "small"
}

while x > 0 {
    x = x - 1
}

for i = 0; i < 10; i = i + 1 {
    print i
}

loop {
    x = x + 1
    if x >= 100 { break }
}
```

### Functions

```snuk
fn add(a, b) {
    return a + b
}

fn greet(name: string, greeting = "hello") {
    print greeting + ", " + name
}

var result = add(3, 4)
greet("world")
greet("world", "hi")
```

### Types

```snuk
type Point {
    var x: float = 0.0
    var y: float = 0.0

    fn to_string() -> string {
        return "(" + self.x + ", " + self.y + ")"
    }
}

var p = Point { x: 3.0, y: 4.0 }
print p.to_string()
p.x = 10.0
```

### Built-in types

| Type | Example |
|------|---------|
| `int` | `42`, `-7` |
| `float` | `3.14`, `-0.5` |
| `bool` | `true`, `false` |
| `string` | `"hello"` |
| `null` | `null` |

### Operators

| Category | Operators |
|----------|-----------|
| Arithmetic | `+` `-` `*` `/` `%` |
| Comparison | `==` `!=` `<` `>` `<=` `>=` |
| Logical | `and` `or` `not` |
| Bitwise | `&` `\|` `^` `~` `<<` `>>` |
| Assignment | `=` `+=` `-=` `*=` `/=` `%=` |

---

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for branching strategy, commit conventions, and pull request guidelines.

Planned features and ideas that are out of scope for the current version are tracked in [FUTURE.md](FUTURE.md).

---

## License

Apache 2.0 — see [LICENSE](LICENSE) for details.
