# ABCNote Qt Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a Qt 6 C++/QML ABCNote project that opens today's note, stores notes by year/month/day JSON files, supports continuous date scrolling, and autosaves edits.

**Architecture:** C++ owns storage, models, controller, and summary generation. QML owns layout and editing UI. `NoteModel` and `NavigationModel` expose date data to QML, while `NoteController` coordinates startup, loading, edits, summary refresh, and save flushing.

**Tech Stack:** Qt 6, CMake, C++17, QML, Qt Quick Controls, Qt Test.

---

## File Structure

- Create `CMakeLists.txt`: root CMake project, app target, test target.
- Create `src/main.cpp`: application bootstrap and QML context setup.
- Create `src/Note.h`: note value object.
- Create `src/NoteStorage.h/.cpp`: JSON persistence and date path mapping.
- Create `src/SummaryService.h/.cpp`: local summary generator.
- Create `src/NoteModel.h/.cpp`: QML list model for continuous note stream.
- Create `src/NavigationModel.h/.cpp`: QML list model for year/month/day rows.
- Create `src/NoteController.h/.cpp`: application orchestration and autosave.
- Modify `Layout/Main.qml`: Qt 6 UI, left navigation, continuous list, status bar.
- Modify `Layout/NoteItemDelegate.qml`: editable note page bound to model roles.
- Create `tests/tst_abcnote.cpp`: Qt Test coverage for storage, summary, and model/controller behavior.
- Create `README.md` and `README.zh.md`: bilingual build and usage notes.

## Tasks

### Task 1: Create Build Skeleton And Tests

**Files:**
- Create: `CMakeLists.txt`
- Create: `tests/tst_abcnote.cpp`

- [ ] Add a Qt 6 CMake project with app and test targets.
- [ ] Add tests that initially fail because storage, summary, and controller classes do not exist.
- [ ] Run `cmake -S . -B build` and expect configuration to fail until Qt/CMake dependencies and source files are present.

### Task 2: Implement Storage And Summary

**Files:**
- Create: `src/Note.h`
- Create: `src/NoteStorage.h`
- Create: `src/NoteStorage.cpp`
- Create: `src/SummaryService.h`
- Create: `src/SummaryService.cpp`
- Modify: `tests/tst_abcnote.cpp`

- [ ] Test date-to-path mapping creates `data/YYYY/MM/YYYY-MM-DD.json`.
- [ ] Test missing notes are created with empty body and matching date.
- [ ] Test saved notes can be loaded back from JSON.
- [ ] Test summary returns a concise local preview for long text and a default message for empty text.
- [ ] Implement the minimal code to pass those tests.

### Task 3: Implement Models

**Files:**
- Create: `src/NoteModel.h`
- Create: `src/NoteModel.cpp`
- Create: `src/NavigationModel.h`
- Create: `src/NavigationModel.cpp`
- Modify: `tests/tst_abcnote.cpp`

- [ ] Test `NoteModel::initializeAroundDate()` loads previous, current, and next days.
- [ ] Test `loadPreviousDays()` prepends earlier dates.
- [ ] Test `loadNextDays()` appends later dates.
- [ ] Test `NavigationModel` exposes year, month, and day rows with levels `0`, `1`, and `2`.
- [ ] Implement models with stable role names for QML: `noteId`, `date`, `dateString`, `contentBody`, `aiSummary`, `level`, `display`.

### Task 4: Implement Controller And Autosave

**Files:**
- Create: `src/NoteController.h`
- Create: `src/NoteController.cpp`
- Modify: `tests/tst_abcnote.cpp`

- [ ] Test startup creates today's note.
- [ ] Test `updateNoteBody()` updates model data and marks notes dirty.
- [ ] Test `flush()` writes dirty notes.
- [ ] Test summary refresh updates the model and saved JSON.
- [ ] Implement debounced autosave with a `QTimer` and explicit `flush()`.

### Task 5: Wire QML UI

**Files:**
- Create: `src/main.cpp`
- Modify: `Layout/Main.qml`
- Modify: `Layout/NoteItemDelegate.qml`

- [ ] Register models and controller as QML context properties.
- [ ] Start on today's note.
- [ ] Implement left navigation click-to-date.
- [ ] Implement top and bottom infinite loading triggers.
- [ ] Bind the delegate editor to `contentBody` and call `noteController.updateNoteBody(noteId, text)`.

### Task 6: Documentation And Verification

**Files:**
- Create: `README.md`
- Create: `README.zh.md`

- [ ] Document prerequisites, including installing Qt 6 under `D:\Program\dev` when needed.
- [ ] Document configure, build, test, and run commands.
- [ ] Run fresh verification commands:
  - `cmake -S . -B build`
  - `cmake --build build`
  - `ctest --test-dir build --output-on-failure`
- [ ] If local Qt/CMake is unavailable, record the exact blocker and the intended commands.

