# Zeo Engine

![Logo](https://raw.githubusercontent.com/wiki/hls333555/ZeoEngine/Logo.png)

Zeo Engine is targeted to be a fully functional real-time cross-platform game engine for developing 2D and 3D games. Currently, it is only in early-access and implemented for 2D only.

## Features

**Editor Preview:**

![EditorPreview](https://raw.githubusercontent.com/wiki/hls333555/ZeoEngine/EditorPreview.png)

**Demo Game:**

![GamePreview](https://raw.githubusercontent.com/wiki/hls333555/ZeoEngine/GamePreview.gif)

Currently implemented features:

* Fully functional scene editor UI including a generic data inspector
* Reflection system
* Serialization and deserialization (.zscene, .zparticle)
* Sprite batch render
* *Basic collision detection* (WIP)
* Particle system
* Basic profiler

More features will come in the future.

## Getting Started

Visual Studio 2019 is recommended, ZeoEngine is officially untested on other development environments whilst we focus on a Windows build.

Start by cloning the repository with `git clone --recursive https://github.com/hls333555/ZeoEngine`.

If the repository was cloned non-recursively previously, use `git submodule update --init` to clone the necessary submodules.

**NOTE**

The engine uses FreeType as default font rasterizer. 

In order to work properly, you should get latest FreeType binaries or build yourself using *vcpkg* with `vcpkg install freetype:x64-windows`, `vcpkg integrate install`. More instructions on [Install vcpkg on Windows, Linux, and macOS | Microsoft Docs](https://docs.microsoft.com/en-us/cpp/build/install-vcpkg?view=msvc-160&tabs=windows).



You can refer to the [wiki page](https://github.com/hls333555/ZeoEngine/wiki) to see how this engine operates internally and how you can use it to create games.

