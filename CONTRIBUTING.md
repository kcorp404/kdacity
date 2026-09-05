# Contributing to Kdacity

Kdacity is a small fork with a narrow purpose: Audacity 4, with everything that talks to a
server removed. Contributions are welcome, with that purpose in mind.

**Please do not report Kdacity issues to the Audacity project.** They did not ship this build and
cannot support it. [Open an issue here](https://github.com/kcorp404/Kdacity/issues) instead.

## The one rule that makes this fork what it is

**Nothing may reintroduce a network call.** A change that adds an account, a store, an update
check, a telemetry ping, a "check for new version", or a link that opens a web page in the
browser will be declined, however well implemented. This is not a style preference — it is the
entire reason the fork exists.

If you need to touch code near the removed features, read the `Offline build (Kdacity)` block in
[`CMakeLists.txt`](CMakeLists.txt) first. It lists every module that is forced off and explains
why turning one back on requires deleting its UI too.

## Developing

* Build instructions: [BUILDING.md](BUILDING.md).
* Kdacity is C++20 and QML. The C++ largely follows MuseScore Studio's module/IoC structure,
  since Audacity 4 is built on the Muse framework.
* Match the surrounding code. The codestyle CI check enforces the upstream formatting rules:
  C++ via `muse/buildscripts/ci/checkcodestyle/checkcodestyle.cmake` (uncrustify, with the
  config supplied by the Muse submodule) and QML via
  `buildscripts/ci/checkcodestyle/check_qml_codestyle.cmake`. You can run both locally with
  `cmake -P <script> ./src/`.
* There is **no CLA**. Contributions are accepted under the project's existing GPLv3 licence.
  (Upstream Audacity requires signing their CLA; that requirement belongs to them and does not
  apply to this fork.)

### Keeping in step with upstream

Kdacity is a fork, not a rewrite. Where a change is a general Audacity 4 improvement rather than
something specific to the offline goal, consider sending it to
[audacity/audacity](https://github.com/audacity/audacity) as well — it will reach far more people
there, and it makes future merges from upstream easier here.

## Reporting bugs

Good reports are reproducible and reduced. Before filing:

* Check whether it also happens in upstream Audacity 4. If it does, it is an upstream bug and
  worth reporting there too — say so in your issue here.
* Try to find the most general form of it. If a bug shows up when amplifying a clip two hours
  into a project, check whether another effect does the same thing, and whether it still happens
  near the start of the project.
* Include your OS, how you installed Kdacity, and the log from
  `%LOCALAPPDATA%\audacity\Audacity4Development\logs\` (Windows) — it usually names the failure
  directly.

## Plug-ins

Kdacity supports the same plug-in APIs as Audacity: Nyquist, LV2, VST3 and Audio Units (macOS).
These are not shipped with the app and you install them yourself.

Note that plug-ins run their own code and can do their own networking — removing Kdacity's online
features does not constrain what a plug-in you install may do.

## Translating

Kdacity ships the translations it inherited from Audacity and does **not** download translation
updates at runtime; that feature was removed along with everything else that reached the network.
Translations are updated by editing the files in `share/locale/` and opening a pull request.

The upstream Transifex integration is not used here, and its configuration and workflows have
been removed.
