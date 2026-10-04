# Anemo 0.2.0

**Release type:** Development / prototype release
**Status:** Experimental
**Date:** October 2026

## Overview

Anemo 0.2.0 is an early development release of the Anemo programming
language and compiler. It provides a working C17 compiler pipeline for the
prototype `.anm` language, target-profile metadata, CI smoke checks, and the
initial Anemo 2.0 universal application platform specification.

This release is not production-ready. It does not provide native application
builds for Android, iOS, macOS, Windows, Linux ARM64, RISC-V, or WebAssembly.

## Included

- Lexer, recursive-descent parser, AST, semantic analysis, and prototype IR
- x86-64 Linux assembly generation and native executable linking
- Integer, boolean, string, and void types
- Functions, calls, conditionals, loops, mutation, returns, and output
- Vortex interactive shell
- Example `.anm` programs
- Windows MSI installer configuration
- OTA update command
- `anemo targets` target-profile discovery
- Explicit `--target` selection and unsupported-target diagnostics
- GitHub Actions CI and compiler smoke tests
- Anemo 2.0 architecture and release documentation

## Current commands

```bash
anemo build <file.anm>
anemo build --target linux-x86_64 <file.anm>
anemo run <file.anm>
anemo run --target linux-x86_64 <file.anm>
anemo vortex
anemo targets
anemo update
anemo version
```

## Buildable target

```text
Target:  linux-x86_64
Triple:  x86_64-unknown-linux-gnu
Status:  Prototype
Output:  Native x86-64 Linux executable
```

Example:

```bash
anemo build --target linux-x86_64 examples/hello.anm
```

## Target profiles

| Target | Triple | Status |
|---|---|---|
| Android ARM64 | `aarch64-linux-android` | Planned |
| Linux ARM64 | `aarch64-unknown-linux-gnu` | Planned |
| Linux x86-64 | `x86_64-unknown-linux-gnu` | Prototype |
| Windows x86-64 | `x86_64-pc-windows-msvc` | Planned |
| macOS Apple Silicon | `aarch64-apple-darwin` | Planned |

Planned targets are rejected explicitly until their compiler backend, runtime
adapter, packaging flow, and release validation are implemented.

## Current language

The prototype supports:

- `glyph` function declarations
- `bind` immutable variables
- `morph` mutable variables
- `shift` reassignment
- `fork`, `elseif`, and `otherwise` conditionals
- `cycle`, `break`, and `continue`
- `offer` return statements
- `chant` output statements
- `invoke` and direct function calls
- grouped expressions, arithmetic, boolean operators, and comparisons
- string escape sequences

Example:

```anm
glyph main [] yields ember
chant "anemo says hello"
offer 0
seal
```

## CI and validation

The repository includes:

- GitHub Actions build workflow
- compiler build verification
- CLI smoke tests
- example program compilation checks
- target-profile checks
- unsupported-target validation
- documentation and whitespace checks

The CI workflow validates the prototype compiler only. It is not
cross-platform release certification.

## Known limitations

Not implemented in this release:

- LLVM native backend
- WebAssembly backend
- semantic `app` and `view` syntax
- semantic UI compiler and reactive UI runtime
- adaptive layout and capability-driven APIs
- native Android, iOS, macOS, Windows, or Linux ARM64 packaging
- RISC-V support
- package manager
- stable MIR or runtime ABI
- async runtime
- language server, debugger, profiler, or hot reload
- cross-platform signing and distribution

## Compatibility

Existing `.anm` programs continue to use the prototype grammar documented in
`SYNTAX.md`. The planned Anemo 2.0 semantic application syntax is documented
but is not accepted by the current parser.

## Upgrade guidance

1. Continue using the existing `glyph`-based syntax.
2. Use `linux-x86_64` as the target profile.
3. Run `make check` before distributing changes.
4. Treat all other target profiles as planned.
5. Do not use semantic app examples as executable syntax yet.

## Release status

Anemo 0.2.0 is suitable for language experimentation, compiler development,
syntax testing, architecture prototyping, documentation review, and Linux
x86-64 demonstrations.

It is not suitable for production mobile applications, production desktop
applications, cross-platform distribution, commercial release builds,
security-sensitive applications, or compatibility guarantees across
operating systems.

## Next milestones

1. Versioned typed MIR
2. Interpreter for rapid development
3. LLVM native backend
4. Core runtime and memory model
5. Semantic application parser
6. Semantic UI compiler
7. Capability and permission system
8. Adaptive UI runtime
9. ARM64 and Windows targets
10. Android and macOS packaging
11. WebAssembly backend
12. Stable Anemo 2.0 release process

## Summary

Anemo 0.2.0 establishes the prototype and architecture for Anemo 2.0:

> Describe once. Understand intent. Adapt everywhere. Compile natively.
