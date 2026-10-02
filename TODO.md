<!--
Copyright (c) 2026 slammingprogramming. Licensed under CC-BY-SA-4.0 (see LICENSE).
-->
# CraftOS-Tweaked to-do list

Items marked **(you)** need something only the maintainer can do (an account
setting, a secret, a decision, or a tool that is not available in CI). Search the
code for `TODO(slammingprogramming)` to find the matching spots.

## 1. Releasing

The release workflow starts when you **push a version tag**. (It does not listen for
the "release published" event: releases created with the default `GITHUB_TOKEN` never
start other workflows, so that trigger could not work.)

1. Pick the version: `vMAJOR.MINOR.PATCH`, e.g. `v0.1.0`; add a suffix for pre-releases
   (`v0.2.0-beta.1`, marked as a pre-release automatically).
2. Set it everywhere with the helper script, which also regenerates `configure`:
   ```sh
   resources/set-version.sh 0.1.0 --release   # --release sets CRAFTOSPC_INDEV to false
   git commit -am "Release v0.1.0"
   git tag v0.1.0
   git push origin HEAD v0.1.0
   ```
   The release workflow refuses to run if the tag differs from `CRAFTOSPC_VERSION` in
   `src/util.hpp` or if `CRAFTOSPC_INDEV` is still `true` (it disables the update check).
3. After the release, go back to development builds: `resources/set-version.sh 0.1.1 --dev`.

The workflow builds everything, creates the GitHub Release if needed, and uploads the
Windows (x64/ARM64) and Linux (x86_64) packages plus `sha256-hashes.txt`.

*Optional:* if you want other workflows to react to releases, create the release with a
token that is not `GITHUB_TOKEN`: make a fine-grained personal access token (GitHub →
Settings → Developer settings → Personal access tokens → Fine-grained tokens) for this
repository with *Contents: read and write*, save it as a repository secret (Settings →
Secrets and variables → Actions → New secret, e.g. `RELEASE_TOKEN`), and use it as
`GH_TOKEN` in the "Create release if needed" step of `release.yml`.

## 2. GitHub settings to check (you)

* **Labels used by the issue forms**: create `crash` and `compatibility` (and make sure
  `bug` and `enhancement` exist) under Issues → Labels. Labels that do not exist are
  silently not applied.
* **"Automatic Dependency Submission (NuGet)"** is a GitHub-generated workflow that fails
  on this repository (it fails on the upstream code too). Turn it off under Settings →
  Advanced Security → Dependency graph → Automatic dependency submission.
* Repository **Sponsor** button: `.github/FUNDING.yml` is empty on purpose (it used to
  point at the upstream author). Add your own, or leave it empty.

## 3. Placeholders to replace (you)

| Where | Currently | Replace with |
|---|---|---|
| `CRAFTOSTWEAKED_HOMEPAGE_URL` in `src/util.hpp`, `<url type="homepage">` in `resources/appdata.xml` | the GitHub repo | your project website |
| `CRAFTOSTWEAKED_DOCS_URL` in `src/util.hpp` (used in error messages) | upstream docs, `https://www.craftos-pc.cc/docs` | docs you host |
| Doc links in `src/terminal/RawTerminal.cpp` and `api/configuration.hpp` (comments) | upstream docs | your docs |
| Screenshots in `resources/appdata.xml` | one upstream screenshot | screenshots of CraftOS-Tweaked |
| Install instructions, package names and links in `README.md` | upstream's | yours, once you publish packages |
| `.gitmodules` (`craftos2-lua`) and `ROM_REPO`/`ROM_REF` in the workflows | upstream repos | your forks, if you want to control them |
| Icons: `resources/CraftOS-Tweaked.ico`, `CraftOS-PC.icns` (referenced in `Info.plist`, built outside this repo), `imageForResource:@"CraftOS-PC"` in `src/platform/macapp.mm` | upstream artwork | new artwork (rename the macOS resources together with the Xcode project) |

## 4. Crash logs (done)

Upstream's crash-log upload was removed. When CraftOS-Tweaked crashes it writes a text log
to `<data folder>/crash-logs/`. Nothing is sent anywhere. On the next start the user gets
the choice **Report on GitHub** (opens the *Crash report* issue form pre-filled with the
version, platform and the start of the log), **Show Log Folder**, or **Ignore**. Headless
and terminal runs print the same link. CI crashes a running instance on purpose and checks
the log and the prompt (Linux only; Windows and macOS are not exercised by CI yet).

Still open: Android has no crash handler yet (it keeps only its certificate setup), and
the Windows handler could use `StackWalk64` for better frames.

## 5. Mobile builds (not done yet)

Both can be built by GitHub Actions, but neither is a quick change. What each needs:

### Android (F-Droid, Obtainium)
The repository only has the Java/Gradle half of the Android app. The native half does not
exist here: `resources/android-project/app/jni/` is SDL's empty template and the
`externalNativeBuild` blocks in `app/build.gradle` are commented out. To build an APK in CI
someone has to write:
1. A native build (CMake for the NDK) that compiles CraftOS-Tweaked as `libmain.so`
   together with the modified Lua from `craftos2-lua`, with `NO_CLI`, `NO_PNG`, `NO_WEBP`
   and `NO_MIXER` at first.
2. Builds of the dependencies for Android: SDL2 (the version matching the Java files in
   `org/libsdl/app`, plus `libhidapi.so`), Poco (Foundation, Util, XML, JSON, Crypto, Net,
   NetSSL) and OpenSSL, e.g. through vcpkg's `arm64-android` triplet or from source.
3. Packaging of the ROM as an APK asset (`MainActivity` copies it out on first run).
4. A Gradle build step in a workflow on a Linux runner, signing with a release keystore.

**You provide the signing key** (one time, keep it safe; Obtainium and F-Droid users cannot
update if the key changes):
```sh
keytool -genkeypair -v -keystore craftos-tweaked.jks -alias craftos-tweaked \
        -keyalg RSA -keysize 4096 -validity 10000
base64 -w0 craftos-tweaked.jks    # paste into a repository secret
```
Secrets to create: `ANDROID_KEYSTORE_BASE64`, `ANDROID_KEYSTORE_PASSWORD`,
`ANDROID_KEY_ALIAS`, `ANDROID_KEY_PASSWORD`. Obtainium then installs the signed APK straight
from GitHub Releases. F-Droid builds from source from its own metadata repository
(<https://gitlab.com/fdroid/fdroiddata>), so for F-Droid you submit a metadata file and a
build recipe there instead; its builds are signed by F-Droid, not with your key.

### iOS (sideload only)
There is no Xcode project in the repository (`CraftOS-PC.xcodeproj` is git-ignored). To build
an unsigned `.ipa` for AltStore/SideStore in CI (macOS runner):
1. Create the Xcode project in the repository (preferably generated from a text description
   such as XcodeGen or CMake so it can be reviewed and rebuilt), using bundle identifier
   `io.github.slammingprogramming.CraftOS-Tweaked`, `src/platform/ios.mm`, `Main.storyboard`.
2. Build SDL2, Poco and OpenSSL for iOS (vcpkg's `arm64-ios` triplet is one route).
3. Remove or stub the App Store in-app-purchase code (`src/platform/ios_iap.mm`,
   `purchasePlugin`, `checkIAPEligibility`), since StoreKit cannot work when sideloaded.
4. Build with code signing disabled and zip `Payload/CraftOS-Tweaked.app` into an `.ipa`;
   publish it with the release and an AltStore/SideStore source JSON. Users' own Apple IDs
   sign it when they install.

Neither of these can be tested in CI beyond "it builds"; they need a real device to check.

## 6. Compatibility items to change later (waiting for the CC: Tweaked repository)

These were left alone on purpose so existing programs, saves and plugins keep working.
Do them once the CC: Tweaked repository is provided for the adaptation.

* **`_HOST`** (`src/Computer.cpp`) still reports `ComputerCraft x.y (CraftOS-PC ...)`
  because Lua programs detect the emulator by it. Change it to match what the latest
  CC: Tweaked reports.
* **`os.version()`** comes from the ROM's `bios.lua` (the `craftos2-rom` repository), not
  from this code (`resources/CraftOSTest.lua` also checks for `"CraftOS 1.9"`). Update both
  to whatever the latest CC: Tweaked uses.
* **Data and ROM locations**: the data directories (`%appdata%\CraftOS-PC`,
  `~/Library/Application Support/CraftOS-PC`, `$XDG_DATA_HOME/craftos-pc`, `~/.craftos`)
  and the ROM path (`/usr/share/craftos`, `/usr/local/share/craftos`, `/craftos` in the
  WebAssembly build) are unchanged. Change them to match CC: Tweaked **with a migration**
  that moves existing saves, or users lose their computers. CI log paths and `README.md`
  mention them too.
* **Plugin API**: `api/CraftOS-PC.hpp`, the `CRAFTOSPC_*` macros and the
  `craftos_pc_version` field keep their names for plugin compatibility. When renaming, add
  a `CraftOS-Tweaked.hpp` that includes the old header so existing plugins still build.
* The `ccpcTerm` protocol extension name is unchanged.

## 7. Updater

The in-app update checker queries this repository's releases (`CRAFTOSTWEAKED_REPO` in
`src/util.hpp`) and shows a notification with a link to the release page. In-place updating
(downloading and running an installer) is disabled unless `CRAFTOSTWEAKED_AUTOUPDATE` is
defined, because releases do not contain installers. To enable it later: produce
`CraftOS-Tweaked-Setup.exe` (optionally `CraftOS-Tweaked-Setup_Delta-v<version>.exe`) and
`CraftOS-Tweaked.dmg`, upload them with `sha256-hashes.txt` to each release, and build with
`-DCRAFTOSTWEAKED_AUTOUPDATE`. Windows installer signing and macOS signing/notarization need
your own certificates stored as repository secrets.

## 8. Packaging not done yet

CI/Release currently produces portable Windows (x64/ARM64) and Linux (x86_64) archives.
Not built: macOS app/dmg, iOS, Android, Windows installers, AppImage, and distro packages
(the upstream PPA, AUR and Homebrew publishing was removed because it pushed to upstream's
accounts). Linux binaries are built on `ubuntu-latest` and use system libraries.

## 9. Licensing reminders

* AGPL-3.0-or-later for code, CC-BY-SA-4.0 for documentation and other non-software
  content, original MIT notice and third-party notices preserved in `LICENSE`.
* `examples/peripheral_base.cpp` and `examples/plugin_base.cpp` were released into the
  public domain by their author and are intentionally left that way.
* AGPL section 13: if you run a modified version as a network service, offer its source to
  users. `os.about()` prints a source link for the desktop app.
* Commits are authored as `slammingprogramming` with a `Co-Authored-By` trailer for the AI
  assistant that helped write them.
