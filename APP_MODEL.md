# Anemo semantic application model

Anemo is moving from a console-first language toward a semantic application
language. The source describes application intent; the compiler and platform
libraries decide how that intent is rendered and packaged for a target device.

## Source model

An application is made from semantic declarations rather than platform
widgets:

```anm
app Hello {
    navigation {
        Home
        Settings
    }

    view Home {
        heading "Hello"

        action "Continue" {
            navigate Settings
        }
    }
}
```

The initial semantic vocabulary is:

- `app`: application boundary and package identity
- `data`: domain data and serialization schema
- `action`: user or system intent
- `navigation`: destinations and navigation relationships
- `view`: semantic screen content
- `heading`, `show`, and `action`: accessible content and operations
- `adaptive`: rules selected from device capabilities and environment
- `when`: capability-guarded behavior

This syntax is a design target for the next frontend revision. Existing
`glyph` programs remain supported while the semantic frontend is introduced;
they are not silently interpreted as UI declarations.

## Semantic IR

The frontend should lower source into a platform-neutral semantic IR with
these top-level sections:

1. **Domain**: data types, validation, serialization, and actions.
2. **Experience**: views, navigation, content roles, and interaction intents.
3. **Adaptation**: rules for size class, input modality, accessibility settings,
   power state, network state, and available capabilities.
4. **Execution**: ordinary computation, effects, concurrency, and error flow.
5. **Capabilities**: required, optional, and gracefully-degraded services.

Backends consume this IR rather than parsing UI syntax directly. This keeps
accessibility, testing, native rendering, and future non-visual clients
consistent.

## Capability-driven APIs

Applications should ask what a device can do, not which operating system it
runs:

```anm
when camera.available {
    camera.capture()
}

when biometrics.available {
    authenticate()
}
```

Capabilities are runtime values and must support an unavailable path. A
platform library may implement a capability with CameraX, AVFoundation,
Media Foundation, V4L2, Metal, or another native API without exposing that
choice to application code.

## Adaptive behavior

Adaptation is based on environment facts, not hard-coded pixel assumptions:

- compact, medium, and expanded layout classes
- touch, mouse, keyboard, stylus, controller, and voice input
- reduced motion, high contrast, dynamic text size, and screen readers
- low power, memory pressure, offline mode, and network quality
- GPU, accelerator, and storage capabilities

Semantic actions provide enough information for platform libraries to add
labels, focus order, keyboard shortcuts, controller mappings, and voice
commands by default.

## Native build contract

One Anemo source project produces separate native artifacts:

| Profile | Target triple | Artifact direction |
| --- | --- | --- |
| Android ARM64 | `aarch64-linux-android` | APK/AAB with native libraries |
| Linux ARM64 | `aarch64-unknown-linux-gnu` | native ELF package |
| Linux x86-64 | `x86_64-unknown-linux-gnu` | native ELF package |
| Windows x86-64 | `x86_64-pc-windows-msvc` | native PE executable/package |
| macOS Apple Silicon | `aarch64-apple-darwin` | native ARM64 app bundle |

The planned native backend boundary is LLVM. LLVM is responsible for
architecture-specific code generation and ABI details; Anemo remains
responsible for language semantics, semantic IR, capability contracts, and
packaging orchestration. A future WebAssembly backend can reuse the same IR
for web and sandboxed execution.

There is no promise of one binary that runs unchanged on every operating
system. The compatibility promise is one source project, target-aware
compilation, explicit capability checks, and generated native packages.

## Compatibility policy

- Portable code uses Anemo core, standard libraries, and semantic UI APIs.
- Platform extensions are isolated behind capability and package interfaces.
- Unsupported required capabilities fail during build or produce an explicit
  runtime diagnostic; they must not silently become no-ops.
- Every release target is compiled and tested independently.
- Target profiles shown by `anemo targets` are planning metadata until their
  backend and packaging toolchain are implemented.
