# Anemo security model

## Capability permissions

Sensitive capabilities must be declared by the application or package:

```text
camera
microphone
location
contacts
bluetooth
filesystem
notifications
biometrics
```

The compiler and packager translate declarations into platform permission
metadata. Runtime access still requires the platform's user authorization.
Denied permissions produce typed errors and do not grant a silent fallback.

## Sandboxing

Application code runs within the target platform's sandbox. The Anemo runtime
must not broaden permissions beyond the package declaration. WebAssembly
targets additionally follow browser sandbox rules.

## FFI

FFI is an explicit escape hatch for C, C++, Rust, Swift, Kotlin/Java, and
Objective-C integrations. FFI declarations must specify:

- target profiles
- calling convention and ABI
- ownership of arguments and return values
- thread and async requirements
- required capabilities
- error behavior

Unsafe FFI is isolated and reported in release metadata.

## Supply chain

Packages should be pinned by version and integrity hash. Release builds must
record compiler, runtime, target, package, and SDK versions. Dependency
licenses and security advisories belong in release reports.

Never commit signing keys, tokens, passwords, private certificates, or device
credentials. Build logs must redact secrets.
