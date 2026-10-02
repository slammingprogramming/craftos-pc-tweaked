# Authors and credits

CraftOS-Tweaked is a fork of [CraftOS-PC 2](https://github.com/MCJack123/craftos2),
and it exists because of the work of everyone listed here. The original
project was released under the MIT License; this fork is distributed under the
GNU Affero General Public License v3.0 or later (and CC-BY-SA-4.0 for
documentation and other non-software content). The original MIT notice is
preserved in [LICENSE](LICENSE), and every source file keeps its original
copyright line.

## Maintainer of CraftOS-Tweaked

* **slammingprogramming** ([@slammingprogramming](https://github.com/slammingprogramming)) —
  maintains this fork and holds the copyright in the work done on it since the
  fork (Copyright (c) 2026 slammingprogramming): the AGPL relicensing, the
  CraftOS-Tweaked rebranding, GitHub Actions build and release pipelines, the
  update checker, and related changes. Source files that contain such changes
  list this copyright next to the original one; the git history is the
  complete record.

## Original author

* **JackMacWindows** ([@MCJack123](https://github.com/MCJack123)) — created
  CraftOS-PC and CraftOS-PC 2 and wrote the overwhelming majority of the
  code (Copyright (c) 2019-2024 JackMacWindows).

## Contributors to CraftOS-PC 2

Listed in alphabetical order, with the contribution recorded in the project's
git history.

* **Chris E. Häußler** ([@chrissxYT](https://github.com/chrissxYT)) — macOS installation instructions fix
* **EmmaKnijn** ([@EmmaKnijn](https://github.com/EmmaKnijn)) — portable build clarification in the docs
* **Hasaabitt** ([@Hasaabitt](https://github.com/Hasaabitt)) — fix in `emu.lua`
* **LoganDark** ([@LoganDark](https://github.com/LoganDark)) — `term.getPixels`, `term.getFrozen`, `term.drawPixels` improvements, and related fixes
* **Miguel Oliveira** — `os.epoch` nano clock precision fix
* **manuel** — fix for the chest peripheral listing empty slots
* **Ocawesome101** ([@Ocawesome101](https://github.com/Ocawesome101)) — fix for building on Linux with musl libc
* **simadude** ([@simadude](https://github.com/simadude)) — fix for "Port already in use" when reopening a WebSocket server
* **Tomodachi94** ([@Tomodachi94](https://github.com/Tomodachi94)) — CI improvements

The contributor list above was compiled from the git history of this
repository. If you contributed and are missing or credited incorrectly, please
open an issue or pull request.

## Special thanks

Carried over from the original CraftOS-PC 2:

* **dan200** — for creating the ComputerCraft mod and making it open source
* **SquidDev** — for picking up ComputerCraft after Dan left and creating
  CC: Tweaked, which this fork aims to follow
* **EveryOS** — for sending JackMacWindows a patched version of Lua that fixed
  an early blocking issue
* Everyone on the Minecraft Computer Mods Discord server for their support
  during the development of CraftOS-PC 2

## CC: Tweaked and ComputerCraft

The ROMs in [`roms/`](roms/README.md) are the Lua side of [CC: Tweaked](https://github.com/cc-tweaked/CC-Tweaked) and
of the original ComputerCraft, and everything CraftOS-Tweaked emulates is theirs:

* **Daniel Ratcliffe** ([@dan200](https://github.com/dan200)) — created ComputerCraft
* **Jonathan Coates** ([@SquidDev](https://github.com/SquidDev)) and **the CC: Tweaked developers** — maintain CC: Tweaked;
  see its [contributors](https://github.com/cc-tweaked/CC-Tweaked/graphs/contributors)

Their files keep their copyright and license headers (`LicenseRef-CCPL`, `MPL-2.0`, ...); nothing in them is edited.

## Third-party code and assets

These keep their own licenses and are not relicensed:

* **ROMs** (`roms/`) — CC: Tweaked and ComputerCraft, and the emulator's own programs by JackMacWindows, under the
  licenses stated in each file (see [`roms/README.md`](roms/README.md))

* **Plugin templates** `examples/peripheral_base.cpp` and
  `examples/plugin_base.cpp` — JackMacWindows, released into the public domain
  by their author, and left that way here

* **Lua** (via the `craftos2-lua` submodule) — Lua.org, PUC-Rio, and the
  CraftOS-PC modifications by JackMacWindows
* **CCEmuX** (portions embedded in `examples/ccemux.cpp`) — Copyright (c) 2018
  CLGD, MIT License
* **gif.h** (`src/gif.hpp`, `src/gif.cpp`) — Charlie Tangora, public domain
* **Speaker instrument sounds** — OpenPathMusic and Lisa Lim (CC BY 3.0), and
  others in the public domain; see [LICENSE](LICENSE)
* **Base64 routines** in `examples/raw_frame_reader.cpp` — René Nyffenegger
* **SDL Android glue** in `resources/android-project/app/src/main/java/org/libsdl/` — the SDL project (zlib license)
* **Autoconf macros** in `m4/` and the generated `configure` script — their respective authors
