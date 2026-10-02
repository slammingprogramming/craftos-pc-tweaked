<!--
SPDX-FileCopyrightText: 2026 slammingprogramming
SPDX-License-Identifier: CC-BY-SA-4.0
-->
# AGENTS.md: everything an agent needs to work on CraftOS-Tweaked

This file is for AI coding agents (and for people who want the short version). It is public, like the rest of the
repository: **never put secrets, tokens, private addresses, personal data, or links to private sessions or chats in it
or anywhere else in the repository.** Keep it current: whenever you change how the project is built, tested, laid
out or released, update the matching section in the same commit.

## 1. What this project is

CraftOS-Tweaked is an emulator of [CC: Tweaked](https://github.com/cc-tweaked/CC-Tweaked) (the Minecraft computer mod),
built to behave **exactly like CC: Tweaked**. It is a hard fork of [CraftOS-PC 2](https://github.com/MCJack123/craftos2)
by JackMacWindows (MIT), relicensed to AGPL-3.0-or-later. Maintainer: GitHub user `slammingprogramming`.

- The C++ side (SDL2, Poco, a patched PUC Lua 5.2 from the `craftos2-lua` submodule) implements the Java-side APIs
  (`fs`, `http`, `os`, `term`, `peripheral`, `redstone`, ...).
- The Lua side is **CC: Tweaked's own `bios.lua` and `rom/`, byte for byte**, one copy per supported CC: Tweaked
  version in `roms/<id>/`.
- The goal is measured by CC: Tweaked's own test suite (McFly) plus our own specs. Do not call something "exactly
  like CC: Tweaked" without a test or a reading of the Java source behind it.

Supported versions (folder names in `roms/`): `mc-1.20.x` (default, CC: Tweaked 1.120.2, Minecraft 1.20.1) and
`mc-26.3` (1.120.3, Minecraft 26.3). More branches can be added (see `docs/cc-tweaked-compatibility.md`).

## 2. Ground rules (decisions the maintainer already made)

- **Licensing**: code is AGPL-3.0-or-later; documentation and non-software content is CC-BY-SA-4.0; the original MIT
  notice and the sound attributions in `LICENSE` must be kept. New files get an SPDX header
  (`SPDX-License-Identifier: AGPL-3.0-or-later` for code, `CC-BY-SA-4.0` for docs) and
  `Copyright (c) 2026 slammingprogramming`. Keep the upstream copyright line in files that came from CraftOS-PC 2.
- **ROMs keep CC: Tweaked's licenses** (mostly the ComputerCraft Public License and MPL-2.0, per file). They are not
  AGPL. Never edit `roms/<id>/bios.lua` or anything under `roms/<id>/rom/` that comes from upstream; behavior that
  the stock ROM lacks goes into `roms-overlay/` (additive only). `tools/rom/compose.py` builds the folders.
- **No code from upstream CraftOS-PC pull requests.** Credit original author and contributors (`AUTHORS.md`).
- **All builds happen in GitHub Actions**, not on developer machines. Local builds are for verification only.
- **Releases are made by pushing a version tag** (`vMAJOR.MINOR.PATCH`); see `TODO.md` section 1.
- **Nothing is uploaded automatically**: crash logs stay on the user's computer (see section 8).
- **Names**: the project is "CraftOS-Tweaked", the binary is `craftos-tweaked`, the Windows files are
  `CraftOS-Tweaked*`. The plugin API keeps its old names for compatibility: `api/CraftOS-PC.hpp`, the `CRAFTOSPC_*`
  macros, `craftos_pc_version`, and the `ccpcTerm` protocol name. Do not rename those.
- **The plugin ABI**: `struct configuration` in `api/configuration.hpp` is shared with plugins. Only ever add fields at
  the end; never reorder or remove.
- **Mobile**: iOS is sideload-only (no App Store), Android is for F-Droid/Obtainium. The build pipelines for both are
  not written yet and are deliberately postponed. Do not start them unless asked.
- **Commits**: author them as the repository owner (use the repo's configured git identity), and end the message with a
  `Co-Authored-By:` trailer naming the AI that helped. Push only to the branch you were asked to use. Do not open pull
  requests unless asked.
- **Placeholders** that need the maintainer (homepage URL, docs URL, screenshots, icons, signing keys) are listed in
  `TODO.md`. Do not invent values; leave the placeholder and keep the list accurate.

## 3. Repository map

| Path | What is there |
|---|---|
| `src/` | The emulator. `apis/` (Lua APIs), `peripheral/`, `terminal/`, `platform/` (per-OS code), `main.cpp`, `Computer.cpp`, `util.cpp`, `configuration.cpp`, `location.cpp` (data folder choice), `termsupport.cpp` (key codes) |
| `api/` | The public plugin API headers (stable ABI) |
| `roms/<id>/` | One CC: Tweaked version each: `bios.lua`, `rom/`, `rom-info.json`, `LICENSES/`, plus overlay files. `roms/README.md` explains the licensing |
| `roms-overlay/` | What the emulator adds to every ROM: `prelude.lua` (runs before `bios.lua`), `rom/autorun/00_craftos_tweaked.lua`, the emulator's own programs, `debug/`, `hdfont.bmp`, `versions.json` (which CC: Tweaked branches are imported) |
| `patches/craftos2-lua/` | Patches for the Lua submodule (see section 6) |
| `tests/specs/` | Our own McFly specs (redstone, peripherals, ...) |
| `tools/cct/` | Test runner (`run-specs.sh`), comparison (`compare.py`), accepted results (`baseline/<id>.txt`) |
| `tools/rom/compose.py` | Builds `roms/<id>/` from a CC: Tweaked checkout plus the overlay |
| `tools/apply-lua-patches.sh` | Applies `patches/craftos2-lua/*.patch` to the submodule |
| `resources/` | `CCT-Test-Bootstrap.lua` (test harness), `CraftOSTest.lua` (older emulator self-test), `set-version.sh`, packaging files, the Android project |
| `docs/cc-tweaked-compatibility.md` | What matches CC: Tweaked, what does not, how to test, how to add a version |
| `.github/workflows/` | `main.yml` (CI, also called by release), `release.yml`, `sync-roms.yml`, `regenerate-configure.yml` |
| `.github/ISSUE_TEMPLATE/` | Issue forms (bug, crash, compatibility, feature) |
| `TODO.md` | The maintainer's to-do list and process notes; keep it accurate |
| `README.md`, `SECURITY.md`, `AUTHORS.md`, `LICENSE` | User docs, vulnerability reporting (private contact, see that file), credits, license texts |
| `configure.ac`, `configure`, `Makefile.in` | Autotools build (Linux/macOS). `configure` is generated: see section 5 |
| `CraftOS-Tweaked.sln/.vcxproj(.filters)` | Windows build. **New `.cpp`/`.hpp` files must be added here and to `Makefile.in`** |

## 4. Building

Dependencies (Debian/Ubuntu): `libsdl2-dev libsdl2-mixer-dev libhpdf-dev libpng++-dev libwebp-dev libpoco-dev
libncurses5-dev autoconf`. Windows uses vcpkg (`vcpkg.json`).

```sh
git submodule update --init --recursive
tools/apply-lua-patches.sh            # required: the patched Lua is part of CC: Tweaked's behavior
make -C craftos2-lua linux -j"$(nproc)"
./configure && make -j"$(nproc)"      # produces ./craftos-tweaked
```

Run it with the ROMs next to the binary (the repo's `roms/` folder works from the repo root) or set
`CRAFTOS_TWEAKED_ROMS=<path to roms>`. The binary needs to be started from a directory where
`craftos2-lua/src/liblua.so` exists (it links the Lua library by relative path).

Rules that bite:
- Adding a source file: update `Makefile.in` (`_OBJ` list; patterns are `apis_*.o`, `peripheral_*.o`), the `.vcxproj`
  and `.filters`. Changing `configure.ac` requires regenerating `configure` (autoconf 2.71); CI checks it.
- Changing a struct in the Lua sources (`lobject.h`, ...) needs a full Lua rebuild (`make -C craftos2-lua clean`).
- Windows is the most likely place for a surprise (MSVC, the Poco that vcpkg builds, wide-char paths). Remember that
  Linux compiling is not proof.

## 5. Testing

Two kinds of tests, both run by CI (job "CC Tweaked tests") for **every** ROM in `roms/`:

1. **CC: Tweaked's own McFly suite** (`projects/core/src/test/resources/test-rom` in the CC: Tweaked repository, branch
   matching the ROM). CI checks out the exact upstream commit recorded in `roms/<id>/rom-info.json`.
2. **Our specs** in `tests/specs/*_spec.lua`, run as `emu/<name>` by the same runner. The monitor specs need a window and
   run under `xvfb-run` when it is installed (pending otherwise).

```sh
git clone --depth 1 --branch mc-1.20.x https://github.com/cc-tweaked/CC-Tweaked ../CC-Tweaked
tools/cct/run-specs.sh ./craftos-tweaked mc-1.20.x ../CC-Tweaked report.txt   # one emulator process per spec file
python3 tools/cct/compare.py tools/cct/baseline/mc-1.20.x.txt report.txt       # fails if anything got worse
```

- Current state (check `tools/cct/baseline/*.txt`): every spec file passes on both versions. Keep it that way; if
  results change on purpose, copy the new report over the baseline in the same commit.
- A single spec: `craftos-tweaked --headless --cc-version mc-1.20.x --mount-ro test-rom=<CC-Tweaked>/projects/core/src/test/resources/test-rom --script resources/CCT-Test-Bootstrap.lua --args /test-rom/spec/apis/fs_spec.lua`
  (for ours add `--mount-ro emu-spec=tests/specs` and use `--args "/emu-spec/redstone_spec.lua /emu-spec"`).
- `resources/CCT-Test-Bootstrap.lua` sets up CC: Tweaked's test computer: a modem on top, a scripted peripheral hub
  below (`periphemu.create(side, "scripted", {...})` stands in for CC: Tweaked's fake hub), and a `startup.lua`.
- `./craftos-tweaked --headless --script resources/CraftOSTest.lua` is the older self-test; its HTTP check needs
  internet access.
- When you find a behavior difference: read the Java source in CC: Tweaked (the exact branch), write or extend a spec
  that fails, fix the emulator, rerun both versions.
- Timeouts in tests: runaway code is stopped by a timer thread; do not "fix" a hanging spec by raising limits, find
  out why the abort did not work.

## 6. The Lua submodule and its patches

`craftos2-lua` is MCJack123's modified Lua 5.2 (MIT). We do not commit into it; our changes are patches in
`patches/craftos2-lua/` applied by `tools/apply-lua-patches.sh` (idempotent; CI runs it before every Lua build,
including Windows). Current patches: interruptible string matching, hash keys iterate in insertion order, argument
errors worded like CC: Tweaked, and `lua_halt`/`lua_externalerror` that do not take the state lock. To change Lua: edit
inside a clone of the submodule at the pinned commit, regenerate the patch with `git diff`, update
`patches/craftos2-lua/README.md`, and verify with a clean apply on a fresh checkout.

## 7. How the pieces fit (things that are easy to get wrong)

- **ROM selection**: `roms/<id>` is chosen by `--cc-version`, else the `ccVersion` config setting, else `mc-1.20.x`.
  `rom-info.json` (`computercraft_version`, `minecraft_version`, upstream branch/commit) drives `_HOST`, the HTTP
  user agent, and whether the ROM counts as "CC: Tweaked" (`ROMVersion::ccTweaked`). Search order: `CRAFTOS_TWEAKED_ROMS`,
  `roms` next to the executable, the data folder, system folders.
- **Boot**: C++ runs `prelude.lua` (Lua 5.1 shims and a saved `os.shutdown`), then CC: Tweaked's `bios.lua`, whose
  startup runs everything in `rom/autorun/`, including our `00_craftos_tweaked.lua` (script/exec hooks, shell
  completions).
- **standardsMode** (CraftOS-PC's "behave like CC: Tweaked" switch): on by default when the active ROM is a CC: Tweaked
  ROM, off for other ROMs; an explicit user setting always wins (`standardsModeExplicit`). It controls time-outs,
  space limits, rounding and keeps a window open after `shutdown` (not in headless/CLI runs).
- **Key codes**: terminals produce LWJGL2 codes internally; `convertKeyCode` (`src/termsupport.cpp`) converts to GLFW
  codes (what CC: Tweaked's `keys` uses) when the ROM is a CC: Tweaked ROM.
- **HTTP** (`src/apis/http_rules.*`, `http.cpp`): host names are resolved first and the connection goes to the
  resolved address; rules (`http_rules`, strings like `deny $private`) are ordered and merged like CC: Tweaked's
  `http.rules`. Defaults: `deny $private`, `allow *` for CC: Tweaked ROMs; the old `http_whitelist`/`http_blacklist`
  still work when `http_rules` is unset. Do not reintroduce a second DNS lookup between the check and the connection.
- **Peripheral errors** carry the position where `peripheral.call` was invoked (one level up), like CC: Tweaked.
  Errors use CC: Tweaked's wording: `bad argument #1 (string expected, got nil)`.
- **Data folder** (`src/location.cpp`): `-d` > `craftos-tweaked.json` next to the program > first-run choice
  (a `computercraft` folder next to the program, or the user's `CraftOS-Tweaked` folder; an old CraftOS-PC folder is
  offered). Mobile and web builds keep their sandbox folders.
- **Not implemented, on purpose**: anything that needs a Minecraft world (`turtle`, `commands`, `pocket`, redstone
  inputs, inventories next to a computer). `tests/specs/missing_apis_spec.lua` records them as pending.

## 8. CI and releases

- `main.yml` (CI): Linux builds (all features, none, standalone ROM), configure drift check, the CC: Tweaked test job
  per ROM, Windows x64 and ARM64. It runs on pushes (except the `rom-sync` branch) and pull requests that touch code,
  ROMs, tools or workflows. A push cancels the previous run on the same branch.
- `release.yml`: runs when a `v*` tag is pushed; refuses unless the tag equals `CRAFTOSPC_VERSION` in `src/util.hpp` and
  `CRAFTOSPC_INDEV` is false. Publishes Windows and Linux packages (each with the `roms` folder), a ROMs-only zip and
  hashes. `resources/set-version.sh <x.y.z> [--release|--dev]` sets the version everywhere and regenerates `configure`.
- `sync-roms.yml`: daily, imports new upstream ROM changes onto the `rom-sync` branch, where CI does not run. A
  maintainer reviews it through a pull request (which runs CI), adapts the emulator if behavior changed, and merges.
- Mobile builds, Windows installers, macOS packages and distro packages are not built yet.
- The in-app update checker only notifies (link to the release page); the installer path is behind
  `CRAFTOSTWEAKED_AUTOUPDATE` and not enabled.
- Crash handling: every platform writes `<data folder>/crash-logs/crash-*.log`; on the next start the user may open a
  pre-filled GitHub issue (the *Crash report* form). There is no upload code anywhere; do not add any.

## 9. Contact, security, issues

Bugs and features go to GitHub issues (forms in `.github/ISSUE_TEMPLATE`). Vulnerabilities go to the private contact
described in `SECURITY.md`, never to a public issue or pull request. Agents must not post vulnerability details anywhere
public.

## 10. Working agreements for agents

- Read `TODO.md` and `docs/cc-tweaked-compatibility.md` before starting; they list known gaps and open decisions.
- Prefer small, verified changes: build, run both ROM versions through the test runner, then commit. Say clearly what
  you did and did not verify (for example, "Linux only").
- When a decision belongs to the maintainer (legal questions, URLs, key material, naming, new platforms), ask or leave
  it in `TODO.md`; do not guess.
- Shell pitfalls seen in practice: `pkill -f <pattern>` can match and kill your own shell (use `pkill -x <name>` or kill
  by PID); `timeout` sends SIGTERM, which the emulator does not honor, so use `timeout -k <secs>` in scripts; a failed
  vcpkg/Windows build only shows its real error at the end of a very long log.
- Keep this file updated: add new commands, gotchas, decisions and layout changes as they happen, and remove what is no
  longer true.
