<!--
SPDX-FileCopyrightText: 2026 slammingprogramming
SPDX-License-Identifier: CC-BY-SA-4.0
-->
# CraftOS-Tweaked and CC: Tweaked: how close are they?

CraftOS-Tweaked is an emulator of [CC: Tweaked](https://github.com/cc-tweaked/CC-Tweaked), the Minecraft mod. It runs
CC: Tweaked's own `bios.lua` and `rom/` folder unmodified, and implements the Java-side APIs (`fs`, `http`, `os`,
`term`, `peripheral`, ...) in C++. "Exactly like CC: Tweaked" is therefore measured in two ways:

1. **CC: Tweaked's own test suite** (McFly, `projects/core/src/test/resources/test-rom`) is run against the emulator in CI
   (job *CC: Tweaked tests*, one spec file per run, for every ROM in `roms/`). `tools/cct/baseline/<version>.txt` holds
   the accepted result per version; CI fails if a spec that used to pass now fails.
2. **Reading the Java code** for what the tests do not cover (listed under *Known differences*).

Current result: **`mc-1.20.x` passes 1314 of 1314 tests** in the 79 spec files that finish (plus `timeout_spec`, see below),
**`mc-26.3` passes 1298 of 1300** (2 `textutils` tests, see below). This is a moving target: the suite only covers what CC: Tweaked's authors tested, and CC: Tweaked keeps changing. Treat
the lists below as the current state, not as a promise.

## Versions

| Folder in `roms/` | Branch | CC: Tweaked | Minecraft |
|---|---|---|---|
| `mc-1.20.x` (default) | `mc-1.20.x` | 1.120.2 | 1.20.1 |
| `mc-26.3` | `mc-26.3` | 1.120.3 | 26.3 |

The Lua API (the functions programs can call) is the same in both; what differs is the ROM: `textutils`, `vector`, the
shell, `startup.lua`, the pocket-computer `equip` program, and (1.20.x only) the `cc.internal` JSON/SNBT modules.
Other branches (`mc-1.20.y`, `mc-1.21.x`, `mc-1.21.y`, `mc-26.1`, `mc-26.2`, `master`) can be added: see *Adding a version*.

`_HOST` is read from the ROM's `rom-info.json`, so it reads exactly like CC: Tweaked's
(`ComputerCraft 1.120.2 (Minecraft 1.20.1)`), and the HTTP `User-Agent` is `computercraft/<version>`.

## What was changed to match CC: Tweaked

* **ROM**: the original CraftOS-PC 2 ROM was a modified copy of an older ComputerCraft bios. Now `bios.lua` and `rom/` are
  CC: Tweaked's, byte for byte. Emulator-specific things live in `roms-overlay/` (see its README): a `prelude.lua` that
  gives the PUC-Lua based VM the Lua 5.1 functions CC: Tweaked's Cobalt VM has (`setfenv`, `loadstring`, `bit`, ...), an
  autorun file, the emulator's programs (`attach`, `config`, `mount`, ...), the debugger and the HD font.
* **`fs`**: path handling is a port of CC: Tweaked's `FileSystem.sanitizePath`; `fs.combine`, `fs.getDir`, `fs.getName`
  follow it, and `fs.makeDir` on an existing file fails with `File exists`.
* **`os.queueEvent`** clones its arguments the way CC: Tweaked does.
* **`http`**: the `$private` rule now covers the same ranges as CC: Tweaked, including IPv6 (`::1`, `fe80::/10`,
  `fc00::/7`, IPv4-mapped addresses), CIDR rules are applied with a correct prefix mask, and scoped addresses
  (`[fe80::1%eth0]`) are rejected.
* **Timeouts**: code that never yields before its first event (for example an endless loop in `startup.lua`) is now
  interrupted with `Too long without yielding` like in CC: Tweaked; before, the emulator never stopped it.
* **Key codes**: CC: Tweaked's `keys` API (and the `key`/`key_up` events) use GLFW key codes (`keys.a` is 65) since
  1.109; the emulator used LWJGL 2 codes (`a` was 30), so the keyboard did not match the CC: Tweaked ROM's `keys` table.
  Events are now converted when the active ROM comes from CC: Tweaked (`rom-info.json` has `computercraft_version`, or
  `"key_codes": "glfw"`); ROMs without that file keep the old codes. The few LWJGL keys GLFW does not have (numpad
  `@`, `:`, `stop`, numpad `,`) are not sent. In the CLI renderer, `9` now sends the right key (it sent Escape's code).
* **Config**: `ccVersion` chooses the ROM; `--cc-version`, `--list-cc-versions`.

## Known differences

Deliberate (kept because CraftOS-PC users rely on them, or because they cannot exist in an emulator):

* **Standards mode.** Several limits that CC: Tweaked has are off unless the `standardsMode` setting is on: the computer
  space limit and the `fs.getFreeSpace`/`fs.getCapacity` values that follow from it, event time-outs of 7 seconds,
  `os.epoch`/timer rounding, the speaker queue size, mouse buttons above 3. The CC: Tweaked test run turns it on.
* **HTTP defaults.** The emulator's default configuration does not block private addresses (`http_blacklist` is empty).
  Add `$private` to get CC: Tweaked's defaults.
* **Not simulated**: anything that needs a Minecraft world (turtles moving, real redstone, inventories, the real
  `commands` API) is provided by emulator-side stand-ins or not at all.
* **Emulator extras** that CC: Tweaked does not have: `config`, `periphemu`, `ccpcTerm`, plugins, the debugger,
  `--script`/`--exec`.

Open (could be changed, not done yet):

* **Runaway pattern matching.** An endless Lua loop is stopped with `Too long without yielding`, but a
  `string.find`/`string.match` that backtracks for minutes runs inside C code that the emulator's time-out cannot
  interrupt (`timeout_spec` hangs on this and is recorded as `TIMEOUT` in the baseline). Fixing it needs a change in the
  modified Lua (`craftos2-lua`): check the halt flag inside `lstrlib.c`'s matcher. Also, outside standards mode the
  time-out is 17 seconds (`abortTimeout`), not CC: Tweaked's 7.
* **Table iteration order.** `pairs` visits hash keys in a different order than Cobalt. On `mc-26.3`, two
  `textutils.serialise` tests (the ones that print a table with both `a = 1` and `[false] = {}`) fail because of this;
  `mc-1.20.x` passes them. It comes from the Lua VM, not from the ROM.
* **HTTP checks.** CC: Tweaked resolves a host name and checks the resulting address against the rules; the emulator only
  checks the name and literal addresses. Rules in CC: Tweaked are evaluated in order with the first match winning; the
  emulator evaluates the allow list and then the block list.
* **`fs` details** beyond what the tests check (for example error texts for unusual paths).
* **Peripherals, `turtle`, `pocket`, `commands`** are not checked by the test suite (they need the Java side), so there is
  no measurement for them.

## Running the tests yourself

```sh
git clone --depth 1 --branch mc-1.20.x https://github.com/cc-tweaked/CC-Tweaked ../CC-Tweaked
tools/cct/run-specs.sh ./craftos-tweaked mc-1.20.x ../CC-Tweaked report.txt
python3 tools/cct/compare.py tools/cct/baseline/mc-1.20.x.txt report.txt
```

Each spec file runs in its own emulator process so that a crash or hang only loses that spec (`SPEC_TIMEOUT` seconds,
default 90). To run one spec: `craftos-tweaked --headless --cc-version mc-1.20.x --mount-ro test-rom=<CC-Tweaked>/projects/core/src/test/resources/test-rom --script resources/CCT-Test-Bootstrap.lua --args /test-rom/spec/apis/fs_spec.lua`.

## Adding a version

1. Add `{ "id": "mc-1.21.x", "branch": "mc-1.21.x" }` to `roms-overlay/versions.json`.
2. Run `tools/rom/compose.py` (or let the *Sync ROMs* workflow do it) to create `roms/<id>/`.
3. Run the tests for it, record `tools/cct/baseline/<id>.txt`, and add it to the CI matrix.
4. Fix what the new ROM needs from the emulator (the sync branch exists to test exactly this) and merge.

## ROM licensing

See [`roms/README.md`](../roms/README.md). In short: the ROMs keep CC: Tweaked's licenses (mostly the ComputerCraft
Public License and MPL-2.0), the emulator's code is AGPL-3.0-or-later, and the two are kept apart by folder and by
per-file license headers. The ComputerCraft Public License has conditions (redistribution under the same license with
sources, a Minecraft-ownership prerequisite, no use in projects that are not Minecraft mods). This is not legal advice;
if your use of the ROMs outside Minecraft matters to you, ask the CC: Tweaked maintainers, who hold those rights.
