# ABCNote Multilingual Settings Design

## Goal

Add application language selection in Settings and make English the default user-facing language. ABCNote will support:

- English
- Arabic
- Simplified Chinese
- Traditional Chinese
- French
- Korean
- Japanese
- Spanish
- Portuguese

All existing default Chinese UI text, status messages, date labels, and AI summary prompt text will be moved behind a localization layer and translated into the supported languages.

## Current Context

ABCNote is a Qt 6 C++/QML desktop app. `main.cpp` creates C++ services and exposes them to QML as context properties. `Layout/Main.qml` owns the main shell and Settings popup. `Layout/NoteItemDelegate.qml` owns per-note editor text and the AI summary controls. `AppSettings` persists application settings through `QSettings`. `NoteController`, `SummaryService`, `NavigationModel`, and `NoteModel` currently emit or format several user-facing strings directly in C++.

The current visible Chinese strings are spread across:

- QML labels, tooltips, button labels, placeholders, and dialog titles.
- `AppSettings` settings-operation status messages.
- `NoteController` toolbar status messages.
- `SummaryService` AI summary errors and the system prompt sent to the model.
- `NavigationModel` year/month labels.
- `NoteModel` localized date title format.

## Recommended Approach

Create a small C++ localization service, tentatively named `Localization`, and expose it to QML as a context property named `i18n`.

This service will:

- Store the current language code.
- Persist the language in `QSettings`, defaulting to `en`.
- Provide supported-language metadata for the Settings language selector.
- Provide translated strings by key through an invokable lookup method.
- Emit a change signal so QML bindings refresh immediately after language changes.
- Provide C++ callers with the same key-based lookup API for status messages and prompts.
- Provide date, year, and month formatting helpers so C++ models stop hard-coding Chinese date text.

This keeps translation data in one compiled C++ table, avoids adding Qt `.ts/.qm` generation to the current simple build, and fits the app's current pattern of exposing C++ helpers to QML.

## Alternatives Considered

### Qt Linguist

Qt Linguist with `.ts/.qm` files is the standard Qt translation path. It handles `qsTr()` well and is strong for large translation workflows.

Trade-off: it adds translation extraction, compiled translation files, runtime translator loading, and build/deploy handling. ABCNote currently has a small text surface and embeds QML directly, so this is more infrastructure than the feature needs right now.

### QML-Only Translation Object

A JavaScript object in QML could hold translations and update text bindings.

Trade-off: this would cover QML but leave C++ status messages, date labels, and AI prompts with a separate path. It would also make tests harder because the non-UI behavior is tested in C++.

### C++ Localization Service

This is the recommended option. It keeps all user-facing strings behind one API, works from both QML and C++, supports unit tests, and requires only small CMake additions.

## Architecture

### `Localization`

Add `src/Localization.h` and `src/Localization.cpp`.

Public QML-facing properties and methods:

- `Q_PROPERTY(QString language READ language NOTIFY languageChanged)`
- `Q_PROPERTY(QVariantList languages READ languages CONSTANT)`
- `Q_INVOKABLE QString t(const QString &key) const`
- `Q_INVOKABLE bool setLanguage(const QString &languageCode)`

C++-facing helpers:

- `QString text(QStringView key) const`
- `QString summaryPrompt() const`
- `QString formatDateTitle(const QDate &date) const`
- `QString formatYearLabel(int year) const`
- `QString formatMonthLabel(int month) const`
- `bool isRightToLeft() const`

The `languages` model will expose entries with:

- `code`
- `name`
- `nativeName`
- `rtl`

Language codes:

- `en`
- `ar`
- `zh-Hans`
- `zh-Hant`
- `fr`
- `ko`
- `ja`
- `es`
- `pt`

If an unknown language code is loaded from settings, the service will fall back to `en` and persist the corrected value on the next explicit language change.

### Translation Keys

Use stable semantic keys rather than English text as keys. Example groups:

- `settings.title`
- `settings.language`
- `settings.dataDirectory`
- `settings.changeDirectory`
- `settings.aiSummary`
- `settings.enableAiSummary`
- `settings.apiKeySavedPlaceholder`
- `settings.apiKeyNewPlaceholder`
- `settings.keySaved`
- `settings.keyNotSaved`
- `settings.clearKey`
- `settings.saveAiSettings`
- `dialog.selectDataDirectory`
- `about.title`
- `about.close`
- `notes.dateNotes`
- `notes.bodyPlaceholder`
- `notes.aiSummary`
- `notes.refresh`
- `status.todayOpened`
- `status.autosaving`
- `status.autosaved`
- `status.partialSaveFailed`
- `status.aiUpdated`
- `status.aiGenerating`
- `status.aiDisabled`
- `status.aiMissingSettings`
- `status.aiEmptyBody`
- `status.aiInvalidBaseUrl`
- `status.aiNoSummaryReturned`
- `status.dataDirectoryUnchanged`
- `status.dataDirectoryMigrationFailed`
- `status.dataDirectorySaveFailed`
- `status.dataDirectorySwitched`
- `status.dataDirectorySwitchedWithExistingFiles`
- `status.aiSettingsSaveFailed`
- `status.aiKeySaveFailed`
- `status.aiSettingsSaved`
- `status.aiKeyClearFailed`
- `status.aiKeyCleared`
- `ai.summaryPrompt`

All translations will be stored as UTF-8 C++ string literals. The project already configures `/utf-8` for MSVC, so this is consistent with the current build.

### Settings Persistence

`Localization` will use the same settings file path as `AppSettings`, so language selection is stored beside existing settings in `ABCNote.ini`.

Persisted key:

- `ui/language`

Default:

- `en`

`AppSettings` should not own language state. It already owns application settings, but localization is consumed by several services. Keeping language in its own service avoids making `AppSettings` a translation hub.

### QML Integration

`main.cpp` will create one `Localization` instance before `NoteController` and expose it:

- `engine.rootContext()->setContextProperty("i18n", &localization)`

`Layout/Main.qml` and `Layout/NoteItemDelegate.qml` will replace hard-coded user-facing strings with `i18n.t("...")`.

Settings will add a `ComboBox` for language selection near the top of the Settings popup. The combo will use `i18n.languages`, display `nativeName`, and call `i18n.setLanguage(modelData.code)` on selection.

QML bindings will include `i18n.language` in text expressions where needed so they refresh when the language changes.

The main window should set layout direction from `i18n.isRightToLeft()` for Arabic:

- Arabic uses right-to-left layout direction.
- Other supported languages use left-to-right.

The first implementation should localize text and basic layout direction. It does not need per-language typography, custom fonts, or mirrored redesign beyond Qt layout direction support.

### C++ Integration

`NoteController`, `AppSettings`, `SummaryService`, `NavigationModel`, and `NoteModel` will receive or store a non-owning `Localization *` where they generate user-facing text.

Rules:

- C++ status setters should call `localization->text(key)` instead of hard-coded strings.
- `SummaryService` should use localized error messages and `ai.summaryPrompt`.
- `NavigationModel` should format year/month labels through localization.
- `NoteModel` should format date titles through localization.
- When language changes, models that display localized date/navigation text should refresh their visible data.

For existing status messages already displayed when the language changes, the initial implementation does not need to retroactively translate previously emitted status text. New operations after the language change should use the selected language. Date and navigation labels should refresh because they are persistent view content.

## Translation Scope

Translate all user-facing app text currently present in source files. Do not translate:

- Internal setting keys.
- JSON field names.
- Credential target names.
- CMake comments.
- Test fixture identifiers unless tests assert user-facing output.
- Product name `ABCNote`.
- API field names such as `Base URL`, `Model`, and `API Key` unless used as visible labels; visible labels should still be localized where natural.

README files are not part of the app runtime language switch. They can remain separate English and Chinese documents.

## Error Handling

If a translation key is missing in the selected language, `Localization` returns the English value.

If the key is missing in English as well, it returns the key string. Unit tests should catch this for known keys.

If language persistence fails, `setLanguage()` returns `false`, keeps the previous language, and leaves the UI unchanged.

If a caller passes an unsupported language code, `setLanguage()` returns `false`.

## Testing

Add or update Qt tests for:

- Default language is English when no setting exists.
- Language selection persists and reloads from `QSettings`.
- Unsupported language codes are rejected.
- Every supported language has a non-empty translation for every registered key.
- English fallback works for an intentionally missing non-English translation in a controlled test helper or narrow test-only path.
- `AppSettings` status messages are English by default.
- `NoteController` and `SummaryService` produce localized status/error text through the localization service.
- `NavigationModel` year/month labels and `NoteModel` date titles are English by default and localized after language changes.

Manual verification:

- Launch app with no `ABCNote.ini`: Settings and notes show English.
- Switch to each supported language in Settings.
- Confirm the Settings popup updates immediately.
- Confirm Arabic changes layout direction and text is readable.
- Confirm note body text and saved note content are not translated or modified.
- Confirm AI summary request prompt language changes with the selected language.

## Implementation Boundaries

This feature will not introduce live machine translation, online translation APIs, or user-editable translation files. Translations are static app strings compiled into the executable.

This feature will not translate user-authored note content, stored AI summaries already written to disk, existing JSON data, or README content.

This feature will not add OS locale auto-detection in the first pass. English remains the deterministic default as requested.

## Open Decisions Resolved

- Default language: English.
- Language selector location: Settings popup.
- Supported languages: English, Arabic, Simplified Chinese, Traditional Chinese, French, Korean, Japanese, Spanish, Portuguese.
- Translation storage: compiled C++ table.
- Runtime scope: UI text, status messages, date/navigation labels, and AI prompt/error strings.
