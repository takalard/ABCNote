# ABCNote Qt Design

Date: 2026-05-02

## Goal

Build ABCNote as a cross-platform Qt 6 desktop and mobile note application. The first implementation targets a working Qt C++/QML project that opens today's note on startup, shows notes in a continuous date-based writing stream, and stores note data locally in a simple year/month/day file structure.

## Platform And Build

ABCNote will use Qt 6, CMake, C++17, and QML.

Qt-related tools and SDKs should be installed under `D:\Program\dev` if installation is required. The project itself remains under `E:\work\ABCNote`.

The build system will be CMake so the same project can be opened by Qt Creator, built from command line, and later adapted for Android or iOS.

Project documentation created during this work should be generated in both English and Chinese versions.

## User Experience

The main window follows a common note application layout:

- A top toolbar with the application name and simple actions.
- A left navigation panel showing notes by date.
- A right content panel showing day notes as paper-like editable pages in a continuous scroll.

The left navigation has exactly three logical levels:

- Year, such as `2026`.
- Month, such as `05`.
- Day file, such as `2026-05-02`.

On startup, the app creates today's note if it does not exist, loads a window of nearby dates, and positions the content panel on today's note.

The right panel uses a vertical QML list. When the user scrolls near the top, earlier dates are loaded. When the user scrolls near the bottom, later dates are loaded. This creates the feeling of an endless chronological writing stream while only keeping a bounded set of delegates visible.

Each day note contains:

- Title: the date.
- Body: editable note text.
- Summary: an automatically generated concise summary.

## Data Model

The first version stores each note as a JSON file:

```text
data/
  2026/
    05/
      2026-05-02.json
```

Each file contains:

```json
{
  "date": "2026-05-02",
  "contentBody": "",
  "aiSummary": "",
  "updatedAt": "2026-05-02T12:00:00"
}
```

This matches the required three-level date structure and keeps user data easy to back up, inspect, and sync later.

## C++ Components

`Note` is the internal data object for a single date.

`NoteStorage` handles reading, writing, creating directories, and atomic file replacement.

`NoteModel` is a `QAbstractListModel` exposed to QML for the continuous note stream. It provides roles for date, formatted date text, body, and summary. It supports loading previous and next date ranges.

`NavigationModel` is a `QAbstractListModel` exposed to QML for the left date navigation. It presents year, month, and day rows with level information for indentation.

`NoteController` is exposed to QML for user actions such as updating note text, saving pending changes, jumping to a date, and regenerating summaries.

## Auto Save

Text changes from QML are sent to C++ through `NoteController`.

The controller marks the note dirty and starts a short save timer, about 800 ms. Repeated edits reset the timer, so typing does not write on every keystroke.

When the timer fires, dirty notes are written through `NoteStorage`.

The application also flushes pending changes on:

- normal application exit,
- window closing,
- app state changes where supported by Qt,
- explicit save actions if added later.

Writes use a temporary file followed by replace or rename where supported. This reduces the chance of corrupting an existing note if the app or OS stops during a write.

Sudden power loss cannot be fully guaranteed by application code, but frequent debounced saves and atomic replacement minimize lost work.

## AI Summary

The first version includes a local placeholder summary generator so the UI and data flow are complete without requiring network credentials.

The placeholder summary extracts a concise summary from the note body, such as the first meaningful sentences or a shortened preview.

The C++ boundary should keep the summary generation isolated behind `SummaryService` so a later OpenAI, local model, or custom API implementation can replace the placeholder without changing QML.

## QML Integration

Existing files will be preserved as the visual starting point:

- `Layout/Main.qml`
- `Layout/NoteItemDelegate.qml`

The QML will be cleaned up for UTF-8 Chinese text, Qt 6 imports, model role names, and reliable editor behavior.

`Main.qml` will own the application layout and call controller methods for navigation and infinite loading.

`NoteItemDelegate.qml` will render one date note page and notify the controller when the body changes.

## Error Handling

If a note file cannot be parsed, the app should avoid overwriting it immediately. It can load an empty note in memory and write a warning to debug output.

If saving fails, the controller should keep the note dirty and expose a status message for the UI.

If the data directory does not exist, the app creates it automatically.

## Testing And Verification

The first implementation should verify:

- CMake configures and builds.
- The app starts and loads QML.
- Today's note file is created on startup.
- Editing note text creates or updates the JSON file.
- Previous and next dates can be loaded.
- Navigation rows follow year/month/day structure.

Automated tests can be added around `NoteStorage`, date range loading, and summary generation because those are independent of the QML UI.
