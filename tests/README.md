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

This is a prototype release gate, not a claim that the planned platform
targets are implemented. Each future target must add backend, runtime,
packaging, and device-level validation before it can be listed as supported.
