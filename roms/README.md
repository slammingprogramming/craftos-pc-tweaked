<!--
SPDX-FileCopyrightText: 2026 slammingprogramming
SPDX-License-Identifier: CC-BY-SA-4.0
-->
# ROMs: the CC: Tweaked versions CraftOS-Tweaked can emulate

Every folder here is one version of [CC: Tweaked](https://github.com/cc-tweaked/CC-Tweaked), named after the
branch it comes from:

| Folder | CC: Tweaked branch | What it is |
|---|---|---|
| `mc-1.20.x` | `mc-1.20.x` | CC: Tweaked for Minecraft 1.20.1. The default. |
| `mc-26.3` | `mc-26.3` | CC: Tweaked for Minecraft 26.3. |

Each folder holds the Lua side of that version exactly as CC: Tweaked ships it (`bios.lua` and `rom/`, byte for byte),
plus a few files that only the emulator needs (see [`roms-overlay/`](../roms-overlay/README.md)), the license texts
(`LICENSES/`) and `rom-info.json`, which records the CC: Tweaked release and commit the folder was imported from.

## Choosing a version

The emulator looks for these folders, in this order: the `CRAFTOS_TWEAKED_ROMS` environment variable, a `roms` folder
**next to the executable** (that is where releases put them), `roms` in the data folder, and the system folders
(`/usr/share/craftos-tweaked/roms` on Linux). The first folder with the requested version wins.

* `craftos-tweaked --list-cc-versions` lists what is installed.
* `craftos-tweaked --cc-version mc-26.3` picks a version for one session.
* `config set ccVersion mc-26.3` in the emulator (or `-o ccVersion=mc-26.3`) makes it the default; restart to apply it.
* `--rom <folder>` points at one ROM folder directly, or at a folder of ROMs like this one.

If nothing is chosen, `mc-1.20.x` is used. A `rom-info.json` is also what makes `_HOST` read exactly like CC: Tweaked's
(`ComputerCraft 1.120.2 (Minecraft 1.20.1)`).

## Licenses

**These files are not covered by the AGPL that covers the emulator.** They come from CC: Tweaked and keep the licenses
they have there, which differ per file:

* `LicenseRef-CCPL`: the ComputerCraft Public License, for the files that come from Daniel Ratcliffe's original
  ComputerCraft (about 80 files, including `bios.lua`, the shell and most of the original APIs).
* `MPL-2.0`: files written for CC: Tweaked, and some of the emulator's own programs.
* `MIT`, `CC0-1.0` and others where a file says so.

Every file with a license header says which one applies (`SPDX-License-Identifier`), and `upstream-REUSE.toml`
(copied from CC: Tweaked) lists the license of files that cannot carry a header, such as help text and data files.
The full texts are in each version's `LICENSES/` folder. Keep these notices when you copy or redistribute any of it.

Files that only exist in CraftOS-Tweaked state their own license in the same way (see `roms-overlay/`).

**Read the ComputerCraft Public License before redistributing the ROMs.** It allows redistribution only under the same
license and with sources, says it is granted to people who own a copy of Minecraft, and does not allow using its code in
other projects that are not Minecraft mods. If you are unsure that your use is covered, ask the CC: Tweaked maintainers.

## How the folders are made

`tools/rom/compose.py` builds a folder from a CC: Tweaked checkout and `roms-overlay/`; nothing in `bios.lua` or `rom/`
is ever edited, and the overlay may only add files. The list of versions is `roms-overlay/versions.json`.

The *Sync ROMs* workflow (`.github/workflows/sync-roms.yml`) runs it every day for every listed branch and pushes any
change to the **`rom-sync`** branch, where CI does not run. Nothing reaches the main branch until you merge it, which is
the moment to check what changed upstream, try the new ROM, and adapt the emulator.

To add a CC: Tweaked version, add it to `roms-overlay/versions.json`, run the *Sync ROMs* workflow and merge the result.
