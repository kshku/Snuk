# Future Plans

Ideas and features that are intentionally out of scope for the current version.
Nothing here is guaranteed — this is a thinking space, not a roadmap.

---

## Language

- Map type ?
- Arbitrary precision integers (bignum) ?
- Pattern matching (`match` / `case`)
- Modules and imports ?
- Generator functions ?
- String interpolation (`"hello {name}"`) ?

## Standard Library

- Math functions (`sin`, `cos`, `sqrt`, `abs`, ...)
- String functions (`split`, `trim`, `replace`, ...)
- File I/O from Snuk code ?
- Random number generation

## Tooling

- `snuk fmt` — code formatter ?
- `snuk check` — static analysis without running ?
- `snuk doc` — documentation generator from comments ?
- Language server protocol (LSP) support ?
- Syntax highlighting definitions ?

## Platform

- Windows, Linux, macOS build verification
- Pre-built binaries for major platforms
- Package manager integration


# Language additions

## Operator overloading ?

Allow operator overloading.

```snuk
type Point {
    var x: int
    var y: int
    fn +(other_obj) {
        Point {x: self.x + other_obj.x, y: self.y + other_obj.y}
    }
}
```

- Requires multiple dispatch

## Function type should include parameter name ?
```snuk
fn name(a: int, b: float)
// or
var name: fn(a: int, b: float)
```

This enables use functions from interfaces by passing parameters using parameter names like
```snuk
interface Drawable {
    fn draw(x: int, y: float, renderer: type Renderer)
}

fn draw_things(obj: Drawable) {
    obj.draw(renderer=renderer, x=10, y=20.0)
}
```
Without this (current):
```snuk
interface Drawable {
    var darw: fn(int, float, type Renderer) // have to declare functions like this
}

fn draw_things(obj: Drawable) {
    obj.draw(10, 20.0, renderer) // cannot pass params by name
}
```

This also makes us unable to bind function expression to a variable if parameters doesn't match.
Another approach is to have function declarations like
```snuk
fn a_fn(a: int, b: float)
```
which creates a function will null body.
It can be then defined as
```snuk
fn a_fn(a: int, b: float) {
    // ...
}
```
But this is inconsistant with
```snuk
var a_fn: fn(int, float)
```
way of defining.
