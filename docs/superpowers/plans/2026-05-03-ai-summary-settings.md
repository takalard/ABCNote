# AI Summary Settings Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add user-owned large-model configuration in Settings and call AI summary only after the user has enabled it and saved an API key.

**Architecture:** `AppSettings` owns persisted AI configuration and API key storage. `SummaryService` performs OpenAI-compatible chat completion requests asynchronously. `NoteController` gates summary refresh requests and writes the returned summary into the existing `aiSummary` note field.

**Tech Stack:** Qt 6 C++17, QSettings, Qt Network, QML Controls, Windows Credential Manager through WinCred when building on Windows.

---

### Task 1: AI Settings Persistence

**Files:**
- Modify: `src/AppSettings.h`
- Modify: `src/AppSettings.cpp`
- Modify: `tests/tst_abcnote.cpp`

- [ ] Add tests proving AI settings persist, API key presence is tracked, and clearing the key disables configured summaries.
- [ ] Implement `aiSummaryEnabled`, `aiBaseUrl`, `aiModel`, `hasAiApiKey`, `saveAiSummarySettings(...)`, `clearAiApiKey()`, and `aiApiKey()`.
- [ ] Store non-secret settings in `ABCNote.ini`; store API key in Windows Credential Manager on Windows, with a QSettings fallback for non-Windows builds and test environments.
- [ ] Run `.\tools\test-msvc-qt.cmd`.

### Task 2: Async OpenAI-Compatible Summary Service

**Files:**
- Modify: `src/SummaryService.h`
- Modify: `src/SummaryService.cpp`
- Modify: `CMakeLists.txt`
- Modify: `tests/tst_abcnote.cpp`

- [ ] Add tests for disabled/missing-key behavior and request payload shape.
- [ ] Convert `SummaryService` to a `QObject` with `requestSummary(...)`.
- [ ] Use `QNetworkAccessManager` to call `{baseUrl}/chat/completions` with Bearer auth.
- [ ] Emit success and failure signals instead of returning a local placeholder.
- [ ] Link `Qt6::Network` in app and tests.
- [ ] Run `.\tools\test-msvc-qt.cmd`.

### Task 3: Controller Gating and Save Flow

**Files:**
- Modify: `src/NoteController.h`
- Modify: `src/NoteController.cpp`
- Modify: `src/NoteModel.cpp`
- Modify: `src/main.cpp`
- Modify: `tests/tst_abcnote.cpp`

- [ ] Inject `AppSettings` into `NoteController`.
- [ ] Make `refreshSummary(noteId)` refuse to summarize when AI is disabled or no key exists.
- [ ] When configured, start async summary generation, update `aiSummary`, mark dirty, and save through existing `flush()`.
- [ ] Stop auto-generating local summaries while loading notes.
- [ ] Run `.\tools\test-msvc-qt.cmd`.

### Task 4: Settings UI

**Files:**
- Modify: `Layout/Main.qml`

- [ ] Expand the settings popup to include AI Summary settings.
- [ ] Add enable switch, Base URL field, Model field, password Key field, Save, Clear Key, and status label.
- [ ] Keep the data directory controls intact.
- [ ] Run `.\tools\build-msvc-qt.cmd` and `.\tools\test-msvc-qt.cmd`.

### Task 5: Release Deploy

**Files:**
- Update generated files under `deploy/`

- [ ] Run release build.
- [ ] Refresh `deploy/ABCNote.exe`.
- [ ] Run `windeployqt` for release deployment.
- [ ] Smoke launch `deploy\ABCNote.exe`.
