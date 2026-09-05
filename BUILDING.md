# Building Kdacity

Kdacity builds the same way Audacity 4 does. This document covers the verified Windows path
first, because that is the platform the fork is developed and packaged on, then the parts
inherited from upstream.

If you only want to run Kdacity, download the installer from the
[Releases page](https://github.com/kcorp404/Kdacity/releases) instead.

## Requirements

* Git
* CMake 3.24 or newer
* A C++20 compiler (tested: MSVC 2022 on Windows; g++ on Linux)
* A CMake generator (tested: Visual Studio 17 2022 on Windows, Ninja elsewhere)
* Qt 6.10 — `MSVC 2022 64-bit` on Windows, `Desktop` on macOS — with these **Additional
  Libraries**:
  * Qt 5 Compatibility Module
  * Qt Network Authorization
  * Qt Shader Tools
  * Qt State Machines
* Windows packaging only: [Inno Setup 6](https://jrsoftware.org/isdl.php)

By default the Qt Online Installer only offers the newest Qt. To reach 6.10, tick
**Show > Archive** in the top-right of the installer, next to the search bar.

## Getting the source

Kdacity uses submodules for the Muse framework and its dependencies, so clone recursively:

```bash
git clone --recurse-submodules https://github.com/kcorp404/Kdacity.git
```

If you already cloned without `--recurse-submodules`:

```bash
git submodule update --init --recursive
```

Audacity 3 lives in-tree under `au3/` and is **not** a submodule — it comes with the clone.

## Building on Windows

Configure, build, install:

```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="C:/Qt/6.10.3/msvc2022_64"
```

```bash
cmake --build build --config Release --parallel
```

```bash
cmake --install build --config Release --prefix dist
```

Then run `dist/bin/Kdacity4.exe`.

### Two quirks that will otherwise cost you an afternoon

**1. `cmake --install` is not optional, and `dist/` is not just an install target.**

`src/app/CMakeLists.txt` sets the executable's `RUNTIME_OUTPUT_DIRECTORY` to
`${CMAKE_INSTALL_PREFIX}/bin`. Under a multi-config generator that means MSVC *builds* the
binary into `dist/bin/Release/`, and the install step then copies it up to `dist/bin/` and runs
`windeployqt` alongside it.

Two consequences:

* Running the freshly built `dist/bin/Release/Kdacity4.exe` directly **will hang on the splash
  screen**. Qt's QML modules have not been deployed next to it yet, so the shell QML fails to
  load and the main window is never created. Always run the installed `dist/bin/Kdacity4.exe`.
* Deleting `dist/` deletes the build output too, and `cmake --install` will then fail with
  `file INSTALL cannot find .../dist/bin/Release/Kdacity4.exe`. Rebuild before installing again.

**2. Removing a QML file or a QML-registered C++ type needs a clean of the generated module.**

Qt generates `*_qmltyperegistrations.cpp` from the moc metatypes of a QML module's sources. Those
generated files and the copied `.qml` files are not invalidated when you *remove* a source, so
the build fails with `'SomeModel': is not a member of ...`. Clear the stale artifacts and
reconfigure:

```bash
rm -rf build/src/<module>/qml/<Uri>/meta_types && find build/src -name "*_qmltyperegistrations.cpp" -delete
```

### With Visual Studio

`generate_sln.bat` in the repository root generates `build/kdacity.sln` and builds the install
target. Open the solution and press F5.

### With VS Code

The default generator in `.vscode/settings.json` is Ninja — install it, or change
`cmake.generator`.

1. Ctrl+Shift+P → **Open Workspace from File**, and pick `.vscode/audacity.code-workspace`.
2. Install the recommended extensions when prompted.
3. Ctrl+Shift+P → **CMake: Configure**.
4. Press F5. The first run builds and installs everything; later runs are much faster.

### With Qt Creator

Open `CMakeLists.txt`, configure with the auto-detected Qt kit, and build. Qt Creator gives the
best QML intellisense and debugging, though debugging on Windows is slow — if you work mostly in
the C++, one of the options above is more comfortable.

## Building the Windows installer

Upstream packages an MSI through CPack and WiX Toolset v3.11. Kdacity also ships an Inno Setup
script, which is what the published installer is built with:

```bash
powershell -ExecutionPolicy Bypass -File buildscripts/packaging/Windows/make_installer.ps1
```

That script runs `cmake --install`, reads the version from `version.cmake` so the installer can
never drift from the app, verifies the staged tree is complete, and compiles
`buildscripts/packaging/Windows/Installer/Kdacity.iss`. The result lands in
`build.artifacts/Kdacity-<version>-x86_64-setup.exe`.

Pass `-SkipInstall` to package a `dist/` tree you have already installed.

The completeness check exists for a reason: an installer built from a `dist/` without `qml/` in
it produces an app that hangs on the splash screen, so the script refuses to package one.

To build the MSI instead, install WiX Toolset v3.11 and use the upstream path in
`buildscripts/ci/windows/package.cmake`. Without WiX present, CPack silently falls back to
producing a ZIP.

## Building on Linux and macOS

These are inherited from upstream and are **untested in this fork**. The general MuseScore Studio
setup applies, since large parts of Audacity 4 are based on it:

1. [Set up a developer environment](https://github.com/musescore/MuseScore/wiki/Set-up-developer-environment)
2. [Install Qt and Qt Creator](https://github.com/musescore/MuseScore/wiki/Install-Qt-and-Qt-Creator)

Then the standard sequence:

```bash
cmake -S . -B build && cmake --build build && cmake --install build
```

Ninja should resolve most remaining dependencies. If it does not, the list can be inferred from
the `setup` script in `buildscripts/ci/<your OS>/`. That list is long, largely because of
MuseScore dependencies that have not been cleaned up yet.

Make sure Git, CMake, Ninja, your package manager, your compiler and Qt are all on `PATH`, or
you will need to point at them explicitly in the CMake cache.

## Build options specific to this fork

Kdacity forces every network-reaching module off in `CMakeLists.txt`, under a block headed
`Offline build (Kdacity)`:

```
AU_BUILD_CLOUD_AUDIOCOM, AU_BUILD_USAGEINFO_MODULE, AU_USE_LIBCURL,
MUSE_MODULE_CLOUD, MUSE_MODULE_CLOUD_MUSESCORECOM, MUSE_MODULE_UPDATE,
MUSE_MODULE_LEARN, MUSE_MODULE_TOURS, MUSE_MODULE_MUSESAMPLER,
MUSE_MODULE_NETWORK, MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT
```

These are set with `FORCE` where they are cache variables, so passing `-D...=ON` on the command
line will not turn them back on. That is deliberate. If you genuinely want one of them back you
must edit that block *and* restore the corresponding UI, which was deleted rather than hidden —
a half-enabled module is what leaves dead cloud entries sitting greyed-out in the menus.
