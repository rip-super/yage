# YAGE - Yet Another Game Engine

A modular 2D game engine written in C++ with OpenGL. Made to give you the things you need without forcing you to use things you don't.

## Features

- Modular architecture: only compile and link what you actually use
- 2D rendering with batched draw calls
- Window and input management via GLFW
- Audio playback via miniaudio
- Camera with smooth follow and zoom
- Custom shaders and textures
- Scene management
- Binary and JSON serialization
- TCP and Unix domain socket networking with peer-to-peer and server-client support

## Requirements

- CMake 3.20+
- C++17 compiler (Clang or GCC)
- OpenGL 4.1+

*NOTE: Clang/GCC should work out of the box, however, in my testing, MSVC required a bit of extra tinkering to get everything running, which is why I did not suggest using it above*

## Building
```bash
git clone https://github.com/rip-super/yage
cd yage
cmake -B build
cmake --build build
```

Binaries are written to `build/bin/`.

## Module System

YAGE is split into separate CMake targets so users only compile and link what
they need. A convenience `yage` target is provided that links everything.

| Target         | What it includes                                        | Extra deps      |
|----------------|---------------------------------------------------------|-----------------|
| `yage_core`    | Window, input, renderer, shader                         | GLFW, GLAD, GLM |
| `yage_extras`  | Camera, scene manager, texture, serializer              | nlohmann/json   |
| `yage_audio`   | Audio playback                                          | miniaudio       |
| `yage_network` | TCP/IPC transport, client, server, peer, message router | asio, threads   |
| `yage`         | Everything above                                        | -               |

In your own `CMakeLists.txt`:
```cmake
# Link everything
target_link_libraries(my_game PRIVATE yage)

# Or only what you need
target_link_libraries(my_game PRIVATE yage_core)
target_link_libraries(my_game PRIVATE yage_extras)
target_link_libraries(my_game PRIVATE yage_audio)
target_link_libraries(my_game PRIVATE yage_network)
```

## Modules

### Core (`yage_core`)

- `Window` - window creation and management, OpenGL context, delta time
- `Input` - keyboard and mouse input, scroll delta
- `Renderer` - batched 2D shape drawing, rotation, textures, custom shaders
- `Shader` - load and compile GLSL shaders

Include: `#include <yage/yage.h>`

### Extras (`yage_extras`)

- `Camera` - smooth follow, zoom, view-projection matrix
- `SceneManager` - scene switching and lifecycle management
- `Texture` - load images from disk via stb_image
- `Serializer` - binary and JSON serialization with a traits-based type system

Includes:
```cpp
#include <yage/extras/camera.h>
#include <yage/extras/scene_manager.h>
#include <yage/extras/texture.h>
#include <yage/extras/serializer.h>
```

### Audio (`yage_audio`)

- Audio playback and mixing via miniaudio

Include: `#include <yage/extras/audio.h>`

### Networking (`yage_network`)

- `TcpTransport` - TCP transport, works over a real network or localhost
- `IpcTransport` - Unix domain socket transport for same-machine communication (macOS/Linux)
- `Client` - connects to a server
- `Server` - listens for and manages multiple client connections
- `Peer` - can both listen and connect simultaneously for peer-to-peer topologies
- `MessageRouter` - typed message dispatch on top of the raw byte transport

Include: `#include <yage/extras/network.h>`

## Examples

All examples are in the `examples/` directory and are built automatically with
the project. Each one is a focused, standalone program demonstrating a single
feature. Reading through them in order is the fastest way to learn the engine.

| Example          | What it shows                                                        |
|------------------|----------------------------------------------------------------------|
| `hello_window`   | Opening a window and the basic game loop                             |
| `simple_shapes`  | Drawing shapes and using colors                                      |
| `user_input`     | Keyboard and mouse input handling                                    |
| `custom_shader`  | Loading and using a custom GLSL shader                               |
| `custom_texture` | Loading and drawing textures from disk                               |
| `camera`         | Smooth camera follow, zoom, and viewport-aware grid rendering        |
| `scene_manager`  | Switching between scenes using lifecycle callbacks                   |
| `audio`          | Playing music and sound effects                                      |
| `serializer`     | Saving and loading game state, persistent high score                 |
| `networking`     | Server-client and peer-to-peer networking with live position sync    |

Run an example:
```bash
./build/bin/hello_window
./build/bin/simple_shapes
./build/bin/user_input
./build/bin/custom_texture
./build/bin/camera
./build/bin/networking
```

## Directory Structure
```
yage/
  deps/               # third-party dependencies
    asio/
    earcut/
    glad/
    glfw/
    glm/
    miniaudio/
    nlohmann/
    stb_image/
  examples/           # example programs
  yage/
    include/yage/     # public headers
      extras/
    src/              # implementation
      extras/
    shaders/          # built-in GLSL shaders
```

## Dependencies

All dependencies are vendored in `deps/` and require no separate installation.

| Library       | License           | Purpose               |
|---------------|-------------------|-----------------------|
| GLFW          | zlib              | Window and input      |
| GLAD          | MIT               | OpenGL loader         |
| GLM           | MIT               | Math                  |
| miniaudio     | MIT/Unlicense     | Audio                 |
| nlohmann/json | MIT               | JSON serialization    |
| asio          | BSL-1.0           | Networking            |
| stb_image     | MIT/Public Domain | Image loading         |
| earcut        | ISC               | Polygon triangulation |