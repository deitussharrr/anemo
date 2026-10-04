# Anemo documentation

Anemo documentation is split into stable language contracts, implementation
architecture, and release planning.

## Language contracts

- [LANGUAGE.md](LANGUAGE.md): source language, application declarations, and
  portability rules
- [TYPE_SYSTEM.md](TYPE_SYSTEM.md): primitive, composed, user-defined, and
  semantic UI types
- [MEMORY_MODEL.md](MEMORY_MODEL.md): ownership, resource lifetimes, and ABI
- [CONCURRENCY.md](CONCURRENCY.md): structured async execution and UI state
- [MIR.md](MIR.md): platform-neutral middle intermediate representation
- [UI_MODEL.md](UI_MODEL.md): semantic UI, reactivity, adaptation, and
  accessibility

## Platform and implementation contracts

- [ARCHITECTURE.md](ARCHITECTURE.md): compiler and runtime boundaries
- [RUNTIME.md](RUNTIME.md): modular runtime and native adapter contract
- [PLATFORMS.md](PLATFORMS.md): target profiles, packaging, and support policy
- [CLI.md](CLI.md): command-line interface and build workflow
- [SECURITY.md](SECURITY.md): capabilities, permissions, FFI, and supply chain
- [TESTING.md](TESTING.md): compiler, semantic, platform, and benchmark testing
- [TOOLING.md](TOOLING.md): formatter, LSP, debugger, profiler, and diagnostics

## Planning and releases

- [ROADMAP.md](ROADMAP.md): staged implementation milestones
- [RELEASE.md](RELEASE.md): release process and artifact checklist
- [RELEASE_NOTES.md](RELEASE_NOTES.md): current development release notes

The prototype grammar remains in [../SYNTAX.md](../SYNTAX.md). Documents that
describe future syntax or backends label those features as planned; they do
not claim that the current C compiler already implements them.
