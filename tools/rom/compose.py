#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (c) 2026 slammingprogramming
"""Builds a CraftOS-Tweaked ROM folder from a CC: Tweaked checkout.

    tools/rom/compose.py --upstream <CC-Tweaked checkout> --id mc-1.20.x --branch mc-1.20.x

writes roms/<id>/ containing

  * bios.lua and rom/ copied byte for byte from CC: Tweaked (nothing in them is edited),
  * the emulator-only files from roms-overlay/ added next to them (the prelude, the autorun hook,
    the emulator's own programs, the debugger, the HD font); the overlay may only ADD files,
  * LICENSES/ and upstream-REUSE.toml from CC: Tweaked, which describe the license of every file, and
  * rom-info.json, which says which CC: Tweaked release the folder came from.

The result only depends on the checkout and the overlay, so running it again with the same inputs
changes nothing.
"""
import argparse
import json
import re
import shutil
import subprocess
import sys
from pathlib import Path

LUA_DIR = Path("projects/core/src/main/resources/data/computercraft/lua")
SKIP_OVERLAY = {"versions.json", "README.md"}


def read_value(path: Path, pattern: str) -> str:
    match = re.search(pattern, path.read_text(encoding="utf-8"), re.M)
    return match.group(1) if match else ""


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--upstream", required=True, type=Path, help="CC: Tweaked checkout (the branch to import)")
    ap.add_argument("--id", required=True, help="folder name, e.g. mc-1.20.x")
    ap.add_argument("--branch", required=True, help="CC: Tweaked branch the checkout is on")
    ap.add_argument("--roms", type=Path, default=Path("roms"), help="output folder that holds all ROMs")
    ap.add_argument("--overlay", type=Path, default=Path("roms-overlay"))
    args = ap.parse_args()

    up, lua = args.upstream, args.upstream / LUA_DIR
    if not (lua / "bios.lua").is_file() or not (lua / "rom").is_dir():
        print(f"error: {lua} does not look like CC: Tweaked's Lua resources", file=sys.stderr)
        return 1
    out = args.roms / args.id
    if out.exists():
        shutil.rmtree(out)
    out.mkdir(parents=True)

    shutil.copy2(lua / "bios.lua", out / "bios.lua")
    shutil.copytree(lua / "rom", out / "rom")

    # The overlay only adds files. A file that CC: Tweaked already has is never replaced.
    for src in sorted(args.overlay.rglob("*")):
        if not src.is_file():
            continue
        rel = src.relative_to(args.overlay)
        if len(rel.parts) == 1 and rel.name in SKIP_OVERLAY:
            continue
        dst = out / rel
        if dst.exists():
            print(f"error: overlay file {rel} would replace a file from CC: Tweaked", file=sys.stderr)
            return 1
        dst.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(src, dst)

    shutil.copytree(up / "LICENSES", out / "LICENSES")
    if (up / "REUSE.toml").is_file():
        shutil.copy2(up / "REUSE.toml", out / "upstream-REUSE.toml")

    commit = subprocess.run(["git", "-C", str(up), "rev-parse", "HEAD"], capture_output=True, text=True).stdout.strip()
    info = {
        "id": args.id,
        "upstream_repository": "https://github.com/cc-tweaked/CC-Tweaked",
        "upstream_branch": args.branch,
        "upstream_commit": commit,
        "computercraft_version": read_value(up / "gradle.properties", r"^version=(.+)$"),
        "minecraft_version": read_value(up / "gradle" / "libs.versions.toml", r'^minecraft\s*=\s*"([^"]+)"'),
    }
    (out / "rom-info.json").write_text(json.dumps(info, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"{args.id}: CC: Tweaked {info['computercraft_version']} (Minecraft {info['minecraft_version']}) @ {commit[:10]}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
