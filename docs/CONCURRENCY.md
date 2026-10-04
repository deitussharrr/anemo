# Anemo concurrency model

## Structured asynchronous execution

Anemo uses structured concurrency. An async operation belongs to a parent
scope and cannot silently outlive that scope.

```anm
async fn loadProfile() {
    let response = await http.get("/profile")
    return response.json()
}
```

Cancellation propagates from parent to child. Errors are values and must be
handled or returned through `Result`.

## Parallel work

```anm
parallel {
    profile = loadProfile()
    messages = loadMessages()
}
```

The block joins before leaving scope. The compiler may schedule independent
operations concurrently or execute them sequentially when a target is
resource-constrained, provided observable semantics are preserved.

## Runtime scheduler

The runtime exposes one scheduler contract. Platform adapters map it to:

- Android/iOS event loops and worker pools
- macOS/Linux native threads and event sources
- Windows native executors
- Web workers and browser event mechanisms

Application code must not depend on a particular executor.

## Shared state

Prefer message passing and immutable values. Shared mutable state requires an
explicit synchronization primitive or actor. Data races are compile-time
errors for safe Anemo code; `unsafe` FFI may opt out with diagnostics.

## UI rule

UI state updates are serialized through the platform UI executor. Background
tasks return values or messages; they do not mutate render nodes directly.
This enables fine-grained reactive updates and prevents platform-specific
threading bugs.
