# Anemo architecture

## Compilation pipeline

```text
source
  -> lexer
  -> parser
  -> typed semantic AST
  -> application/domain/UI IR
  -> Anemo MIR
  -> optimization and adaptation
  -> target lowering
  -> native/WASM backend
  -> platform packaging
```

The frontend owns language meaning. The semantic IR owns application intent.
The MIR owns typed execution and requirements. Backends own target code
generation. Packaging owns manifests, permissions, signing inputs, and
platform artifact layout.

## Repository boundaries

The current repository is a C17 prototype with the following pipeline:

1. `lexer.c`: tokenization
2. `parser.c`: recursive-descent parsing
3. `ast.c`: source tree ownership
4. `semantic.c`: type and rule validation
5. `ir.c`: prototype execution IR
6. `codegen.c`: x86-64 assembly emission
7. `main.c`: CLI and build orchestration

The universal architecture adds these future boundaries:

```text
compiler/
  frontend/
  semantics/
  mir/
  optimizer/
  backends/
runtime/
platforms/
packages/
tooling/
```

The existing implementation should be migrated incrementally. A new backend
must not duplicate parsing or semantic analysis.

## Backend strategy

LLVM is the planned native backend boundary. It provides instruction
selection, register allocation, ABI lowering, and target optimization for
ARM64, x86-64, and RISC-V. Anemo remains responsible for lowering its typed
MIR to LLVM IR and linking the required runtime modules.

WebAssembly is a separate target backend with browser-oriented capabilities.
An interpreter or JIT is useful for development and previews, but production
native targets use AOT compilation when platform policy permits.

## Invariants

- source semantics are independent of operating system and CPU
- platform branches are explicit and diagnosed
- semantic nodes remain available to accessibility and testing tooling
- required capabilities are visible in MIR and packaging metadata
- generated artifacts identify their target profile and compiler version
- unsupported targets fail clearly instead of silently falling back
