# Anemo CLI

## Current commands

```text
anemo build <file.anm>
anemo run <file.anm>
anemo vortex
anemo update
anemo targets
anemo version
```

The current `build` and `run` commands invoke the prototype x86-64 assembly
pipeline. `targets` exposes target-profile metadata and does not yet compile
for those profiles.

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
