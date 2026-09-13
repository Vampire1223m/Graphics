# Linux Graphics From Scratch

A low-level Linux graphics project built from the ground up, starting with
DRM/KMS and progressing to Vulkan rendering directly on the display.

This project began as a way to understand the Linux graphics stack from the
bottom up and has since become the foundation for a larger goal: building a
graphical environment for Arch Linux without starting from an existing
desktop stack.

---

## From DRM/KMS to Vulkan

The project began with **DRM/KMS**, before Vulkan was involved.

### The first implementation used the DRM API to:

- Open a DRM device
- Enumerate display resources
- Find a connected display
- Enumerate display modes
- Select a CRTC
- Create a dumb buffer
- Map the buffer into memory
- Write pixels directly
- Create a framebuffer
- Display it through KMS

### The Vulkan implementation now handles:

- Vulkan initialization
- Physical GPU selection
- Display enumeration through `VK_KHR_display`
- Display mode selection
- Compatible display plane selection
- Vulkan display surface creation
- Graphics/presentation queue selection
- Logical device creation
- Swapchain creation
- Swapchain image view creation
- Frame synchronization
- Vulkan dynamic rendering
- SPIR-V shader loading
- Graphics pipeline creation
- Direct triangle rendering

The program runs from a Linux TTY and does not require an existing GUI.

---

## Architecture

```text
include/
├── core.h
├── display.h
├── params.h
├── pipeline.h
└── renderer.h

src/
├── drm/
│   └── dumbdrm.c
└── vulkan/
    ├── core.c
    ├── display.c
    ├── main.c
    ├── pipeline.c
    └── renderer.c

shaders/
├── triangle.vert
└── triangle.frag
```

The project intentionally works below conventional Linux window systems.

```text
Application
    |
    v
  Vulkan
    |
    v
VK_KHR_display
    |
    v
 DRM / KMS
    |
    v
   GPU
    |
    v
 Display
```

---

## Prerequisites

To build this project, you will need:

- **C Compiler** — GCC or Clang
- **CMake** — 3.15+
- **Vulkan Headers**
- **Vulkan Loader**
- **GLSL Compiler** — `glslc`
- A Vulkan-capable GPU
- Linux with DRM/KMS support

---

## Building and Running

This project uses an out-of-source CMake build to keep the root directory clean. 

```bash
# 1. Clone the repository
git clone https://github.com/Vampire1223m/Graphics.git
cd Graphics

# 2. Create a build directory
mkdir build
cd build

# 3. Generate Makefiles and compile
cmake ..
make

# 4. Run
cd ../bin
./VulkanEngine
```

---

## Current Status

The current implementation successfully renders a Vulkan triangle directly
to a `1920x1200 @ 165Hz` display from a Linux TTY.
![My Project Screenshot](pics/triangle.png)
---

## Motivation

This project started for three fairly simple reasons:

**I wanted to learn.  
I was bored.  
And I wanted to find out how far I could take it.**

Rather than starting with an existing window system or graphics framework, I
decided to work upward from the lowest practical layer of the Linux graphics
stack.

The result is a deliberately low-level exploration of how a program goes from
raw display control to GPU rendering.

The eventual objective is to use this foundation to build my own GUI and
compositor for Arch Linux.
