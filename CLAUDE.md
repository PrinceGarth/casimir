# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## AI contribution policy

This repository is Casimir, an unofficial, AI-assisted ("vibe-coded") fork of upstream Pioneer. Upstream Pioneer does not accept AI-generated contributions; this fork's own policy (see README "Contributing") allows AI-assisted work with disclosure.

- AI-authored changes, commits, and comments are fine here, in this fork.
- Never produce patches, PRs, or issue text intended for upstream `pioneerspacesim/pioneer`. Upstream's ban stands, and this fork respects it.
- Never help get around upstream's AI policy, or argue with, pressure or harass Pioneer's developers or community. Casimir exists because of their human-written work; speak about them with respect.
- When fixing an upstream issue in Casimir, refer to it in plain text ("upstream Pioneer issue 5868") in commits, PRs and issues. Never write `pioneerspacesim/pioneer#5868` or a link to it: GitHub posts a "mentioned this" backlink on upstream's issue. Never comment on upstream issues to say Casimir fixed them.
- Don't copy in code from upstream PRs that were closed for being AI-generated or for licensing reasons.
- Don't knowingly reproduce code from incompatibly licensed sources. Mention it if generated code looks lifted from somewhere.
- Build, test (`./build/unittest`), run `./autoformat` and `scripts/translations.py check` before calling a change done.
- Never upload strings or AI translations to Pioneer's Transifex (see Translations below).
- Keep licence files, `AUTHORS.txt`, and about-window credits intact (see Licensing below).

## Build

```
./bootstrap                 # generates build/ via CMake (pass extra CMake args through)
make -C build -j$(nproc)    # builds the `casimir` executable (build/casimir)
./casimir                   # run the game (wrapper: rebuilds if needed, then execs build/casimir)
```

User config and saves: `~/.casimir/` on Linux (`src/posix/FileSystemPosix.cpp`), deliberately separate from upstream Pioneer's `~/.pioneer`.

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
- `data/` — game content: Lua scripts (`data/modules` etc.), models, textures, ship defs, systems, factions, UI (pigui) Lua, translation files (`data/lang/<resource>/<lang>.json`, see Translations). Content here is largely data/Lua, not C++; changes to gameplay/balance/missions usually live here rather than in `src/`.
- `contrib/` — third-party/vendored code bundled with the build.
- `cmake/` — CMake helper modules used by the root `CMakeLists.txt`.
- `scripts/` — developer/CI utility scripts (formatting, release packaging, translation tooling, enum scanning, etc.), not part of the runtime.

## Translations

No translation portal. Upstream Pioneer's human translations arrive by merging `upstream/master`. Casimir's own strings are AI-translated, and fixes start from a screenshot (issue form `.github/ISSUE_TEMPLATE/translation.yml`). User-facing summary: README "Translations".

- Files: `data/lang/<resource>/<lang>.json`, each key `{"description": ..., "message": ...}`, sorted with 2-space indent (`scripts/translations.py` writes this format).
- Casimir-owned strings: any key in a `data/lang/casimir-*/` folder (Lua: `Lang.GetResource("casimir-ui")`), or a `CASIMIR_`-prefixed key in an upstream folder. C++ strings have to be in `core/` (declared in `src/LangStrings.inc.h`), so they use the prefix.
- Fallback: a missing language *file* falls back to `en.json` for the whole resource (`src/Lang.cpp` GetResource). A missing *key* in an existing file does not: C++ shows the raw key, and Lua gets `nil`, which can crash a pigui module. So a key added to en.json must also be added to every existing language file: `scripts/translations.py fill` does this, in English.
- `scripts/translations.py check` (in CI) fails on missing keys, unbalanced `{ }`, placeholders written as `(name)` or `[name]`, any translation whose placeholders differ from English, and a Casimir string with no `description`. Placeholders: Lua `{name}` everywhere; in `core/` also C++ `%name`, `%0` or `%{name}` with optional `{formatspec}`; elsewhere `string.format` `%s`/`%i`/`%.2f`, which must keep English's order. Keep names untranslated and unchanged: `{planeta}` or `(client)` prints literally or as `nil`.
- `scripts/translation-exceptions.json`: reviewed upstream translations that differ on purpose (a fixed name written out to inflect it, or an extra variable the code really provides; check the Lua `interp` table). Each entry stores the exact message, so it lapses when the text changes. Casimir-owned strings can't use it.
- Casimir fixed ~300 broken placeholders in upstream's translations (Oct 2026). Those fixes are Casimir's; never send them upstream.
- `scripts/translations.py todo` lists Casimir strings still in English, per language, with descriptions: the input for translating.
- After merging `upstream/master`: run `scripts/translations.py fill` then `check` (upstream's en.json can gain keys before their bot fills the other languages, and new upstream translations can bring new placeholder breakage; fix it the same way). Conflicts in `data/lang/*/<lang>.json` usually mean upstream changed a string Casimir fixed: take upstream's version, then re-run `check`. If the merge conflicts on `.tx/config` or `scripts/update-translations.sh`, keep them deleted (`rip` them, then `git add -A` those paths).
- Don't reword an existing upstream English string; add a new key. Don't AI-translate upstream strings unless asked. Never `tx push` or upload anything to Pioneer's Transifex.

Adding a string:
1. Add it to `data/lang/casimir-<area>/en.json` with a `description` saying where it appears in the game (screen, button, situation).
2. Translate it into every language that `data/lang/core/` has, creating the folder's `<lang>.json` files as needed. Keep placeholders exactly.
3. Run `scripts/translations.py fill` then `check`.

Fixing a translation from a screenshot:
1. Grep the text visible in the screenshot in `data/lang/*/<lang>.json` to find the key (or grep the English in `en.json` if the text is untranslated).
2. Confirm the code actually uses that key (`grep -rn KEY data src`) and that it's the one on screen.
3. Edit only that language's file, then run `scripts/translations.py check`.
4. If the key is upstream's (not Casimir-owned), the fix is fine, but note in the PR that a later upstream translation update to the same key will conflict on merge.

## Licensing (for forks / redistribution)

Forking and making AI-assisted changes in a separate fork is legal; the no-AI rule is an upstream contribution policy, not a licence term. Not legal advice.

- Code and Lua modules: GPLv3 (`licenses/GPL-3.txt`). If distributing a fork: keep it GPLv3, keep copyright notices, offer source, mark changes as modified, don't imply upstream endorsement.
- Assets in `data/` (art, music, models, textures, sounds) are **CC-BY-SA-3.0** by default (`AUTHORS.txt`, `data/pigui/modules/about-window.lua`), not GPL. Keep attribution; ship modified assets under CC-BY-SA-3.0 or compatible.
- Exceptions: fonts are SIL OFL 1.1 (Pionillium, Inpionata, Orbiteer), DejaVu licence, and Apache-2.0 (WenQuanYi Micro Hei). `data/icons/icons.svg` is believed to be the CC-BY-SA-3.0 WPZOOM icon set (David Ferreira; unconfirmed, the SVG has no licence metadata). `data/textures/skybox/skybox.dds` is the NASA public-domain Milky Way panorama (credit required; likely mapping).
- `data/galaxy_colour.png` is the NASA Spitzer image (NASA/JPL-Caltech/R. Hurt; policy in `licenses/`). Git history shows it replaced an earlier Baobobafet image in 2014 (`0444e824c`, credit added in `d33782e68`). Nothing in the code references it since GalacticView was removed (2020), so it can be deleted safely if the NASA policy is unwanted.
- AI-generated code in a fork goes under GPLv3; AI-generated art may not be copyrightable (jurisdiction-dependent). Keep `AUTHORS.txt`, `licenses/`, and the about-window credits intact when publishing.
