# Anemo language specification

## Purpose

Anemo is a strongly typed universal application language. A source project
describes computation, application intent, and capabilities once. Target
backends specialize that description into native artifacts for Android, iOS,
macOS, Windows, Linux, or WebAssembly.

The current compiler implements the prototype `glyph` syntax described in
`SYNTAX.md`. The language revision below is the compatibility target for the
Anemo 2.0 frontend; it must be introduced with diagnostics and migration
support rather than changing the meaning of existing programs unexpectedly.

## Core syntax

```anm
let name = "Anemo"

fn greet(name: String) -> String {
    return "Hello " + name
}

chant greet(name)
```

The prototype equivalents remain valid while migration is staged:

```anm
glyph greet [name: text] yields text
offer "Hello " + name
seal
```

## Application declarations

An application is a semantic boundary:

```anm
app Hello {
    view Home {
        heading "Hello"

        action "Continue" {
            navigate Settings
        }
    }
}
```

`app`, `view`, `action`, `data`, `navigation`, `adaptive`, and `when` are
semantic declarations. They are lowered into the semantic IR; they are not
platform widgets.

## Language requirements

The stable language must define:

- lexical and grammar rules
- name resolution and modules
- inference and explicit type annotations
- functions, closures, structs, enums, traits, and generics
- pattern matching and exhaustive diagnostics
- `Option<T>` and `Result<T, E>` error flow
- structured `async`/`await` concurrency
- deterministic serialization boundaries
- capability declarations and unavailable paths
- source compatibility and edition rules

Each feature needs parser, type-checker, MIR, runtime, diagnostics, and test
coverage before it is considered stable.

## Portability rule

Portable source must use Anemo core and capability interfaces. Direct platform
code is allowed only inside an explicit platform extension:

```anm
platform android {
    // Android-specific escape hatch.
}
```

Platform extensions must be reported by the compiler and isolated from common
code. The compiler must never promise that an extension is available on every
target.
