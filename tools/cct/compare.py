#!/usr/bin/env python3
# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (c) 2026 slammingprogramming
"""Compares a run of tools/cct/run-specs.sh with the committed baseline.

    tools/cct/compare.py <baseline> <report>

Both files have one line per spec: "<spec> <PASS|FAIL|TIMEOUT|CRASH> <tests run> <tests passed>".
Exits with 1 if any spec got worse than the baseline (a spec that was passing stopped passing, or fewer of its
tests pass). Anything that got better is listed so the baseline can be updated (copy the report over it).
"""
import sys

ORDER = {"PASS": 3, "FAIL": 2, "TIMEOUT": 1, "CRASH": 0}


def load(path):
    rows = {}
    for line in open(path, encoding="utf-8"):
        parts = line.split()
        if len(parts) == 4:
            rows[parts[0]] = (parts[1], int(parts[2]), int(parts[3]))
    return rows


def main():
    base, new = load(sys.argv[1]), load(sys.argv[2])
    worse, better = [], []
    for spec in sorted(set(base) | set(new)):
        b = base.get(spec, ("(none)", 0, 0))
        n = new.get(spec, ("(missing)", 0, 0))
        if n[0] == "(missing)":
            worse.append(f"{spec}: no longer runs (was {b[0]})")
        elif b[0] == "(none)":
            (better if n[0] == "PASS" else worse).append(f"{spec}: new spec, {n[0]} ({n[2]}/{n[1]} pass)")
        elif ORDER[n[0]] < ORDER[b[0]] or (n[0] == b[0] and n[2] < b[2]):
            worse.append(f"{spec}: {b[0]} {b[2]}/{b[1]} -> {n[0]} {n[2]}/{n[1]}")
        elif ORDER[n[0]] > ORDER[b[0]] or n[2] > b[2]:
            better.append(f"{spec}: {b[0]} {b[2]}/{b[1]} -> {n[0]} {n[2]}/{n[1]}")
    for line in better:
        print("better: " + line)
    for line in worse:
        print("WORSE:  " + line)
    if better and not worse:
        print("\nThe emulator got better than the baseline: copy the report over the baseline file to lock that in.")
    return 1 if worse else 0


if __name__ == "__main__":
    sys.exit(main())
