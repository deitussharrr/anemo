# Anemo

![Anemo Logo](installer/anemo-logo.png)

> **Understand more. Compile less. Ship less. Do less work. Run faster.**

Anemo is a compiled programming language and planned universal application
platform. The long-term goal is one semantic source project that can be
specialized into native applications for Android, iOS, macOS, Windows, Linux,
and WebAssembly.

Anemo is not one universal binary. Each operating system and architecture gets
its own native artifact, while the developer maintains one portable source
project. The compiler uses semantic application intent, capability-driven
APIs, adaptive UI, and target-specific backends to reduce compatibility work.

> **Current status:** the repository contains a working C17 prototype compiler
> for the existing `.anm` language. It emits x86-64 Linux assembly. The
> universal semantic frontend, LLVM backend, runtime, and native packaging
> layers are specified and being introduced incrementally.

## Features

- Full compile pipeline (lexer, parser, semantic check, IR, codegen)
- Custom language keywords (`glyph`, `bind`, `morph`, `fork`, `cycle`, etc.)
- CLI build/run tooling
- Interactive shell mode: `Vortex` (`anemo vortex`)
- Windows MSI installer with branded wizard UI
- Portability target profiles (`anemo targets`)

## Universal application model

The planned application language describes intent instead of platform widgets:

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

Anemo will use semantic roles such as `heading`, `action`, `form`, `list`, and
`navigation` to support adaptive layouts, accessibility, keyboard/controller/
voice input, semantic testing, and platform-native rendering.

Capability-driven code asks what a device can do instead of branching on its
operating system:

```anm
when camera.available {
    camera.capture()
}
```

## Architecture

```text
Anemo source
  -> lexer/parser
  -> typed semantic AST
  -> application/domain/UI IR
  -> Anemo MIR
  -> optimization and adaptation
  -> LLVM native backend or WebAssembly backend
  -> platform runtime and packaging
```

The planned native targets are Android ARM64, Linux ARM64/x86-64, Windows
x86-64, and macOS Apple Silicon, followed by additional Apple, RISC-V, and web
targets. Run `anemo targets` to inspect the current profile metadata.

The language foundation is specified in:

- [docs/README.md](docs/README.md)
- [docs/LANGUAGE.md](docs/LANGUAGE.md)
- [docs/TYPE_SYSTEM.md](docs/TYPE_SYSTEM.md)
- [docs/MEMORY_MODEL.md](docs/MEMORY_MODEL.md)
- [docs/CONCURRENCY.md](docs/CONCURRENCY.md)
- [docs/MIR.md](docs/MIR.md)
- [docs/UI_MODEL.md](docs/UI_MODEL.md)

Implementation, platform, and release documentation:

- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)
- [docs/RUNTIME.md](docs/RUNTIME.md)
- [docs/PLATFORMS.md](docs/PLATFORMS.md)
- [docs/CLI.md](docs/CLI.md)
- [docs/SECURITY.md](docs/SECURITY.md)
- [docs/TESTING.md](docs/TESTING.md)
- [docs/TOOLING.md](docs/TOOLING.md)
- [docs/ROADMAP.md](docs/ROADMAP.md)
- [docs/RELEASE.md](docs/RELEASE.md)
- [docs/RELEASE_NOTES.md](docs/RELEASE_NOTES.md)

## Current CLI

```bash
./anemo build program.anm
./anemo run program.anm
./anemo vortex
./anemo update
./anemo targets
./anemo version
```

Running `anemo` with no arguments prints ASCII art and shows available commands.

The current `build` and `run` commands target the prototype x86-64 Linux
backend. Target profiles shown by `anemo targets` are planning metadata until
their compiler backend, runtime adapter, packaging flow, and release tests are
implemented.

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

## Current language summary

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

## Documentation

Start with the [documentation index](docs/README.md).

### Product and platform design

- [APP_MODEL.md](APP_MODEL.md): semantic application model
- [PORTABILITY.md](PORTABILITY.md): native target and compatibility overview
- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md): compiler boundaries and backends
- [docs/RUNTIME.md](docs/RUNTIME.md): modular runtime and platform adapters
- [docs/PLATFORMS.md](docs/PLATFORMS.md): target profiles and packaging
- [docs/UI_MODEL.md](docs/UI_MODEL.md): semantic UI, reactivity, and adaptation

### Language and implementation contracts

- [SYNTAX.md](SYNTAX.md): implemented prototype grammar
- [docs/LANGUAGE.md](docs/LANGUAGE.md): language evolution and app syntax
- [docs/TYPE_SYSTEM.md](docs/TYPE_SYSTEM.md): type system
- [docs/MEMORY_MODEL.md](docs/MEMORY_MODEL.md): ownership and resource model
- [docs/CONCURRENCY.md](docs/CONCURRENCY.md): structured concurrency
- [docs/MIR.md](docs/MIR.md): platform-neutral intermediate representation

### Engineering and releases

- [docs/CLI.md](docs/CLI.md): current and planned commands
- [docs/SECURITY.md](docs/SECURITY.md): permissions, FFI, and supply chain
- [docs/TESTING.md](docs/TESTING.md): test matrix and benchmarks
- [docs/TOOLING.md](docs/TOOLING.md): formatter, LSP, debugger, and profiler
- [docs/ROADMAP.md](docs/ROADMAP.md): staged implementation roadmap
- [docs/RELEASE.md](docs/RELEASE.md): release checklist and artifact policy
- [docs/RELEASE_NOTES.md](docs/RELEASE_NOTES.md): current development release

## Release status

The current development release is **0.2.0**. It includes the prototype
compiler, Vortex shell, target profile discovery, and the Anemo 2.0
specification set. It does not yet include LLVM, WebAssembly, semantic `app`
parsing, native mobile/desktop packaging, or the adaptive runtime.

## MSI Installer (Windows)

Installer files are in [`installer/`](installer/):

- `anemo.wxs` (WiX v4 definition)
- `build-msi.ps1` (build script)
- `README.md` (installer instructions)

Build MSI (creates `built/<version>/anemo-<version>.msi`):

```powershell
.\installer\build-msi.ps1
```
