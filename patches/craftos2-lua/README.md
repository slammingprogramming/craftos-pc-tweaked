<!--
SPDX-FileCopyrightText: 2026 slammingprogramming
SPDX-License-Identifier: CC-BY-SA-4.0
-->
# Patches for craftos2-lua

`craftos2-lua` is MCJack123's modified Lua 5.2 (MIT License, same as Lua itself). It is a git submodule, so changes to it
live here as patches and are applied by `tools/apply-lua-patches.sh` (CI runs it before building Lua). The patches
are under the same MIT License as the files they change.

| Patch | Why |
|---|---|
| `0001-interruptible-pattern-matching.patch` | `string.find`/`match`/`gmatch`/`gsub` run in C and never looked at the "stop this computer" flag, so a pattern that backtracks for minutes could not be interrupted. CC: Tweaked raises `Too long without yielding` there too (`timeout_spec`). |
| `0002-tables-iterate-in-insertion-order.patch` | `pairs`/`next` visited hash keys in hash-table order. CC: Tweaked's VM (Cobalt) has its own fixed order, and its ROM relies on a deterministic one (`textutils.serialise` in `mc-26.3`). Hash keys are now visited in the order they were added (array part first, as before). Costs 8 bytes per hash node. |
| `0003-argument-errors-like-cc-tweaked.patch` | Argument type errors read `bad argument #1 (string expected, got nil)` like CC: Tweaked's, not `(expected string, got no value)`. |
| `0004-halt-without-the-state-lock.patch` | `lua_halt` and `lua_externalerror` (used by the time-out timer) waited for the state's lock, which is held while code runs once a modem is attached, so a computer with a modem could not be interrupted at all. They now only set the flags the interpreter polls. |

If the submodule is moved to a fork, these can be committed there and this folder removed.
