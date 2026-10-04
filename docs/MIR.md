# Anemo middle intermediate representation

## Role

MIR is the stable boundary between language semantics and target backends.
Frontends lower into MIR; optimizers, analyzers, native backends, and the
interpreter consume MIR. No backend should parse source syntax directly.

## Layers

```text
source AST
  -> typed semantic AST
  -> domain/application/UI IR
  -> execution MIR
  -> target lowering
```

MIR contains five cooperating sections:

1. **Domain**: data layouts, validation, serialization, and actions.
2. **Experience**: semantic views, roles, navigation, and interaction effects.
3. **Adaptation**: capability predicates and environment-dependent variants.
4. **Execution**: typed values, control flow, calls, async tasks, and effects.
5. **Requirements**: runtime modules, permissions, target constraints, and FFI.

## Required properties

MIR is:

- typed after semantic analysis
- explicit about ownership and effects
- target-neutral before lowering
- serializable for diagnostics and incremental compilation
- versioned independently from source syntax
- deterministic for the same source and build configuration

## Example

```text
view Home
  node heading "Hello"
  node action "Continue"
    effect navigate Settings
  requires capability navigation
```

An action is represented as an intent and effect, not as a platform click
callback. A renderer can map it to touch, mouse, keyboard, controller, or
voice interaction.

## Optimization passes

The initial pass pipeline is:

1. constant folding
2. dead code and unreachable semantic-node elimination
3. capability requirement propagation
4. effect and dependency analysis
5. closure conversion
6. ownership and escape analysis
7. specialization and inlining
8. UI static-subtree elimination
9. target lowering and link-time optimization

Every pass must preserve source diagnostics and provide an explainable reason
when a requested optimization cannot be applied.

## Backends

The intended production backend is LLVM for native targets. A WebAssembly
backend and interpreter/JIT can consume the same MIR. Current repository code
has a prototype execution IR and x86-64 assembly backend; it is not yet this
versioned MIR contract.
