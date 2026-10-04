# Anemo roadmap

This roadmap is ordered to protect the language and MIR foundations before
adding platform breadth.

## 0.1 Prototype

- lexer, parser, AST, semantic checking
- basic types, functions, control flow
- x86-64 prototype compiler
- `.anm` examples and syntax reference

## 0.2 Compiler foundation

- versioned typed MIR
- interpreter for fast development execution
- constant folding and dead-code elimination
- first LLVM native backend for Linux x86-64
- deterministic diagnostics and golden tests

## 0.3 Core runtime

- memory and resource contracts
- modules and package metadata
- structured async execution
- filesystem, networking, and error APIs
- runtime dependency closure and size reports

## 0.4 Semantic UI

- `app`, `view`, `content`, `action`, and navigation declarations
- semantic UI AST and MIR lowering
- desktop preview renderer
- reactive state and fine-grained invalidation

## 0.5 Adaptive engine

- capabilities and permission declarations
- input modality and form-factor adaptation
- accessibility roles, focus, labels, and reduced motion
- semantic test primitives

## 0.6 Additional native targets

- ARM64 native lowering
- Windows x86-64 packaging
- macOS Apple Silicon packaging
- Android ARM64 packaging

## 0.7 Web and ecosystem

- WebAssembly backend
- Web capability adapter
- package manager and platform package layout
- FFI contracts
- language server and formatter

## 1.0 universal application platform

Stabilize language edition, MIR ABI, runtime ABI, UI model, capability model,
package format, compiler CLI, target matrix, and release validation.

The roadmap is not a promise of dates. A feature is complete only when its
specification, implementation, diagnostics, tests, and documentation agree.
