# Data Directory Settings Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a top-right settings entry that lets users change the note data directory, defaulting to an executable-adjacent `data` folder and copying existing data before switching.

**Architecture:** Add a focused `AppSettings` QObject for settings persistence, directory selection, and migration coordination. Keep `NoteStorage` responsible for the root path and file operations, and let `NoteController` reload models after a path switch.

**Tech Stack:** Qt 6, C++17, QML Controls, Qt Test, CMake.

---

### Task 1: AppSettings Storage Rules

**Files:**
- Create: `src/AppSettings.h`
- Create: `src/AppSettings.cpp`
- Modify: `tests/tst_abcnote.cpp`
- Modify: `CMakeLists.txt`

- [ ] Add tests for default executable-adjacent data path, persisted user path, migration copy, collision behavior, and storage path switch.
- [ ] Run `cmake --build build-msvc-qt3 --target ABCNoteTests` and `ctest --test-dir build-msvc-qt3 --output-on-failure`; expected first test run fails before implementation.
- [ ] Implement `AppSettings` with `dataRoot`, `statusMessage`, `setDataRootForTesting`, `migrateAndSwitchDataRoot`, `chooseDataDirectory`, and `copyDirectoryContents`.
- [ ] Add `NoteStorage::setRootPath`.
- [ ] Add `NoteController::reloadCurrentWindow`.
- [ ] Run the same build and test commands; expected pass.

### Task 2: Runtime Wiring

**Files:**
- Modify: `src/main.cpp`
- Modify: `Layout/Main.qml`

- [ ] Construct `AppSettings` before `NoteStorage` and initialize storage with `appSettings.dataRoot()`.
- [ ] Connect `AppSettings` to `NoteStorage` and `NoteController`.
- [ ] Expose `appSettings` to QML.
- [ ] Add a top-right settings button, popup with current directory, and “Change Directory” action.
- [ ] Run `cmake --build build-msvc-qt3 --target ABCNote`.
- [ ] Run `ctest --test-dir build-msvc-qt3 --output-on-failure`.
