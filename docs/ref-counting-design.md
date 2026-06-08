# Reference Counting Design

## Overview

Snuk uses pure reference counting (no tracing GC). Each managed object is wrapped in a `SnukRefCounter` with strong/weak counts and a destructor callback (`free_fn`).

Objects are freed when `strong_count + weak_count == 0`. The `free_fn` runs when `strong_count` reaches 1 (before the final decrement).

## weak_ref Rules

The `weak_ref` flag controls whether a scope's parent link is weak or strong:

- **weak_ref=true**: Scope does NOT keep its parent alive. Safe when:
  - Data is accessed directly (not through parent chain)
  - Parent is kept alive by other references during the scope's lifetime
- **weak_ref=false**: Scope keeps its parent alive (strong link). Required when:
  - Closures capture variables from enclosing scope
  
### By Scope Type

| Creation Site | `weak_ref` | Rationale |
|---|---|---|
| **Type init scope** (`execute_type_declaration`) | Conditional via `weak_ref` param | Top-level types use `type_scope` for method dispatch (safe with weak). Nested types need strong parent for variable capture. |
| **Instance scope** (`execute_inst_creation`) | **Always true** | Fields accessed through closure/type_scope, never through parent chain. `self` already uses explicit weak ref. |
| **Call scope → param scope** (`execute_call_expr`) | **Always true** | Param scope retained by fn value's closure during execution. |
| **Fn param scope (method)** (`execute_fn_expr`, inside type) | Conditional via `weak_ref` param | `self` comes from instance value. Nested fns inside methods need strong parent for capture. |
| **Fn param scope (nested fn)** (`execute_fn_expr`, inside block) | false (strong) | Captures enclosing variables. Creates intentional refcycle — no escape possible without GC. |

## Known Limitations

### Data Cycles (Unfixable with Pure Refcounting)

Instance field assignments can create data-level cycles:

```snuk
hall.north = garden
garden.south = hall
```

Each instance value holds a strong retain on the other's scope RC. Pure refcounting cannot resolve this.

**Behavior at shutdown**: `snuk_memory_deinit()` frees the arena, releasing all pages to the OS. The leaked scopes' `free_fn` never runs, but no external resources are leaked (scopes contain only heap-allocated values within the same arena).

**If deterministic cleanup with destructors is needed**, a tracing GC or cycle detector must be added.

### Nested Closures (Intentional Refcycle)

Functions defined inside other functions that capture variables create a refcycle:

```snuk
fn outer() {
    var x = 10
    fn inner() { return x }
    return inner
}
```

The outer call scope's env holds the inner fn value (retaining inner's param scope), and inner's param scope has a strong parent link to the outer call scope. This creates a cycle that pure refcounting cannot resolve.

This is acceptable because:
- The cycle naturally breaks when the fn value is dropped (the outer call scope has no other references)
- Without a GC, Snuk accepts arena-cleanup for these cases
