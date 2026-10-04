# Anemo testing and benchmarking

## Test layers

1. **Lexer/parser**: token boundaries, grammar, source spans, and recovery.
2. **Semantic**: types, ownership, capabilities, diagnostics, and exhaustiveness.
3. **MIR**: deterministic lowering, effects, requirements, and optimization.
4. **Runtime**: memory, async cancellation, lifecycle, and error propagation.
5. **Semantic UI**: roles, state dependencies, accessibility, and actions.
6. **Platform**: target compilation, packaging, permissions, and startup.
7. **End-to-end**: representative applications on every release profile.

Tests should prefer semantic assertions such as “tap action Login” and “assert
heading Welcome” instead of coordinates or platform widget paths.

## Compatibility matrix

Every release target records:

- compiler and SDK versions
- host and target triples
- build result
- package validation result
- startup and smoke-test result
- known limitations

One target passing does not make another target supported.

## Benchmarks

Benchmarks use the same source, assets, device, workload, and functionality
when comparing Anemo with Flutter, React Native, Compose, .NET MAUI, and native
implementations. Publish methodology with results.

Measure:

- time to first frame
- idle and peak memory
- CPU and GPU utilization
- frame time and dropped frames
- binary and installed size
- runtime module size
- energy per hour
- large-list, image, text, and animation workloads

Performance targets are engineering goals, not promises, until reproducible
measurements exist.
