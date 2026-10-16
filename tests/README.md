# Anemo tests

The current test entry point is:

```bash
make check
```

The smoke suite verifies:

- all declared target profiles are discoverable
- the example program compiles
- the generated executable prints the expected output
- the version command reports the current prototype version

Additional release-gate checks are available locally:

```bash
make strict-warnings
make sanitizers
```

The strict-warning job treats portability and diagnostics warnings as errors.
The sanitizer job runs the compiler and smoke suite with AddressSanitizer and
UndefinedBehaviorSanitizer on the CI host.

This is a prototype release gate, not a claim that the planned platform
targets are implemented. Each future target must add backend, runtime,
packaging, and device-level validation before it can be listed as supported.
