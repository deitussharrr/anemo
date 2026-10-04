# Anemo platform support

## Target profiles

The initial target matrix is:

| Profile | Triple | Native artifact |
| --- | --- | --- |
| Android ARM64 | `aarch64-linux-android` | APK/AAB |
| Linux ARM64 | `aarch64-unknown-linux-gnu` | ELF package |
| Linux x86-64 | `x86_64-unknown-linux-gnu` | ELF package |
| Windows x86-64 | `x86_64-pc-windows-msvc` | PE executable/package |
| macOS Apple Silicon | `aarch64-apple-darwin` | ARM64 app bundle |

Additional planned profiles include iOS ARM64, macOS x86-64, Windows ARM64,
Linux RISC-V, and WebAssembly.

`anemo targets` lists profile metadata. A profile is not considered supported
until its backend, runtime adapter, packaging flow, and release validation are
implemented.

## One source, multiple artifacts

Anemo promises one portable source project, not one universal binary. Each
operating system has different executable formats, SDKs, permissions,
signing, and distribution rules. The build system selects the target profile
and produces a native artifact for that environment.

## Packaging

Release packaging must generate:

- target-specific executable and runtime libraries
- application metadata and permissions
- debug symbols as a separate upload
- reproducible build metadata
- signing instructions or signed output where credentials are configured

Signing credentials never belong in source or build logs. Store them in the
platform's secure CI secret mechanism.

## Compatibility policy

Portable APIs are the default. A platform extension must declare its supported
profiles and provide a fallback or a clear required-capability error.
Compatibility is validated per profile; passing on one target does not imply
passing on another.
