# Anemo CLI

## Current commands

```text
anemo build [--target <profile>] <file.anm>
anemo run [--target <profile>] <file.anm>
anemo vortex
anemo update
anemo targets
anemo version
```

The current `build` and `run` commands invoke the prototype x86-64 assembly
pipeline. Without `--target`, they use the current `linux-x86_64` profile.
`targets` exposes target-profile metadata. Planned profiles are rejected
explicitly instead of being compiled with the wrong backend.

## Planned project workflow

```text
anemo new <name>
anemo dev
anemo build --release
anemo test
anemo package
anemo debug
anemo profile
anemo add <package>
anemo remove <package>
```

## Target selection

The intended release form is:

```text
anemo build --target android-arm64 --release
anemo build --target macos-arm64 --release
anemo package --all
```

`--target` must be explicit in CI or derived from a configured project target
set. `--all` builds every configured target and reports failures per target.
It must never silently skip a target.

At the current prototype stage, only `linux-x86_64` is buildable. The other
profiles are visible for planning and fail with an actionable diagnostic.

## Diagnostics

Diagnostics include a stable code, source span, severity, explanation, and
actionable suggestion:

```text
ANM204: platform-specific branching detected
help: prefer `when camera.available { ... }`
```

Commands that inspect compiler reasoning are planned:

```text
anemo why-large
anemo why-runtime
anemo why-rebuild
anemo why-slow
```

They should explain retained runtime modules, missed optimizations, semantic
rebuild dependencies, and target-specific decisions.
