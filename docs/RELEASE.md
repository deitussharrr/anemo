# Anemo release process

## Versioning

Anemo uses semantic versioning for released compiler and runtime interfaces.
Breaking language, MIR, runtime, package, or CLI changes require a major
version or an explicitly named language edition.

## Release checklist

Before publishing:

1. update version and release notes
2. run formatting, compiler, semantic, documentation, and `make check` checks
3. build every claimed target profile
4. run target smoke tests and package validation
5. generate debug symbols and source metadata
6. produce dependency, license, and security reports
7. verify no credentials or private files are in artifacts
8. record compiler, runtime, SDK, and package versions
9. publish checksums and artifact manifests
10. attach known limitations and rollback guidance

## Artifact layout

```text
dist/<version>/
  android/<app>.aab
  ios/<app>.ipa-or-app/
  macos/<app>.app/
  windows/<app>.exe
  linux/<app>-<arch>
  web/<app>.wasm
  symbols/
  checksums.txt
  manifest.json
```

Only implemented and validated targets may appear as supported release
artifacts. Planned profiles remain visible as planned, never as empty or
placeholder downloads.

The repository CI gate currently validates only the prototype x86-64 Linux
compiler. It does not certify Android, Apple, Windows, ARM64, RISC-V, or Web
artifacts.

## Anemo 1.0 scope and status

Anemo 1.0 is a future stable release, not the current 0.2.0 development
release. Its deliberately limited scope is the implemented `.anm` language
and CLI, stable diagnostics and interfaces, reproducible compiler builds, and
the validated `linux-x86_64` native backend. A target profile is not a
supported backend merely because it is listed by `anemo targets`.

LLVM, WebAssembly, semantic application/UI syntax, an adaptive runtime,
runtime ABI guarantees, and native Android, iOS, macOS, Windows, or ARM64
packaging are outside this 1.0 scope. They require separate implementation
and release validation before being considered for a later edition.

The 1.0 gate must pass `make check`, `make strict-warnings`, and
`make sanitizers`, in addition to the artifact, reproducibility, and security
checks below. Until that gate is met, documentation and version output must
continue to identify the project as development/prototype status.

## Reproducibility

Release builds pin source revision, dependencies, SDKs, target triples, and
compiler configuration. The manifest records these inputs and the checksum
for every artifact.

## Current release status

The current repository release is a C17 prototype. It emits x86-64 Linux
assembly and has target-profile metadata, but it does not yet provide LLVM,
WASM, semantic app parsing, or native packaging for the listed profiles.
