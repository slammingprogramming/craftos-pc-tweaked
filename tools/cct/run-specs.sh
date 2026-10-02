#!/bin/bash
# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (c) 2026 slammingprogramming
#
# Runs CC: Tweaked's test suite (and CraftOS-Tweaked's own specs from tests/specs, listed as emu/...) against the emulator, one spec file at a time (so that a hanging or crashing spec only
# loses itself) and prints a summary table.
#
#   tools/cct/run-specs.sh <emulator> <cc-version id> <CC-Tweaked checkout> [report file]
#
# Environment: JOBS (parallel runs, default 4), SPEC_TIMEOUT (seconds per spec, default 90).
set -u
EMU="$1"; VERSION="$2"; CCT="$3"; REPORT="${4:-}"
TESTROM="$CCT/projects/core/src/test/resources/test-rom"
JOBS="${JOBS:-4}"; SPEC_TIMEOUT="${SPEC_TIMEOUT:-90}"
HERE="$(cd "$(dirname "$0")/../.." && pwd)"
WORK="$(mktemp -d)"
export SDL_AUDIODRIVER=dummy

run_one() {
    spec="$1"
    name="$(echo "$spec" | tr '/' '_')"
    mkdir -p "$WORK/$name"
    case "$spec" in
        emu/*) args="/emu-spec/${spec#emu/} /emu-spec" ;;          # CraftOS-Tweaked's own specs, tests/specs
        *) args="/test-rom/$spec" ;;
    esac
    # monitors need a window: those specs run on a virtual display when one is available (they are pending otherwise)
    launcher=(); renderer=(--headless)
    case "$spec" in emu/monitor*) if command -v xvfb-run >/dev/null 2>&1; then launcher=(xvfb-run -a); renderer=(--software-sdl); fi ;; esac
    timeout -k 5 "$SPEC_TIMEOUT" "${launcher[@]}" "$EMU" "${renderer[@]}" --cc-version "$VERSION" -d "$WORK/$name" \
        --mount-ro test-rom="$TESTROM" --mount-ro emu-spec="$HERE/tests/specs" --script "$HERE/resources/CCT-Test-Bootstrap.lua" --args "$args" \
        > "$WORK/$name/stdout.txt" 2>&1
    code=$?
    log="$WORK/$name/computer/0/test-log.txt"
    summary="$(sed -n 's/.*Ran \([0-9]*\) test(s), of which \([0-9]*\) passed.*/\1 \2/p' "$log" 2>/dev/null | tail -1)"
    if [ "$code" = 124 ] || [ "$code" = 137 ]; then echo "$spec TIMEOUT 0 0"
    elif [ -z "$summary" ]; then echo "$spec CRASH 0 0"
    else set -- $summary; echo "$spec $([ "$1" = "$2" ] && echo PASS || echo FAIL) $1 $2"
    fi
}

while read -r spec; do
    while [ "$(jobs -r | wc -l)" -ge "$JOBS" ]; do sleep 0.2; done
    run_one "$spec" >> "$WORK/results.txt" &
done < <({ cd "$TESTROM" && find spec -name '*_spec.lua'; cd "$HERE/tests/specs" && find . -name '*_spec.lua' | sed 's|^\./|emu/|'; } | sort)
wait

sort "$WORK/results.txt" > "$WORK/sorted.txt"
printf '%-52s %-8s %6s %6s\n' SPEC RESULT RAN PASSED
awk '{printf "%-52s %-8s %6s %6s\n", $1, $2, $3, $4}' "$WORK/sorted.txt"
awk '{ran+=$3; pass+=$4; n[$2]++} END {printf "\nspecs: %d pass, %d fail, %d timeout, %d crash; tests: %d ran, %d passed\n", n["PASS"], n["FAIL"], n["TIMEOUT"], n["CRASH"], ran, pass}' "$WORK/sorted.txt"
[ -n "$REPORT" ] && { mkdir -p "$(dirname "$REPORT")"; cp "$WORK/sorted.txt" "$REPORT"; }
echo "(per-spec logs: $WORK)"
! grep -qv ' PASS ' "$WORK/sorted.txt"
