# Contributing to Snuk

Thank you for your interest in contributing! This document covers everything you need to know to contribute effectively.

---

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Branching Strategy](#branching-strategy)
- [Commit Conventions](#commit-conventions)
- [Pull Requests](#pull-requests)
- [Versioning](#versioning)
- [Reporting Bugs](#reporting-bugs)
- [Suggesting Features](#suggesting-features)

---

## Code of Conduct

Be respectful. Be constructive. We are all here to build something good.

---

## Getting Started

### Prerequisites

- C compiler (GCC or Clang)
- CMake 3.20+
- Git

### Building

```bash
git clone --recurse-submodules https://github.com/yourusername/Snuk.git
cd snuk
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Running

```bash
# Start the REPL
./build/snuk

# Run a file
./build/snuk myfile.snuk

# Run a command
./build/snuk -c <command>
```

---

## Branching Strategy

Snuk uses a simplified trunk-based branching model.

### Permanent branches

| Branch | Purpose |
|--------|---------|
| `main` | Stable, always releasable. Protected. |
| `dev`  | Integration branch. All feature branches merge here first. |

### Short-lived branches

All work happens on short-lived branches cut from `dev` and merged back into `dev` via pull request.

Branch names follow this pattern:

```
<type>/<short-description>
```

Where `<type>` is one of:

| Type | When to use |
|------|-------------|
| `feat` | Adding a new feature |
| `fix` | Fixing a bug |
| `docs` | Documentation only |
| `refactor` | Code changes that neither fix a bug nor add a feature |
| `test` | Adding or updating tests |
| `chore` | Build system, tooling, dependencies |
| `perf` | Performance improvements |

### Examples

```
feat/lexer-string-escape
fix/multiline-comment-termination
docs/contributing-guide
refactor/memory-allocator-naming
chore/add-snfile-submodule
```

### Release flow

```
dev  -->  main  -->  tag vX.Y.Z
```

When `dev` is stable and ready for release, it is merged into `main` and a version tag is created. No direct commits to `main`.

---

## Commit Conventions

Snuk follows [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/).

### Format

```
<type>(<scope>): <short summary>

[optional body]

[optional footer]
```

### Type

| Type | When to use |
|------|-------------|
| `feat` | A new feature |
| `fix` | A bug fix |
| `docs` | Documentation changes |
| `refactor` | Code restructuring, no behavior change |
| `test` | Adding or fixing tests |
| `chore` | Build, tooling, dependency changes |
| `perf` | Performance improvement |
| `style` | Formatting, whitespace, no logic change |
| `ci` | CI/CD configuration changes |

### Scope

The scope is optional and refers to the part of the codebase being changed.

Common scopes: `lexer`, `parser`, `interpreter`, `memory`, `logger`, `io`, `ast`, `repl`, `build`

### Summary rules

- Use the imperative mood: `add`, `fix`, `remove` — not `added`, `fixes`, `removed`
- Lowercase first letter
- No period at the end
- Keep it under 72 characters

### Examples

```
feat(lexer): add string escape sequence handling

fix(memory): correct inverted assert condition in snuk_free

docs(contributing): add branching strategy section

refactor(lexer): rename start_line to line_start for consistency

chore(build): add SnFile as git submodule

perf(lexer): avoid redundant peek in punctuator check
```

### Breaking changes

If a commit introduces a breaking change, add `!` after the type and describe it in the footer:

```
feat(memory)!: rename SnukAllocKind variants to match allocator names

BREAKING CHANGE: SNUK_ALLOC_KIND_TEMP is now SNUK_ALLOC_KIND_LINEAR.
Update all call sites accordingly.
```

---

## Pull Requests

### Before opening a PR

- [ ] Branch is cut from `dev`, not `main`
- [ ] Code builds without warnings (`-Wall -Wextra`)
- [ ] All existing `.snuk` test files still run correctly
- [ ] New behavior is covered by a test file in `tests/`
- [ ] Commit messages follow the convention above

### PR title

Use the same format as a commit message:

```
feat(parser): add support for do/while statements
fix(lexer): handle EOF inside multi-line comment
```

### PR description

Keep it short. What does this change? Why? Is there anything reviewers should pay special attention to?

### Merging

- Squash merge into `dev` for small changes
- Merge commit for larger features where history is meaningful
- No force-pushing to `dev` or `main`

---

## Versioning

Snuk uses [Semantic Versioning](https://semver.org/): `MAJOR.MINOR.PATCH`

| Segment | When to increment |
|---------|------------------|
| `MAJOR` | Breaking language or API change |
| `MINOR` | New feature, backward compatible |
| `PATCH` | Bug fix, backward compatible |

### Pre-release versions

While Snuk is in early development, versions are prefixed `0.x.y`. A `0.x` major means the public API is not yet stable — breaking changes may occur on minor bumps.

```
0.1.0  — first working language (lexer + parser + interpreter)
0.2.0  — custom shape types
0.3.0  — canvas + raylib integration
1.0.0  — stable language spec, public API frozen
```

### Tagging

Releases are tagged on `main`:

```bash
git tag -a v0.1.0 -m "release: v0.1.0 — initial language core"
git push origin v0.1.0
```

---

## Reporting Bugs

Open an issue and include:

- Snuk version (`snuk --version`)
- Operating system and compiler
- The `.snuk` file or REPL input that triggered the bug
- What you expected to happen
- What actually happened

---

## Suggesting Features

Open an issue with the `enhancement` label. Describe:

- What you want to do that you currently cannot
- Why it matters for Snuk's goals
- Any syntax or API you have in mind

Check `FUTURE.md` first — your idea might already be planned.
