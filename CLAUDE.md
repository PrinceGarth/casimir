# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## AI contribution policy

This repository is Casimir, an unofficial, AI-assisted ("vibe-coded") fork of upstream Pioneer. Upstream Pioneer does not accept AI-generated contributions; this fork's own policy (see README "Contributing") allows AI-assisted work with disclosure.

- AI-authored changes, commits, and comments are fine here, in this fork.
- Never produce patches, PRs, or issue text intended for upstream `pioneerspacesim/pioneer`. Upstream's ban stands, and this fork respects it.
- Don't copy in code from upstream PRs that were closed for being AI-generated or for licensing reasons.
- Don't knowingly reproduce code from incompatibly licensed sources. Mention it if generated code looks lifted from somewhere.
- Build, test (`./build/unittest`) and run `./autoformat` before calling a change done.
- Keep licence files, `AUTHORS.txt`, and about-window credits intact (see Licensing below).

## Build

```
./bootstrap                 # generates build/ via CMake (pass extra CMake args through)
make -C build -j$(nproc)    # builds the `pioneer` executable
./pioneer                   # run the game (portable mode: run from repo root, data/ alongside)
```

VSCode: copy `pioneer-default.code-workspace` to `pioneer.code-workspace`, use CMake Tools with a configure preset from `CMakePresets.json` (e.g. `x64-Linux-Debug`).

Linux deps (Debian/Ubuntu): g++, cmake, mesa-common-dev, libfreeimage-dev, libglew-dev, libfreetype6-dev, libsigc++-2.0-dev, libvorbis-dev, libassimp-dev (>=5.0.1), libsdl2-dev, libsdl2-image-dev, libopenal-dev (optional).

Useful CMake options (`-DFOO=1` passed to `./bootstrap`): `PROFILER_ENABLED`, `USE_AVX2`, `USE_TIME_TRACE` (Clang), `USE_ASAN` (Clang), `REMOTE_LUA_REPL`, `WITH_OBJECTVIEWER`, `WITH_DEVKEYS`.

Precompile models for faster startup: `make -C build build-data`.

Full details, including Windows/macOS toolchains and pioneer-thirdparty, are in `COMPILING.txt`.

## Tests

Tests use doctest and live in `src/test/` (`SimulationTests.cpp`, `TestFixed.cpp`, `TestLuaPushPull.cpp`, `TestProperty.cpp`, `TestStringHash.cpp`, `TestStringName.cpp`, `TestTaskGraph.cpp`), built as the `unittest` target.

```
make -C build unittest -j$(nproc)
./build/unittest                      # run all tests
./build/unittest --test-case="Name"   # run a single doctest test case
```

## Formatting

C++ style is enforced by `.clang-format` (LLVM-based, see file for the deltas). Run `./autoformat` before committing — it invokes `scripts/clang-format.sh` and offers to apply a patch for staged changes. `scripts/clang-format.sh` can also be run directly for CI-style checking.

## Architecture

Pioneer is a C++ space sim with a Lua scripting layer for game content (missions, BBS/bulletin-board interactions, economy, some UI glue). Top-level layout:

- `src/` — engine and game C++ source (flat, historically organized; see subfolders below for the structured areas).
  - `src/core/` — low-level utility/container code shared across the engine.
  - `src/graphics/` — the renderer abstraction (backend-agnostic interfaces; look here before touching anything that draws).
  - `src/pigui/` — Dear ImGui-based UI layer used for in-game screens (station UI, map, HUD elements).
  - `src/scenegraph/` — model scene graph used for loading/rendering ship and station models (`.sgm`/model source).
  - `src/terrain/`, `GeoSphere*`/`GeoPatch*`/`BaseSphere*` at the src root — procedural planet terrain generation and geometry patching.
  - `src/galaxy/` — star system/galaxy generation (factions, sector data, star system procedural generation).
  - `src/ship/` — ship-specific subsystems (as distinct from the top-level `Ship.cpp`/`ShipType.cpp`/`ShipAICmd.cpp` which hold the main ship model/AI).
  - `src/lua/` — the Lua binding layer (`Lua*.cpp/h`) that exposes engine objects (bodies, ships, factions, economy, game state, input, etc.) to Lua; this is the seam between C++ engine and Lua game content in `data/`.
  - `src/collider/` — collision detection/geometry.
  - `src/sound/`, `src/text/` — audio backend (including a null backend for headless/unit-test runs) and text rendering/font handling.
  - `src/editor/` — the standalone model/scene editor (separate `editormain.cpp` entry point and its own `CMakeLists.txt`), not part of the main game binary.
  - `src/win32/`, `src/posix/` — platform-specific shims.
  - `src/test/` — doctest unit/simulation tests (see Tests above).
  - Central classes at the `src/` root worth knowing up front: `Pi.cpp`/`Pi.h` (global engine/game singleton and main loop wiring), `Game.cpp` (game session state), `Space.cpp` (the simulated space containing bodies), `Body.cpp` and its subclasses (`Ship`, `Player`, `SpaceStation`, `Planet`, `Star`, `CargoBody`, `Missile`, `Projectile`, `HyperspaceCloud`, ...), `Frame.cpp` (reference frame tree bodies live in), `SectorView`/`SectorMap`/`SystemView` (map UI + underlying galaxy sector data views).
- `data/` — game content: Lua scripts (`data/modules` etc.), models, textures, ship defs, systems, factions, UI (pigui) Lua, localization strings pulled from Transifex. Content here is largely data/Lua, not C++; changes to gameplay/balance/missions usually live here rather than in `src/`.
- `contrib/` — third-party/vendored code bundled with the build.
- `cmake/` — CMake helper modules used by the root `CMakeLists.txt`.
- `scripts/` — developer/CI utility scripts (formatting, release packaging, translation tooling, enum scanning, etc.), not part of the runtime.

Localization strings are pulled automatically from Transifex — don't hand-edit translation files or open PRs for translations.

## Licensing (for forks / redistribution)

Forking and making AI-assisted changes in a separate fork is legal; the no-AI rule is an upstream contribution policy, not a licence term. Not legal advice.

- Code and Lua modules: GPLv3 (`licenses/GPL-3.txt`). If distributing a fork: keep it GPLv3, keep copyright notices, offer source, mark changes as modified, don't imply upstream endorsement.
- Assets in `data/` (art, music, models, textures, sounds) are **CC-BY-SA-3.0** by default (`AUTHORS.txt`, `data/pigui/modules/about-window.lua`), not GPL. Keep attribution; ship modified assets under CC-BY-SA-3.0 or compatible.
- Exceptions: fonts are SIL OFL 1.1 (Pionillium, Inpionata, Orbiteer), DejaVu licence, and Apache-2.0 (WenQuanYi Micro Hei). `data/icons/icons.svg` is believed to be the CC-BY-SA-3.0 WPZOOM icon set (David Ferreira; unconfirmed, the SVG has no licence metadata). `data/textures/skybox/skybox.dds` is the NASA public-domain Milky Way panorama (credit required; likely mapping).
- `data/galaxy_colour.png` is the NASA Spitzer image (NASA/JPL-Caltech/R. Hurt; policy in `licenses/`). Git history shows it replaced an earlier Baobobafet image in 2014 (`0444e824c`, credit added in `d33782e68`). Nothing in the code references it since GalacticView was removed (2020), so it can be deleted safely if the NASA policy is unwanted.
- AI-generated code in a fork goes under GPLv3; AI-generated art may not be copyrightable (jurisdiction-dependent). Keep `AUTHORS.txt`, `licenses/`, and the about-window credits intact when publishing.
