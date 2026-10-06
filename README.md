# Anemo

> **Status: active C17 prototype**

Anemo is an experimental compiled programming language and compiler written in C17. The working prototype accepts `.anm` source code, performs lexical analysis, parsing, semantic checking, and intermediate-representation generation, then emits x86-64 Linux assembly.

The broader cross-platform application-platform ideas in this repository are design goals—not implemented compiler targets.

## Quick verification

On Linux or MinGW, build the compiler:

```bash
make
```

Compile and run the included example:

```bash
./anemo run examples/hello.anm
```

The current backend produces x86-64 Linux ELF artifacts. It is not yet a portable runtime, LLVM backend, WebAssembly compiler, or native application packager.

## What is implemented

```text
.anm source
  → lexer
  → recursive-descent parser
  → AST
  → semantic analysis
  → intermediate representation
  → x86-64 assembly emission
  → assembler/linker
  → Linux executable
```

- Lexer, parser, AST, semantic analysis, IR generation, and x86-64 code generation
- A small language with immutable/mutable bindings, functions, conditionals, loops, expressions, and static types
- Command-line build and run commands
- Vortex interactive shell mode: `anemo vortex`
- Target-profile discovery: `anemo targets` (metadata only for targets that do not yet have a backend)

## Example

```anm
bind greeting = "hello from Anemo"
chant greeting
```

See [examples/](examples/) for runnable programs and [SYNTAX.md](SYNTAX.md) for the implemented grammar.

## Current language

| Area | Prototype support |
| --- | --- |
| Bindings | `bind` (immutable), `morph` (mutable), `shift` (reassignment) |
| Functions | `glyph`, `offer`, named calls and `invoke … with …` |
| Control flow | `fork` / `elseif` / `otherwise` / `seal`; `cycle` |
| Types | `ember` (int), `pulse` (bool), `text` (string), `mist` (void) |
| Backend | x86-64 Linux assembly; up to six function arguments |

## Project layout

- `lexer.c`, `parser.c`, `ast.c` — front end and syntax tree
- `semantic.c` — type and semantic checks
- `ir.c` — intermediate representation
- `codegen.c` — x86-64 assembly emission
- `examples/` — sample Anemo programs
- `docs/` — language, architecture, runtime, and roadmap notes

## Research and engineering directions

The current implementation is a base for exploring intermediate representations, type systems, code generation, and portable-language design. Useful next steps include:

- tests and regression cases for the language front end;
- diagnostics and source locations;
- a more explicit IR validation pass;
- backend portability and measurable code-generation experiments.

These are future directions, not completed capabilities.

## Documentation

- [Language and syntax](SYNTAX.md)
- [Documentation index](docs/README.md)
- [Architecture](docs/ARCHITECTURE.md)
- [Intermediate representation](docs/MIR.md)
- [Type system](docs/TYPE_SYSTEM.md)
- [Testing](docs/TESTING.md)
- [Roadmap](docs/ROADMAP.md)

## Build notes

Linux/MinGW:

```bash
make
```

Windows with MSYS2/MinGW GCC:

```powershell
gcc -std=c17 -Wall -Wextra -Werror -Wno-error=format-truncation -O2 -o anemo.exe main.c lexer.c parser.c ast.c semantic.c ir.c codegen.c utils.c
```

## License

No license is currently declared. Contact the repository owner before reuse.
