# ABCNote

Anything and everything. Just documenting life.

This started as a personal project for my own use, but I've decided to open-source it. Feel free to fork it, tweak it, and let your imagination run wild!

# ABCNote

ABCNote is a Qt 6 C++/QML note application designed for desktop and future mobile targets. It organizes notes by date, opens today's note on startup, shows notes in a continuous scroll, and autosaves edits to local JSON files.

## Prerequisites

- Qt 6.5 or newer with Qt Quick, Qt Quick Controls 2, and Qt Test.
- CMake 3.21 or newer.
- A C++17 compiler supported by Qt.

If Qt or related development tools need to be installed on this machine, install them under:

```text
D:\Program\dev
```

## Build

This workspace includes helper scripts for the installed local MSVC and Qt toolchain:

```powershell
.\tools\configure-msvc-qt.cmd
.\tools\build-msvc-qt.cmd
.\tools\test-msvc-qt.cmd
```

Run the app:

```powershell
.\build-msvc-qt3\ABCNote.exe
```

The scripts use:

- CMake
- Ninja: installed by winget
- Visual Studio Build Tools(2022)
- Qt(6.8.3)

The scripts also add Windows SDK paths explicitly because this Build Tools installation does not add them automatically.

## VSCode Debugging

The repository includes VSCode configuration under `.vscode/`.

Use:

- `Ctrl+Shift+B` to build with `ABCNote: Build (MSVC/Qt)`.
- `F5` and select `Debug ABCNote (MSVC/Qt)` to launch the app with the debugger.
- `Terminal > Run Task... > ABCNote: Test (MSVC/Qt)` to run unit tests.
- `Terminal > Run Task... > ABCNote: Run App` to run the app without the debugger.

The debug configuration adds Qt runtime paths automatically, including `Qt6*.dll`, QML imports, and platform plugins.

## Data Storage

Only non-empty notes are stored as JSON files under the application data location. Empty dates shown in the editor are temporary in-memory pages and are not written to disk.

The internal note structure is:

```text
data/
  2026/
    05/
      2026-05-02.json
```

Each note contains the date, body, generated summary, and last update time.

If a persisted note is cleared to empty text, its JSON file is removed and the date disappears from the left navigation. The left navigation shows only persisted non-empty notes, and year/month rows can be expanded or collapsed.

## Current AI Summary Behavior

The first version uses a local summary generator. It extracts the first meaningful sentences from the note body and can later be replaced by an OpenAI, local model, or custom API implementation.
