# DirimEngine

DirimEngine is a small C++ 3D game engine project with a first-person demonstration game built on top of it. The engine is the reusable part of the repository; the game is a concrete example of how its rendering, scene, physics, input, UI, audio, model-loading, and pathfinding systems can be composed into a playable experience.

## Project Parts

### Engine

The reusable engine implementation lives in `engine/include/dirimengine/`. Its systems are represented by the following headers:

- `scene.hpp` and `uiscene.hpp`: world and UI scene management
- `gameobject.hpp`, `objects.hpp`, and `model.hpp`: scene objects, geometry, and model loading
- `shaderProgram.h`, `mesh.hpp`, and `skybox.hpp`: OpenGL rendering support
- `camera.h`, `inputs.hpp`, and `physics.hpp`: camera, input, and collision behavior
- `lightsystem.hpp` and `audiosystem.hpp`: lighting and IrrKlang audio integration
- `levelloader.hpp` and `pathfinding.hpp`: grid level loading and navigation
- `player.hpp`, `enemy.hpp`, and `ui.hpp`: higher-level gameplay-facing engine components

Third-party and generated integration code is kept under `vendor/`. Native IrrKlang libraries are stored under `lib/`.

### Demonstration Game

The demonstration game is implemented by `game/src/main.cpp`. It uses the engine systems to provide:

- a first-person OpenGL scene
- a menu, gameplay state, and death state
- textured geometry and imported 3D models
- enemy navigation and collision handling
- weapon and ammunition interactions
- dynamic lighting, UI text, and audio

The game content is organized under `game/assets/`, with separate directories for audio, fonts, levels, models, shaders, and textures.

## Requirements

- Linux with a working OpenGL 3.3-capable driver
- CMake 3.10 or newer
- A C++ compiler with C support
- GLFW 3.3 or newer
- SDL2
- OpenGL development libraries
- Assimp
- FreeType
- IrrKlang runtime and development files

The project includes GLAD, GLM, IrrKlang headers, and IrrKlang shared libraries in `vendor/` and `lib/` respectively. System packages are still required for GLFW, SDL2, OpenGL, Assimp, and FreeType.

## Build and Run

Configure and build from the repository root:

```sh
cmake -S . -B build
cmake --build build
./build/DirimEngine
```

The existing convenience script performs the same clean configure, build, and run sequence:

```sh
./run.sh
```

The asset root is supplied by CMake through `DIRIMENGINE_ASSET_ROOT`, so the executable no longer depends on the original developer's checkout path.

## Controls

- `W`, `A`, `S`, `D`: move
- Mouse: look around
- Left mouse button: fire
- `R`: restart after death
- `Escape`: quit
- Menu: `Up` / `Down` and `Enter`

## Repository Layout

```text
.
├── CMakeLists.txt       # CMake build definition
├── run.sh               # Clean configure, build, and launch helper
├── engine/              # Reusable engine headers and implementation units
├── game/                # Demonstration executable and game assets
├── vendor/              # Vendored headers and GLAD source
├── lib/                 # Bundled IrrKlang shared libraries
├── docs/                # Architecture and development notes
└── build/               # Local CMake output; generated and ignored
```

## Development Notes

The current executable is intentionally a demonstration target rather than a separately packaged engine library. The architecture documentation in `docs/ARCHITECTURE.md` records the boundary between the reusable engine and the demonstration game.

Before committing changes, build from a clean directory so stale CMake paths cannot hide source or dependency problems:

```sh
rm -rf build
cmake -S . -B build
cmake --build build
```
