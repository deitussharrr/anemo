# Anemo tooling

## Formatter and language server

The formatter owns canonical layout for source, semantic UI, and project
configuration. The language server provides:

- completion and type information
- diagnostics with stable codes
- go-to-definition and references
- rename and safe refactoring
- capability and target warnings
- semantic UI preview metadata

Tooling consumes the same typed AST and MIR as the compiler. It must not
reimplement language rules.

## Debugger

The debugger should expose source, typed AST, MIR, semantic UI tree, render
graph, state, async tasks, memory, capabilities, and native platform calls.
Breakpoints should be valid on semantic actions and views, not only machine
instruction locations.

## Profiler

`anemo profile` is planned to report startup, CPU, GPU, allocations, frame
time, network, battery, and runtime-module costs. Profiling output must retain
source and semantic names so developers can explain why work occurred.

## Preview

Universal preview should render compact, medium, expanded, touch, keyboard,
controller, reduced-motion, and low-power profiles from one source. Preview is
an analysis aid; it does not replace target-device validation.

## Explainability

The compiler should explain:

- why a runtime module was retained
- why a semantic node rebuilt
- why a capability was required
- why an optimization was not applied
- why a target cannot be built
