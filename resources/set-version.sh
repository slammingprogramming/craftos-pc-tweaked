#!/bin/sh
# SPDX-License-Identifier: AGPL-3.0-or-later
# Copyright (c) 2026 slammingprogramming
#
# Sets the CraftOS-Tweaked version in every file that carries it.
#
#   resources/set-version.sh 0.2.0              # development version (CRAFTOSPC_INDEV true)
#   resources/set-version.sh 0.2.0 --release    # release version (CRAFTOSPC_INDEV false)
#
# Afterwards commit the changes and, for a release, tag them:
#   git tag v0.2.0 && git push origin v0.2.0
set -e
cd "$(dirname "$0")/.."

VERSION="$1"
MODE="${2:---dev}"
case "$VERSION" in
    [0-9]*.[0-9]*.[0-9]*) ;;
    *) echo "usage: $0 MAJOR.MINOR.PATCH [--dev|--release]" >&2; exit 1 ;;
esac
case "$MODE" in --dev) INDEV=true ;; --release) INDEV=false ;; *) echo "unknown option $MODE" >&2; exit 1 ;; esac

MAJOR="${VERSION%%.*}"; REST="${VERSION#*.}"; MINOR="${REST%%.*}"; PATCH="${REST#*.}"
case "$PATCH" in *[!0-9]*) echo "PATCH must be a plain number (got $PATCH)" >&2; exit 1 ;; esac
CODE=$((MAJOR * 10000 + MINOR * 100 + PATCH)) # Android versionCode, must only ever go up

edit() { # edit FILE SED-EXPRESSION...
    file="$1"; shift
    sed -i.bak "$@" "$file" && rm -f "$file.bak"
}

edit src/util.hpp -e "s|^#define CRAFTOSPC_VERSION .*|#define CRAFTOSPC_VERSION    \"v$VERSION\"|" -e "s|^#define CRAFTOSPC_INDEV .*|#define CRAFTOSPC_INDEV      $INDEV|"
edit configure.ac -e "s|^AC_INIT(\[CraftOS-Tweaked\], \[[^]]*\])|AC_INIT([CraftOS-Tweaked], [$VERSION])|"
edit vcpkg.json -e "s|\"version\": \"[^\"]*\"|\"version\": \"$VERSION\"|"
edit resources/Info.plist -e "/CFBundleShortVersionString/{n;s|<string>.*</string>|<string>$VERSION</string>|;}" -e "/<key>CFBundleVersion</{n;s|<string>.*</string>|<string>$VERSION</string>|;}"
edit src/platform/CraftOS-Tweaked.rc -e "s|FILEVERSION [0-9,]*|FILEVERSION $MAJOR,$MINOR,$PATCH,0|" -e "s|PRODUCTVERSION [0-9,]*|PRODUCTVERSION $MAJOR,$MINOR,$PATCH,0|" \
    -e "s|\"FileVersion\", \"[^\"]*\"|\"FileVersion\", \"$VERSION.0\"|" -e "s|\"ProductVersion\", \"[^\"]*\"|\"ProductVersion\", \"$VERSION.0\"|"
edit resources/CraftOS-Tweaked.exe.manifest -e "/name=\"CraftOS-Tweaked\"/{n;s|version=\"[0-9.]*\"|version=\"$VERSION.0\"|;}" # only our assemblyIdentity, not the XML declaration or the comctl32 dependency
edit resources/android-project/app/build.gradle -e "s|versionCode [0-9]*|versionCode $CODE|" -e "s|versionName \"[^\"]*\"|versionName \"$VERSION\"|"
for manifest in resources/android-project/app/AndroidManifest.xml resources/android-project/app/src/main/AndroidManifest.xml; do
    edit "$manifest" -e "s|android:versionCode=\"[0-9]*\"|android:versionCode=\"$CODE\"|" -e "s|android:versionName=\"[^\"]*\"|android:versionName=\"$VERSION\"|"
done

echo "Version set to v$VERSION (CRAFTOSPC_INDEV=$INDEV)."
if command -v autoconf >/dev/null 2>&1; then
    autoconf && rm -rf autom4te.cache && echo "Regenerated configure."
else
    echo "autoconf not found: run the 'Regenerate configure' workflow (Actions tab) or run autoconf yourself."
fi
git status --short
