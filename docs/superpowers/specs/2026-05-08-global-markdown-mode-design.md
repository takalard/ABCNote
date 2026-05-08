# Global Markdown Mode Design

## Goal

Move the Markdown edit/preview switch from each daily note into the top toolbar next to the Settings button. The switch controls every daily note at once and persists across application restarts.

## Current State

`Layout/NoteItemDelegate.qml` owns a per-delegate `previewMode` property and renders a local Edit/Preview segmented control under each date title. Because each delegate owns its state, switching one note does not affect the others.

`AppSettings` already owns user preferences stored in `ABCNote.ini` through `QSettings`, including data directory and AI summary options. It is exposed to QML as `appSettings`.

## Approach

Add a global UI preference to `AppSettings`:

- `markdownPreviewMode`, defaulting to `false` so the app opens in edit mode unless the user previously chose preview.
- `setMarkdownPreviewMode(bool)`, exposed to QML and persisted to `ui/markdownPreviewMode`.
- `markdownPreviewModeChanged`, emitted when the value changes.

Update the QML layout:

- Add a compact Edit/Preview segmented control in `ApplicationWindow.header`, immediately before the Settings gear.
- Bind its checked state to `appSettings.markdownPreviewMode`.
- Remove the per-note segmented control and local `previewMode` property from `NoteItemDelegate.qml`.
- Show the editor when `!appSettings.markdownPreviewMode` and the Markdown preview panel when `appSettings.markdownPreviewMode`.

## Data Flow

When the user clicks Preview in the toolbar, QML calls `appSettings.setMarkdownPreviewMode(true)`. `AppSettings` writes the value to `ABCNote.ini`, updates its in-memory value, and emits `markdownPreviewModeChanged`. All visible note delegates update through their binding. Newly created delegates read the same setting.

## Testing

Add a focused `AppSettings` test that verifies:

- The default value is edit mode.
- Setting preview mode persists to the settings file.
- A new `AppSettings` instance loads preview mode from the same settings file.
- Switching back to edit mode persists as well.

Existing model, storage, and controller tests should continue to pass unchanged.
