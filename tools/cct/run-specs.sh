#!/bin/bash
# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (c) 2026 slammingprogramming
#
# Runs CC: Tweaked's test suite against the emulator, one spec file at a time (so that a hanging or crashing spec only
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
    timeout -k 5 "$SPEC_TIMEOUT" "$EMU" --headless --cc-version "$VERSION" -d "$WORK/$name" \
        --mount-ro test-rom="$TESTROM" --script "$HERE/resources/CCT-Test-Bootstrap.lua" --args "/test-rom/$spec" \
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
done < <(cd "$TESTROM" && find spec -name '*_spec.lua' | sort)
wait

sort "$WORK/results.txt" > "$WORK/sorted.txt"
printf '%-52s %-8s %6s %6s\n' SPEC RESULT RAN PASSED
awk '{printf "%-52s %-8s %6s %6s\n", $1, $2, $3, $4}' "$WORK/sorted.txt"
awk '{ran+=$3; pass+=$4; n[$2]++} END {printf "\nspecs: %d pass, %d fail, %d timeout, %d crash; tests: %d ran, %d passed\n", n["PASS"], n["FAIL"], n["TIMEOUT"], n["CRASH"], ran, pass}' "$WORK/sorted.txt"
[ -n "$REPORT" ] && { mkdir -p "$(dirname "$REPORT")"; cp "$WORK/sorted.txt" "$REPORT"; }
echo "(per-spec logs: $WORK)"
! grep -qv ' PASS ' "$WORK/sorted.txt"
