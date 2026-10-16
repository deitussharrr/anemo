# Anemo 1.0 stable release checklist

This checklist is the release gate for Anemo 1.0. A checked item must have
evidence in CI, a reproducible command, or a linked release artifact. The
checklist must not be treated as complete merely because a feature is
documented.

## Current decision

**Anemo is not 1.0 stable yet.** The current release is `0.2.0`, a prototype
that builds the implemented `.anm` grammar for Linux x86-64. Android, iOS,
macOS, Windows, ARM64, RISC-V, and WebAssembly remain planned targets.

## Compiler correctness

- [ ] A clean checkout builds with GCC and Clang.
- [ ] Strict warnings pass with `-Wall -Wextra -Wpedantic -Werror`.
- [ ] `make check` passes on a clean Linux x86-64 runner.
- [ ] AddressSanitizer and UndefinedBehaviorSanitizer checks pass.
- [ ] Positive fixtures cover arithmetic, functions, branching, loops,
  mutation, strings, calls, and returns.
- [ ] Negative fixtures cover malformed syntax, type errors, missing symbols,
  integer overflow, void values, and unsupported escapes.
- [ ] Generated code is validated for the documented x86-64 ABI, including
  stack alignment, register preservation, argument passing, and return values.
- [ ] Dynamic-array growth and file-size arithmetic is checked for overflow.

## Target and runtime claims

- [ ] `anemo targets` lists every profile with an accurate status.
- [ ] Unsupported profiles fail with stable, actionable diagnostics.
- [ ] Each supported profile has a backend, runtime adapter, packaging flow,
  CI coverage, and a clean-machine smoke test.
- [ ] Release artifacts are never published for planned profiles.
- [ ] Compiler, runtime, and generated-code ABI versions are documented.

## CLI and security

- [ ] `version`, `targets`, `help`, `build`, `run`, and `check` behavior is
  tested from a fresh install.
- [ ] Missing files, unreadable files, oversized files, long paths, and output
  collisions produce explicit errors.
- [ ] Build and update process execution does not interpolate untrusted input
  into a shell command, or has platform-specific equivalent protections.
- [ ] Automatic update checks are bounded, disableable, and covered by tests.
- [ ] Release archives contain no credentials, private files, or generated
  local artifacts.

## Documentation and support

- [ ] README and all linked reference documents distinguish implemented and
  planned functionality.
- [ ] Quick-start instructions work from a clean supported environment.
- [ ] Migration, compatibility, security reporting, and bug-reporting
  policies are published.
- [ ] Release notes identify known limitations and rollback guidance.

## Release engineering

- [ ] CI runs build, smoke, strict-warning, sanitizer, documentation, and
  whitespace checks.
- [ ] A versioned changelog and release notes are reviewed.
- [ ] The exact release commit is tagged `v1.0.0` only after every blocker is
  checked.
- [ ] Source archives and binaries have reproducible manifests and SHA-256
  checksums.
- [ ] Artifacts are verified from a clean environment after publication.

## Exit rule

Do not call Anemo 1.0 stable while any compiler-correctness, security,
supported-target, or clean-environment validation item is unchecked. If the
scope is intentionally limited to Linux x86-64, publish that limitation as
the product definition rather than implying universal platform support.
