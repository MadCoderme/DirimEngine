# Development

## Clean Build

Use an out-of-source CMake build:

```sh
cmake -S . -B build
cmake --build build
```

For a fully fresh configuration:

```sh
rm -rf build
cmake -S . -B build
cmake --build build
```

## Build Dependencies

CMake locates GLFW, SDL2, OpenGL, Assimp, and FreeType from the host system. IrrKlang is linked from the repository's `lib/` directory, with its headers in `vendor/include/irrklang`.

## Asset Paths

Game resources live under `game/assets/`. CMake defines `DIRIMENGINE_ASSET_ROOT` for the executable, and both the engine's built-in rendering resources and the demonstration's models, levels, textures, and audio resolve from that root.

## Scope of Changes

Keep reusable engine behavior separate from demonstration-specific behavior when extending the project. New engine facilities belong with the engine headers; game rules, content selection, and presentation belong in the demonstration application. Avoid committing generated CMake output, local caches, or compiled binaries.
