# Anemo runtime

## Modular design

The runtime is a collection of linkable modules, not one mandatory monolith:

```text
core
memory
async
ui
graphics
networking
filesystem
camera
bluetooth
location
notifications
platform
```

The compiler derives required modules from MIR effects and capability
requirements. Unused modules are not linked into release artifacts.

## Platform adapter contract

Every adapter implements the same stable contracts for:

- event scheduling and cancellation
- application lifecycle
- windows, surfaces, and input
- filesystem and secure storage
- networking
- permissions and capability state
- graphics surfaces
- accessibility announcements and focus

Adapters may use native APIs internally:

| Platform | Typical native facilities |
| --- | --- |
| Android | Android SDK, NDK, Vulkan, CameraX |
| iOS/macOS | Apple SDKs, Metal, AVFoundation |
| Windows | Win32/WinUI, DirectX, Media Foundation |
| Linux | system libraries, Vulkan, desktop portals |
| Web | WebAssembly, Web APIs, WebGPU |

The application sees the Anemo contract, not the adapter's implementation.

## Lifecycle and errors

Runtime operations return typed errors. A missing optional capability must
produce an explicit unavailable state and a developer-defined degraded path.
A missing required capability is a build or startup error with the capability,
target, and remediation.

Lifecycle events include startup, foreground, background, suspend, resume, and
shutdown. Background work must be cancellable and must not assume that a
process remains alive indefinitely on mobile platforms.

## Runtime size

Release builds perform dependency closure and dead-module elimination.
Runtime-size reports must list retained modules and explain why each one is
required.
