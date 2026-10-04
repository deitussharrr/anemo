# Anemo memory model

## Goals

The memory model must be safe by default, predictable enough for native
applications, and practical for mobile devices. It must not force developers
to manually manage ordinary application memory.

## Representation

- immutable small values use registers or stack storage where profitable
- owned values have one clear lifetime owner
- shared values use reference-counted storage
- cycles are detected or prevented by the runtime
- large buffers and platform resources use explicit resource handles
- unsafe native memory is isolated behind audited FFI boundaries

The compiler may choose a different representation when observable language
semantics are preserved.

## Ownership and borrowing direction

Anemo will use ownership information internally, but the surface syntax should
not copy Rust wholesale. The initial rules are:

- values are moved when ownership is transferred
- immutable borrows may coexist
- mutable access is exclusive for its duration
- escaping references are rejected
- captured closures retain only values that actually escape

Diagnostics should suggest cloning, moving, or changing a value's ownership
rather than exposing implementation details.

## Runtime resources

Files, sockets, cameras, GPU objects, and platform handles are resources.
They must have deterministic release paths and an error result for failed
release or use. A platform runtime may add a safety fallback, but finalization
is not the primary correctness mechanism.

## Optimization

The optimizer may apply stack promotion, copy elision, escape analysis,
reference-count batching, and dead-runtime elimination. These are
implementation details and must not change observable destruction order where
the language specifies one.

## ABI

The MIR ABI must describe value layout, alignment, ownership transfer, string
encoding, error representation, and FFI ownership. Every native target gets a
versioned ABI contract. Generated code must not assume that pointers,
integers, or handles have the same width on every target.
