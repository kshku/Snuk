# Future Plans

Ideas and features that are intentionally out of scope for the current version.
Nothing here is guaranteed — this is a thinking space, not a roadmap.

---

## Language

- Map type ?
- Type casting ?
- Arbitrary precision integers (bignum) ?
- Pattern matching (`match` / `case`)
- Error handling (result type or try/catch) ?
- Modules and imports ?
- Closures and anonymous functions
- Generator functions ?
- String interpolation (`"hello {name}"`) ?
- Static typing mode (opt-in strict types)

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


# Language additions ?

## Interfaces

Interface similar to go.
Named method contracts, satisfied implicitly through duck typing.
No `implements` declaration needed — if a type has the methods, it satisfies the interface.

### Syntax

```snuk
interface Drawable {
    fn draw()
    fn update(dt)
}

// parameter annotated with interface — runtime checked
fn render(obj: Drawable) {
    obj.draw()
}
```

- Interfaces provide explicit API contracts for libraries.
- Allows extending the existing types.
- Requires external method definitions (fn TypeName.method)
- Can add `extend` keyword as syntax sugar
```snuk
extend string {
    fn shout() {
        print string.uppercase() + '!!!'
    }
}
// equivalent to
fn string.shout() {
    print string.uppercase() + '!!!'
}
```
- Language syntax grows
