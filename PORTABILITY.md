# Portability

Anemo is intended to keep one source project portable while producing native
artifacts for each platform. The source and language pipeline stay shared;
platform-specific backends will produce the appropriate native executable.

The planned backend direction is LLVM, with target triples describing the
native artifact being built. Runtime behavior should be capability-driven:
platform features are selected from available capabilities rather than
hard-coded source-project assumptions.

`anemo targets` currently lists the target profiles, triples, and backend
status:
android-arm64, linux-arm64, linux-x86_64, windows-x86_64, and macos-arm64.
Only `linux-x86_64` is marked as a prototype backend. The remaining entries
are planned and are rejected explicitly if selected with `--target`. The
current compiler still emits x86-64 Linux assembly and does not yet provide
LLVM or platform backends.
