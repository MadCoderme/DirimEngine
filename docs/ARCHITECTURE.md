# Architecture

DirimEngine has two logical layers in one current executable.

## Engine Layer

The engine layer supplies reusable runtime services and data types:

```text
Application
    ├── Scene and UIScene
    ├── GameObject, Model, Mesh, ShaderProgram
    ├── Camera, Input, Physics
    ├── Lighting and Audio
    └── Level Loading and Pathfinding
```

The implementation is header-oriented. `stb_image.cpp` and `vendor/src/glad.c` provide the compiled implementation units required by the headers and OpenGL setup.

## Demonstration Layer

`game/src/main.cpp` is the demonstration application. It owns the game-specific composition and state, including:

- menu, gameplay, and death screens
- player health and ammunition
- enemy and pickup setup
- level and model selection
- game-specific audio, lighting, and UI text

The engine headers and demonstration entry point now live in separate directories, while CMake still produces one `DirimEngine` executable. Keeping this boundary explicit makes a future extraction into an engine library and a separate game target straightforward.

## Runtime Data Flow

```text
Input -> Demo game state -> Scene / GameObjects
                         -> Physics / Pathfinding
                         -> Audio / Lighting / UI
                         -> OpenGL rendering
```

Models, textures, shaders, levels, fonts, and audio under `game/assets/` are demonstration content. They are not engine dependencies and can be replaced by another game.
