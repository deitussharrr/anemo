# Anemo Syntax Reference

This document describes the syntax accepted by the current Anemo compiler implementation.

## 1. Source Structure

- A program is one or more `glyph` function declarations.
- Newlines are statement separators.
- Blocks are closed with `seal`.
- Comments start with `#` and continue to end of line.

## 2. Lexical Rules

### 2.1 Whitespace

- Spaces, tabs, and carriage returns are ignored.
- Newlines are tokens and are syntactically meaningful.

### 2.2 Comments

```anm
# this is a comment
```

### 2.3 Identifiers

- Start with letter or `_`
- Continue with letters, digits, or `_`
- Case-sensitive

### 2.4 Literals

- Integer literal: `123`
- String literal: `"hello"`
- Boolean literals: `yes`, `no`

Supported string escapes:

- `\n`
- `\t`
- `\r`
- `\"`
- `\\`

### 2.5 Types

- `ember` = integer
- `pulse` = boolean
- `text` = string
- `mist` = void

## 3. Keywords

Control/definitions:

- `glyph`, `yields`, `bind`, `morph`, `shift`
- `fork`, `otherwise`, `cycle`, `offer`, `seal`
- `invoke`, `with`, `chant`

Type/literal keywords:

- `ember`, `pulse`, `text`, `mist`, `yes`, `no`

Logical/comparison keywords:

- `both`, `either`, `flip`
- `same`, `diff`, `less`, `more`, `atmost`, `atleast`

## 4. Program Grammar (EBNF)

```ebnf
program         ::= nl* function (nl+ function)* nl* EOF

function        ::= "glyph" IDENT "[" param_list? "]" "yields" type nl+
                    block
                    "seal" line_end

param_list      ::= param ("," param)*
param           ::= IDENT ":" type
type            ::= "ember" | "pulse" | "text" | "mist"

block           ::= { statement | nl }

statement       ::= bind_stmt
                  | morph_stmt
                  | shift_stmt
                  | fork_stmt
                  | cycle_stmt
                  | offer_stmt
                  | chant_stmt
                  | expr_stmt

bind_stmt       ::= "bind" IDENT "=" expr line_end
morph_stmt      ::= "morph" IDENT "=" expr line_end
shift_stmt      ::= "shift" IDENT "=" expr line_end

fork_stmt       ::= "fork" expr nl+
                    block
                    [ "otherwise" nl+ block ]
                    "seal" line_end

cycle_stmt      ::= "cycle" expr nl+
                    block
                    "seal" line_end

offer_stmt      ::= "offer" [ expr ] line_end
chant_stmt      ::= "chant" expr line_end
expr_stmt       ::= expr line_end

line_end        ::= nl+ | before("seal" | "otherwise" | EOF)
nl              ::= NEWLINE
```

## 5. Expression Grammar and Precedence

Highest precedence at top.

```ebnf
expr            ::= either_expr

either_expr     ::= both_expr { "either" both_expr }
both_expr       ::= eq_expr   { "both" eq_expr }
eq_expr         ::= cmp_expr  { ("same" | "diff") cmp_expr }
cmp_expr        ::= add_expr  { ("less" | "more" | "atmost" | "atleast") add_expr }
add_expr        ::= mul_expr  { ("+" | "-") mul_expr }
mul_expr        ::= unary_expr{ ("*" | "/") unary_expr }
unary_expr      ::= ("-" | "flip") unary_expr | primary

primary         ::= INT
                  | STRING
                  | "yes"
                  | "no"
                  | IDENT
                  | call_expr

call_expr       ::= "invoke" IDENT [ "with" expr { "," expr } ]
```

Notes:

- There are currently no grouping parentheses in expressions.
- Call syntax is keyword-based (`invoke ... with ...`), not `name(...)`.

## 6. Statements

### 6.1 Immutable binding

```anm
bind x = 10
```

### 6.2 Mutable binding

```anm
morph counter = 3
```

### 6.3 Reassignment

```anm
shift counter = counter - 1
```

### 6.4 Conditional

```anm
fork x more 0
chant "positive"
otherwise
chant "non-positive"
seal
```

### 6.5 Loop

```anm
cycle counter more 0
chant counter
shift counter = counter - 1
seal
```

### 6.6 Return

```anm
offer 0
```

Void-returning (`mist`) glyphs may use `offer` with no value.

### 6.7 Print

```anm
chant "hello"
```

## 7. Function Rules

Example:

```anm
glyph add [a: ember, b: ember] yields ember
offer a + b
seal
```

Required entrypoint:

```anm
glyph main [] yields ember
offer 0
seal
```

Compiler-enforced constraints:

- Program must define `glyph main`.
- `main` must have empty parameter list `[]`.
- `main` must `yields ember`.
- Non-`mist` glyphs must contain at least one `offer <expr>`.
- Duplicate glyph names are not allowed.

## 8. Type Semantics

`+`, `-`, `*`, `/`:

- operands: `ember`, result: `ember`

`both`, `either`, `flip`:

- operands: `pulse`, result: `pulse`

`less`, `more`, `atmost`, `atleast`:

- operands: `ember`, result: `pulse`

`same`, `diff`:

- operands must have the same type, result: `pulse`

`shift`:

- target must be previously declared with `morph`
- assigned value type must match original type

`chant`:

- accepts `ember`, `pulse`, or `text`

## 9. Full Example

```anm
glyph add [a: ember, b: ember] yields ember
offer a + b
seal

glyph main [] yields ember
bind x = invoke add with 7, 5
chant "sum"
chant x

morph counter = 3
cycle counter more 0
chant counter
shift counter = counter - 1
seal

offer 0
seal
```

## 10. Additional Examples

### 10.1 Variables (`bind`, `morph`, `shift`)

```anm
glyph main [] yields ember
bind name = "anemo"
morph score = 10
shift score = score + 5
chant name
chant score
offer 0
seal
```

### 10.2 Conditionals (`fork`, `otherwise`)

```anm
glyph main [] yields ember
bind age = 18
fork age atleast 18
chant "adult"
otherwise
chant "minor"
seal
offer 0
seal
```

### 10.3 Loops (`cycle`)

```anm
glyph main [] yields ember
morph n = 5
cycle n more 0
chant n
shift n = n - 1
seal
offer 0
seal
```

### 10.4 Functions and Calls (`glyph`, `invoke with`)

```anm
glyph multiply [a: ember, b: ember] yields ember
offer a * b
seal

glyph main [] yields ember
bind result = invoke multiply with 6, 7
chant result
offer 0
seal
```

### 10.5 Booleans and Logic (`pulse`, `both`, `either`, `flip`)

```anm
glyph main [] yields ember
bind is_weekend = yes
bind is_holiday = no
bind can_rest = is_weekend either is_holiday
bind must_work = flip can_rest
chant can_rest
chant must_work
offer 0
seal
```

### 10.6 Equality and Comparisons (`same`, `diff`, `less`, `more`)

```anm
glyph main [] yields ember
bind a = 10
bind b = 20
chant a less b
chant a same b
chant a diff b
offer 0
seal
```

### 10.7 String Escapes

```anm
glyph main [] yields ember
chant "line1\nline2"
chant "tab:\tvalue"
chant "quote: \"anemo\""
chant "slash: \\"
offer 0
seal
```

### 10.8 Void Function (`mist`)

```anm
glyph log_value [x: ember] yields mist
chant x
offer
seal

glyph main [] yields ember
invoke log_value with 42
offer 0
seal
```
