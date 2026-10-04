# Anemo type system

## Goals

Anemo types provide memory safety, predictable native performance, and useful
inference without requiring annotations for obvious local values.

## Built-in types

The core types are:

```text
Int       signed machine-independent integer
UInt      unsigned machine-independent integer
Float     IEEE floating-point value
Bool      true or false
Char      Unicode scalar value
String    UTF-8 text
Bytes     byte sequence
```

Integer widths used for ABI and serialization must be explicit (`Int32`,
`UInt64`, and so on). `Int` is the source-level general-purpose integer and
has a target-defined efficient representation documented by the ABI.

## Composed types

```text
Array<T>
Map<K, V>
Set<T>
Option<T>
Result<T, E>
```

`Option<T>` represents an absent or present value. `Result<T, E>` represents
success or failure. Neither uses null as an implicit value.

## User types

```anm
struct User {
    id: UUID
    name: String
}

enum LoadState {
    Loading
    Ready(User)
    Failed(String)
}
```

Enums are algebraic data types. `match` must be exhaustive unless an explicit
fallback is supplied.

## Type checking

The checker performs:

1. lexical scope and module resolution
2. declaration and duplicate-name checks
3. inference for local expressions
4. generic constraint solving
5. ownership/effect checks
6. capability and platform-availability checks
7. exhaustive-pattern checks

Errors must identify the source span, the inferred types, and a concrete
correction where possible.

## Semantic UI types

UI declarations are typed semantic values, not arbitrary widget trees:

- `Heading`
- `Text`
- `Image`
- `Action`
- `Input`
- `Form`
- `List`
- `Navigation`
- `Dialog`
- `Menu`
- `Toolbar`
- `Media`

Each semantic value carries role, label, state dependencies, effects, and
availability metadata for accessibility and target adaptation.
