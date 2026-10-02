<!--
SPDX-FileCopyrightText: 2026 slammingprogramming
SPDX-License-Identifier: CC-BY-SA-4.0
-->
# CraftOS-Tweaked and CC: Tweaked: how close are they?

CraftOS-Tweaked is an emulator of [CC: Tweaked](https://github.com/cc-tweaked/CC-Tweaked), the Minecraft mod. It runs
CC: Tweaked's own `bios.lua` and `rom/` folder unmodified, and implements the Java-side APIs (`fs`, `http`, `os`,
`term`, `peripheral`, ...) in C++. "Exactly like CC: Tweaked" is therefore measured in two ways:

1. **CC: Tweaked's own test suite** (McFly, `projects/core/src/test/resources/test-rom`) **and CraftOS-Tweaked's own
   specs** (`tests/specs`) are run against the emulator in CI (job *CC: Tweaked tests*, one spec file per run, for every
   ROM in `roms/`). `tools/cct/baseline/<version>.txt` holds
   the accepted result per version; CI fails if a spec that used to pass now fails.
2. **Reading the Java code** for what the tests do not cover (listed under *Known differences*).

Current result (both versions): **every spec file passes**. `mc-1.20.x`: 90 specs, 1398 of 1398 tests; `mc-26.3`: 90 specs,
1384 of 1384. 79 spec files come from CC: Tweaked itself and cover the Lua-visible behavior of `fs`, `os`, `textutils`,
`term`/`window`, `http` rules, the Lua VM (time-outs, `load`, ...) and the ROM's own libraries and programs; the other
10 are CraftOS-Tweaked's own (`tests/specs`) and measure the parts CC: Tweaked can only test inside Minecraft (below).
This is a moving target: the suite only covers what CC: Tweaked's authors and we tested, and CC: Tweaked keeps changing.
Treat the lists below as the current state, not as a promise.

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

## Standards mode

`standardsMode` (CraftOS-PC's switch for "behave like CC: Tweaked") is **on by default whenever the active ROM is a CC:
Tweaked ROM**, and off for any other ROM you point `--rom` at. Set it yourself (`config set standardsMode false`, or in
`global.json`) and your choice always wins. In this mode: computers stop with an error screen when code runs too long
without yielding (7 seconds, then 1.5 more), the computer-space limit and `fs.getFreeSpace`/`fs.getCapacity` follow it,
`os.epoch`/timers round like CC: Tweaked's, `load` cannot read bytecode, speakers use CC: Tweaked's audio queue, mouse
buttons above 3 are ignored, and a window stays open (turned off) after `shutdown`, waiting for Ctrl+R. Headless and
`--cli` runs still just exit when the computer shuts down.

## What was changed to match CC: Tweaked

* **ROM**: the original CraftOS-PC 2 ROM was a modified copy of an older ComputerCraft bios. Now `bios.lua` and `rom/` are
  CC: Tweaked's, byte for byte. Emulator-specific things live in `roms-overlay/` (see its README): a `prelude.lua` that
  gives the PUC-Lua based VM the Lua 5.1 functions CC: Tweaked's Cobalt VM has (`setfenv`, `loadstring`, `bit`, ...), an
  autorun file, the emulator's programs (`attach`, `config`, `mount`, ...), the debugger and the HD font.
* **The Lua VM** (patches to the `craftos2-lua` submodule, `patches/craftos2-lua`): string matching can be interrupted
  (`Too long without yielding` also for a runaway `string.find`); `pairs` visits hash keys in insertion order, which
  fixes `textutils.serialise` on `mc-26.3`; argument errors read `bad argument #1 (string expected, got nil)`; and the
  time-out works on computers that have a modem (it used to wait for a lock forever).
* **`load`** with a reader function can yield, like in CC: Tweaked, without helper threads (a reader that errors makes
  `load` return `nil, message`).
* **`fs`**: path handling is a port of CC: Tweaked's `FileSystem.sanitizePath`; `fs.combine`, `fs.getDir`, `fs.getName`
  follow it, and `fs.makeDir` on an existing file fails with `File exists`.
* **`os.queueEvent`** clones its arguments the way CC: Tweaked does; `os.setComputerLabel` limits labels to 32
  characters like CC: Tweaked.
* **`http`**: host names are looked up and the resolved address is what the rules are applied to (and connected to, so a
  name that resolves differently a second time cannot reach a forbidden place). Rules work like CC: Tweaked's
  `http.rules`: **in order, merged** (each option comes from the first matching rule that sets it), with `host` patterns
  (`*` wildcards, CIDR such as `10.0.0.0/8`, `$private`), `port`, `max_upload`, `max_download`, `websocket_message` and
  `use_proxy`. CC: Tweaked's defaults (`deny $private`, then `allow *`) apply with a CC: Tweaked ROM. `$private` covers
  the same IPv4 and IPv6 ranges (including IPv4-mapped and 6to4 addresses), scoped addresses are refused, `Unknown host`
  is reported for names that do not resolve, and responses without a length are limited while they are read. Set the
  rules with `config.add("http_rules", "allow localhost port=8080")` (a rule is
  `[allow|deny] <host> [port=N] [max_upload=N] [max_download=N] [websocket_message=N] [use_proxy=true|false]`) or the
  `http_rules` list in `global.json`; the older `http_whitelist`/`http_blacklist` settings still work when `http_rules`
  is not set.
* **Key codes**: CC: Tweaked's `keys` API (and the `key`/`key_up` events) use GLFW key codes (`keys.a` is 65) since
  1.109; the emulator used LWJGL 2 codes (`a` was 30), so the keyboard did not match the CC: Tweaked ROM's `keys` table.
  Events are now converted when the active ROM comes from CC: Tweaked (`rom-info.json` has `computercraft_version`, or
  `"key_codes": "glfw"`); ROMs without that file keep the old codes. The few LWJGL keys GLFW does not have (numpad
  `@`, `:`, `stop`, numpad `,`) are not sent. In the CLI renderer, `9` now sends the right key (it sent Escape's code).
* **Terminal and monitors**: `write` takes any value (`nil`, booleans, tables, numbers printed like Java), moves the
  cursor by the text's length even past the edge, colour arguments follow `TermMethods.parseColour` (`Colour out of
  range`), and `monitor.setTextScale` limits its argument like CC: Tweaked.
* **Peripherals** follow the Java classes (`tests/specs`): errors from a peripheral method carry the position where
  `peripheral.call` was invoked and `peripheral.call` on a detached side says `No peripheral attached`; modem channel
  checks and messages, `open` of an open channel never counts as a new one, printers need a started page (`Page not
  started`), disk labels work for data disks, speaker notes accept any case and the same value ranges, sounds validate
  their names, and the redstone API says `nil` where Lua would say `no value`.
* **Scripted peripherals**: `periphemu.create(side, "scripted", {type = "...", methods = {...}})` attaches a peripheral
  made of Lua functions. The test run uses it to stand in for CC: Tweaked's fake peripheral hub.
* **Config**: `ccVersion` chooses the ROM; `--cc-version`, `--list-cc-versions`.
* **Timeouts**: code that never yields before its first event (for example an endless loop in `startup.lua`) is
  interrupted with `Too long without yielding` like in CC: Tweaked.

## Known differences

Deliberate (kept because CraftOS-PC users rely on them, or because they cannot exist in an emulator):

* **Outside standards mode** (an old or custom ROM, or you turned it off): no computer-space limit, a 17 second time-out
  (`abortTimeout`), no rounding, and the HTTP rules default to `allow *`, so `localhost` works.
* **No Minecraft world**: `turtle`, `commands` and `pocket` do not exist (`tests/specs/missing_apis_spec.lua` records
  them as pending, so the gap shows up in every test run), there are no redstone inputs, no inventories next to a
  computer, no disk drive blocks. Redstone outputs are stored and read back; inputs are always 0 unless a plugin sets them.
* **Unlimited printer ink and paper**, a computer peripheral that is on from the moment it is attached, and wireless
  modems that reach every modem in the emulator.
* **Monitors need a window**: in a headless run `periphemu.create(side, "monitor")` fails and the monitor specs are
  pending; the test job in CI runs them on a virtual display.
* **Emulator extras** that CC: Tweaked does not have: `config`, `periphemu`, `ccpcTerm`, extra peripheral methods (for
  example `speaker.playLocalMusic`, the monitor's pixel drawing), plugins, the debugger, `--script`/`--exec`.
* **Table order for keys that were removed and added again**: an assignment of `nil` leaves the key's place in the
  order, so adding it again puts it back where it was, not at the end. (CC: Tweaked's own order is an implementation detail
  of its VM; no program can rely on it.)

Open (could be changed, not done yet):

* **Everything that needs Minecraft** (above) has specs that record what is missing, but not an implementation.
* **`fs` details** beyond what the tests check (for example error texts for unusual paths).
* **HTTPS through a proxy** looks the host up at the proxy, as CC: Tweaked's proxy does; the address is not pinned there.

## Running the tests yourself

```sh
git clone --depth 1 --branch mc-1.20.x https://github.com/cc-tweaked/CC-Tweaked ../CC-Tweaked
tools/cct/run-specs.sh ./craftos-tweaked mc-1.20.x ../CC-Tweaked report.txt
python3 tools/cct/compare.py tools/cct/baseline/mc-1.20.x.txt report.txt
```

`run-specs.sh` runs both CC: Tweaked's specs and `tests/specs` (listed as `emu/...`; the monitor ones use `xvfb-run` when it
is installed). Each spec file runs in its own emulator process so that a crash or hang only loses that spec (`SPEC_TIMEOUT` seconds,
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
