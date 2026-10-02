# Authors and credits

CraftOS-Tweaked is a fork of [CraftOS-PC 2](https://github.com/MCJack123/craftos2),
and it exists because of the work of everyone listed here. The original
project was released under the MIT License; this fork is distributed under the
GNU Affero General Public License v3.0 or later. The original MIT notice is
preserved in [LICENSE](LICENSE), and every source file keeps its original
copyright line.

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

## Third-party code and assets

These keep their own licenses and are not relicensed:

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
