# Anemo 0.2.0 development release

## Summary

This release documents the transition from the prototype console compiler
toward the Anemo universal application platform.

## Included

- C17 lexer, parser, semantic checker, prototype IR, and x86-64 code generator
- `glyph`, `bind`, `morph`, `fork`, `cycle`, `offer`, and `chant` syntax
- Vortex interactive shell
- `anemo targets` target-profile discovery command
- explicit `--target` validation with clear planned-backend diagnostics
- target metadata for Android ARM64, Linux ARM64/x86-64, Windows x86-64, and
  macOS Apple Silicon
- reproducible CI build and prototype smoke-test gate
- semantic application model documentation
- language, type, memory, concurrency, MIR, UI, architecture, runtime,
  platform, CLI, security, testing, tooling, roadmap, and release contracts

## Not included yet

- LLVM native backend
- WebAssembly backend
- semantic `app`/`view` parser and compiler support
- native Android, iOS, macOS, Windows, or Linux packaging
- reactive UI runtime and platform renderers
- package manager, LSP, debugger, profiler, and hot reload

## Compatibility

Existing `.anm` programs retain the prototype grammar documented in
`SYNTAX.md`. The new semantic application syntax is a staged design target
and is not accepted by the current parser.

## Known limitations

- current generated binaries target x86-64 Linux
- assembly and linking require the host toolchain
- target profiles are metadata only
- platform permissions, signing, and distribution are not automated

The CI gate validates the current prototype compiler only. It is not a
cross-platform release certification.

## Upgrade guidance

Use the existing syntax for current programs. Read `docs/LANGUAGE.md`,
`docs/MIR.md`, and `docs/UI_MODEL.md` before adopting the planned Anemo 2.0
application model.
