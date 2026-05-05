# Temp Notes And Collapsible Navigation Design

Date: 2026-05-02

## Goal

Change ABCNote so empty dates are temporary in memory only, and add collapsible year/month navigation on the left.

## Behavior

Opening today still shows today's note in the right content stream. However, if the body is empty, no JSON file is created and the date does not appear as a real recorded note in the left navigation.

Scrolling to earlier or later dates also creates in-memory temporary notes only. These dates become persistent only when their body text is non-empty after trimming whitespace.

If an existing persisted note is cleared to empty text, its JSON file is deleted. The note can remain visible on the right as a temporary page, but it disappears from the left navigation.

The left navigation shows only persisted non-empty notes. Year and month rows can be expanded or collapsed. Day rows remain clickable and jump the content stream to that date.

## Implementation Notes

`NoteStorage` will gain non-creating load, existence checks, delete support, and a content-aware save operation.

`NoteModel` will load temporary notes without writing files. Updates remain in memory until the controller flushes them.

`NoteController` will persist only non-empty bodies. Empty dirty notes delete their persisted file if one exists.

`NavigationModel` will keep expansion state for year and month keys. It will expose `expanded` and `expandable` roles, plus a `toggleExpanded(row)` invokable for QML.

## Verification

Tests must prove:

- Loading a missing note no longer creates a file.
- Saving an empty note removes or avoids a file.
- Controller startup does not create today's file.
- Editing a temp note to non-empty text creates a file after flush.
- Clearing a persisted note deletes its file after flush.
- Navigation can collapse and expand year/month rows.

