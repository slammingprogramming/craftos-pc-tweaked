<!--
SPDX-FileCopyrightText: 2026 slammingprogramming
SPDX-License-Identifier: CC-BY-SA-4.0
-->
# roms-overlay: the files CraftOS-Tweaked adds to every ROM

CC: Tweaked's ROM is used unmodified. Everything the emulator needs on top of it lives here, and
`tools/rom/compose.py` copies it into each `roms/<version>/` folder (it only ever adds files):

| File | What it does |
|---|---|
| `prelude.lua` | Runs once before `bios.lua`. Gives the PUC-Lua based emulator the Lua 5.1 functions CC: Tweaked's Cobalt VM has (`setfenv`, `loadstring`, `bit`, ...). |
| `rom/autorun/00_craftos_tweaked.lua` | CC: Tweaked runs everything in `/rom/autorun` at startup. This restores `os.shutdown(code)`, runs the `--script`/`--exec` code, adds shell completion for the emulator's programs and reports plugin errors. |
| `rom/programs/*`, `rom/help/*` | The emulator's own programs: `attach`, `detach`, `config`, `mount`, `unmount`, `gist`, `env`, `screenfetch`, `cash`, demos. |
| `debug/` | The debugger's computer. |
| `hdfont.bmp` | The font for the HD font option. |
| `versions.json` | Which CC: Tweaked branches are imported (not copied into the ROMs). |

## Licenses

The programs, the debugger and the font come from CraftOS-PC 2 by JackMacWindows; each file has an
`SPDX-License-Identifier` header (`MPL-2.0` or `MIT`, and `LicenseRef-CCPL` for `debug/bios.lua`, which is derived from
ComputerCraft's bios). Files without a header (`rom/programs/mobile/onboarding.lua`, `hdfont.bmp`) were published under
the CraftOS-PC 2 MIT License. The prelude and the autorun file are new in CraftOS-Tweaked
(`AGPL-3.0-or-later`; the shims in the prelude come from CraftOS-PC 2's bios, MIT License). The license texts are
copied into every ROM's `LICENSES/` folder; `LicenseRef-CCPL` and `MPL-2.0` come from CC: Tweaked.
