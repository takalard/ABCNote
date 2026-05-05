# Temp Notes And Collapsible Navigation Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Persist only non-empty notes and add collapsible year/month navigation.

**Architecture:** Storage becomes content-aware and no longer creates files for empty missing notes. The note stream can hold temporary notes in memory. Navigation is rebuilt from persisted dates and keeps expansion state for year/month rows.

**Tech Stack:** Qt 6, C++17, QML, Qt Test.

---

## Tasks

### Task 1: Storage Semantics

**Files:**
- Modify: `src/NoteStorage.h`
- Modify: `src/NoteStorage.cpp`
- Modify: `tests/tst_abcnote.cpp`

- [ ] Add tests for non-creating load and empty-note deletion.
- [ ] Implement `load(const QDate&)`, `exists(const QDate&)`, `remove(const QDate&)`, and content-aware `save(Note)`.

### Task 2: Note Model And Controller

**Files:**
- Modify: `src/NoteModel.cpp`
- Modify: `src/NoteController.cpp`
- Modify: `tests/tst_abcnote.cpp`

- [ ] Add tests proving startup and scrolling do not create empty files.
- [ ] Add tests proving non-empty edits persist and cleared notes are deleted.
- [ ] Update model loading to use temporary notes for missing dates.
- [ ] Update controller flush to save non-empty notes and delete empty notes.

### Task 3: Collapsible Navigation

**Files:**
- Modify: `src/NavigationModel.h`
- Modify: `src/NavigationModel.cpp`
- Modify: `Layout/Main.qml`
- Modify: `tests/tst_abcnote.cpp`

- [ ] Add tests for year/month collapse and expand.
- [ ] Add `expanded` and `expandable` roles.
- [ ] Add `toggleExpanded(row)` and preserve expansion state.
- [ ] Update QML so year/month rows toggle expansion and day rows jump to dates.

### Task 4: Verification

**Files:**
- Modify: `README.md`
- Modify: `README.zh.md`

- [ ] Update behavior docs in English and Chinese.
- [ ] Run `tools\build-msvc-qt.cmd`.
- [ ] Run `tools\test-msvc-qt.cmd`.

