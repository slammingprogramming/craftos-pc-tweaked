#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (c) 2026 slammingprogramming
#
# Applies the patches in patches/craftos2-lua/ to the craftos2-lua submodule (the modified Lua that the emulator runs
# programs with). Safe to run more than once: patches that are already applied are skipped. Run it after
# "git submodule update --init" and before building Lua; CI does this for every build.
set -e
cd "$(dirname "$0")/.."
for p in patches/craftos2-lua/*.patch; do
    if git -C craftos2-lua apply --reverse --check "../$p" >/dev/null 2>&1; then
        echo "already applied: $p"
    else
        git -C craftos2-lua apply "../$p"
        echo "applied: $p"
    fi
done
