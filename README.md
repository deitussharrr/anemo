# Anemo Compiler

![Anemo Logo](installer/anemo-logo.png)

Anemo is a compiled programming language with `.anm` source files.
This project implements a full C17 compiler pipeline that emits x86-64 Linux assembly, assembles with `as`, and links to a standalone ELF executable.

## Features

- Full compile pipeline (lexer, parser, semantic check, IR, codegen)
- Custom language keywords (`glyph`, `bind`, `morph`, `fork`, `cycle`, etc.)
- CLI build/run tooling
- Interactive shell mode: `Vortex` (`anemo vortex`)
- Windows MSI installer with branded wizard UI

## CLI

```bash
./anemo build program.anm
./anemo run program.anm
./anemo vortex
./anemo update
./anemo version
```

Running `anemo` with no arguments prints ASCII art and shows available commands.

## OTA Updates

- Anemo automatically checks GitHub releases for updates (once per day by default).
- Manual update command:

```bash
./anemo update
```

Environment variables:

- `ANEMO_GITHUB_REPO` (default: `tussh/anemo`)  
  Example: `owner/repo`
- `ANEMO_DISABLE_UPDATE_CHECK=1` to disable automatic checks

## Build

Linux/MinGW:

```bash
make
```

Windows (MSYS2 MinGW GCC example):

```powershell
gcc -std=c17 -Wall -Wextra -Werror -Wno-error=format-truncation -O2 -o anemo.exe main.c lexer.c parser.c ast.c semantic.c ir.c codegen.c utils.c
```

## Language Summary

- Immutable variable: `bind`
- Mutable variable: `morph`
- Reassignment (mutable only): `shift`
- Function definition: `glyph`
- Function return: `offer`
- Conditional: `fork` / `elseif` / `otherwise` / `seal`
- Loop: `cycle` / `break` / `continue` / `seal`
- Function call expressions:
  - `invoke <name> with <arg1>, <arg2>`
  - `<name>(<arg1>, <arg2>)`
- Grouping: `(` `)`
- Print statement: `chant <expr>`

Types:

- `ember` (int)
- `pulse` (bool)
- `text` (string)
- `mist` (void)

Boolean literals:

- `yes`
- `no`

Boolean operators:

- `both` (and)
- `either` (or)
- `flip` (not)

Comparisons:

- `same`, `diff`, `less`, `more`, `atmost`, `atleast`

Complete grammar and syntax reference:

- [SYNTAX.md](SYNTAX.md)

## Example Program

See [examples/hello.anm](examples/hello.anm) and other files in `examples/`.

Run:

```bash
./anemo run examples/hello.anm
```

## Compiler Pipeline

1. Lexer (`lexer.c`)
2. Recursive descent parser (`parser.c`)
3. AST model (`ast.c`)
4. Semantic analysis (`semantic.c`)
5. IR generation (`ir.c`)
6. x86-64 assembly emission (`codegen.c`)
7. Assembly emission to `.s`
8. Assembly+link to executable via `as` and `gcc`

## Notes

- No interpreter is used.
- Generated binaries target x86-64 Linux ELF.
- Current calling convention support: up to 6 arguments per function.

## MSI Installer (Windows)

Installer files are in [`installer/`](installer/):

- `anemo.wxs` (WiX v4 definition)
- `build-msi.ps1` (build script)
- `README.md` (installer instructions)

Build MSI (creates `built/<version>/anemo-<version>.msi`):

```powershell
.\installer\build-msi.ps1
```
