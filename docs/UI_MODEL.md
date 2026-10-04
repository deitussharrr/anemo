# Anemo semantic UI model

## Intent over widgets

Anemo describes what an application means. A semantic node carries role,
label, state dependencies, actions, and adaptation metadata. Platform
renderers choose the native presentation.

```anm
content {
    heading "Welcome"

    action "Continue" {
        navigate Home
    }
}
```

Explicit layout is an escape hatch:

```anm
layout {
    VStack {
        Text("Welcome")
        Button("Continue") {
            navigate Home
        }
    }
}
```

Semantic mode is preferred because it gives the compiler enough information
for accessibility, testing, input adaptation, and non-visual clients.

## Core roles

The first role set is heading, text, image, action, input, form, list,
navigation, dialog, menu, toolbar, media, and map. Roles are not tied to a
specific rendering toolkit.

## Reactive state

```anm
state count = 0

Text(count)

action "+" {
    count += 1
}
```

State dependencies form a graph. A change invalidates only affected semantic
nodes. Static nodes are emitted once; dynamic nodes are updated; interactive
nodes expose effects and capabilities.

## Adaptation

The runtime selects variants from:

- compact, medium, and expanded form factors
- touch, mouse, keyboard, stylus, controller, and voice input
- reduced motion, contrast, text scale, and screen-reader settings
- power, thermal, memory, network, and graphics conditions
- optional capabilities such as camera, GPS, biometrics, and haptics

Applications request capabilities rather than checking operating systems:

```anm
when camera.available {
    camera.capture()
}
```

Required unavailable capabilities fail clearly during build or startup.
Optional capabilities must define a visible degraded path.

## Accessibility

Semantic actions automatically provide role, label, focus order, keyboard and
controller mappings, and screen-reader metadata. Developers can override
labels and hints, but a renderer must not discard semantic information.

## Rendering contract

The UI compiler lowers semantic nodes to a render graph. Platform adapters
implement the graph using native controls or Anemo graphics backends:

```text
Android -> native Android/Vulkan facilities
iOS/macOS -> native Apple/Metal facilities
Windows -> native Windows/DirectX or Vulkan facilities
Linux -> native Linux/Vulkan facilities
Web -> WebAssembly/WebGPU facilities
```

The graph and semantic tree remain testable without a display, enabling
coordinate-free tests such as “tap action Login” and “assert heading Welcome”.
