# Multilingual Settings Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Add a Settings language selector and localize ABCNote runtime UI/status text into English, Arabic, Simplified Chinese, Traditional Chinese, French, Korean, Japanese, Spanish, and Portuguese, with English as the default.

**Architecture:** Add a focused C++ `Localization` QObject that owns language persistence, translation lookup, supported-language metadata, and date/navigation formatting. Expose it to QML as `i18n`, inject it into C++ services that emit user-facing text, and keep existing note/user data untouched.

**Tech Stack:** Qt 6 Core/QML/Quick, C++17, QSettings, Qt Test, existing CMake/MSVC Qt build scripts.

---

## File Structure

- Create `src/Localization.h`: QObject API, language metadata, translation lookup, and formatting helpers.
- Create `src/Localization.cpp`: supported languages, translation table, QSettings persistence, fallback behavior, date/month/year formatting.
- Modify `CMakeLists.txt`: add `Localization.cpp` and `Localization.h` to shared app/test sources.
- Modify `src/main.cpp`: construct `Localization`, pass settings path/application dir consistently, expose `i18n`, inject into models/services/controllers/settings.
- Modify `src/AppSettings.h/.cpp`: accept an optional `Localization *`, localize status messages.
- Modify `src/SummaryService.h/.cpp`: accept an optional `Localization *`, localize errors and AI system prompt.
- Modify `src/NoteController.h/.cpp`: accept an optional `Localization *`, localize toolbar status messages.
- Modify `src/NavigationModel.h/.cpp`: accept an optional `Localization *`, refresh labels on language changes.
- Modify `src/NoteModel.h/.cpp`: accept an optional `Localization *`, refresh date title role on language changes.
- Modify `Layout/Main.qml`: add Settings language ComboBox and replace hard-coded UI strings with `i18n.t(...)`.
- Modify `Layout/NoteItemDelegate.qml`: replace hard-coded note/summary strings with `i18n.t(...)`.
- Modify `tests/tst_abcnote.cpp`: add focused localization tests and update expectations that currently assert Chinese defaults.

---

### Task 1: Add Localization API Tests

**Files:**
- Modify: `tests/tst_abcnote.cpp`
- Later implementation target: `src/Localization.h`, `src/Localization.cpp`, `CMakeLists.txt`

- [ ] **Step 1: Include the future header**

Add this include beside the other project includes:

```cpp
#include "Localization.h"
```

- [ ] **Step 2: Declare localization test slots**

Add these private slots after `summaryServiceCallsOpenAiCompatibleEndpoint()`:

```cpp
    // Verifies localization defaults to English and exposes all supported languages.
    void localizationDefaultsToEnglishAndListsLanguages();

    // Verifies localization persists the selected language.
    void localizationPersistsLanguageSelection();

    // Verifies unsupported language codes are rejected without changing state.
    void localizationRejectsUnsupportedLanguage();

    // Verifies every supported language has translations for registered keys.
    void localizationAllSupportedLanguagesCoverRegisteredKeys();

    // Verifies localized date, year, and month formatting.
    void localizationFormatsDatesAndNavigationLabels();
```

- [ ] **Step 3: Add test implementations**

Insert these tests after `summaryServiceCallsOpenAiCompatibleEndpoint()`:

```cpp
void ABCNoteTests::localizationDefaultsToEnglishAndListsLanguages()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Localization localization(QDir(dir.path()).filePath("settings.ini"));

    QCOMPARE(localization.language(), QStringLiteral("en"));
    QCOMPARE(localization.t(QStringLiteral("settings.title")), QStringLiteral("Settings"));
    QCOMPARE(localization.t(QStringLiteral("notes.bodyPlaceholder")), QStringLiteral("Start writing today..."));

    const QVariantList languages = localization.languages();
    QCOMPARE(languages.size(), 9);
    QCOMPARE(languages.first().toMap().value(QStringLiteral("code")).toString(), QStringLiteral("en"));
    QVERIFY(languages.first().toMap().contains(QStringLiteral("nativeName")));
    QVERIFY(languages.first().toMap().contains(QStringLiteral("rtl")));
}

void ABCNoteTests::localizationPersistsLanguageSelection()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString settingsPath = QDir(dir.path()).filePath("settings.ini");
    Localization first(settingsPath);
    QVERIFY(first.setLanguage(QStringLiteral("ja")));
    QCOMPARE(first.language(), QStringLiteral("ja"));
    QCOMPARE(first.t(QStringLiteral("settings.title")), QStringLiteral("設定"));

    Localization second(settingsPath);
    QCOMPARE(second.language(), QStringLiteral("ja"));
    QCOMPARE(second.t(QStringLiteral("notes.refresh")), QStringLiteral("更新"));
}

void ABCNoteTests::localizationRejectsUnsupportedLanguage()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Localization localization(QDir(dir.path()).filePath("settings.ini"));
    QVERIFY(localization.setLanguage(QStringLiteral("fr")));
    QVERIFY(!localization.setLanguage(QStringLiteral("xx")));
    QCOMPARE(localization.language(), QStringLiteral("fr"));
}

void ABCNoteTests::localizationAllSupportedLanguagesCoverRegisteredKeys()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Localization localization(QDir(dir.path()).filePath("settings.ini"));
    const QStringList keys = localization.translationKeys();
    QVERIFY(keys.size() > 30);

    const QVariantList languages = localization.languages();
    for (const QVariant &entry : languages) {
        const QString code = entry.toMap().value(QStringLiteral("code")).toString();
        QVERIFY2(localization.setLanguage(code), qPrintable(code));
        for (const QString &key : keys) {
            const QString value = localization.t(key);
            QVERIFY2(!value.trimmed().isEmpty(), qPrintable(code + QStringLiteral(":") + key));
            QVERIFY2(value != key, qPrintable(code + QStringLiteral(":") + key));
        }
    }
}

void ABCNoteTests::localizationFormatsDatesAndNavigationLabels()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Localization localization(QDir(dir.path()).filePath("settings.ini"));
    const QDate date(2026, 5, 2);

    QCOMPARE(localization.formatYearLabel(2026), QStringLiteral("2026"));
    QCOMPARE(localization.formatMonthLabel(5), QStringLiteral("May"));
    QCOMPARE(localization.formatDateTitle(date), QStringLiteral("Saturday, May 2, 2026"));

    QVERIFY(localization.setLanguage(QStringLiteral("zh-Hans")));
    QCOMPARE(localization.formatYearLabel(2026), QStringLiteral("2026年"));
    QCOMPARE(localization.formatMonthLabel(5), QStringLiteral("05月"));
    QCOMPARE(localization.formatDateTitle(date), QStringLiteral("2026年5月2日 星期六"));

    QVERIFY(localization.setLanguage(QStringLiteral("ar")));
    QVERIFY(localization.isRightToLeft());
    QVERIFY(localization.formatMonthLabel(5).contains(QStringLiteral("مايو")));
}
```

- [ ] **Step 4: Run test to verify it fails**

Run:

```powershell
tools\test-msvc-qt.cmd
```

Expected: build or test fails because `Localization.h` does not exist or the new API is undefined.

- [ ] **Step 5: Implement the minimal API and translations**

Create `src/Localization.h`:

```cpp
#pragma once

#include <QDate>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

class Localization : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString language READ language NOTIFY languageChanged)
    Q_PROPERTY(QVariantList languages READ languages CONSTANT)
    Q_PROPERTY(bool rightToLeft READ isRightToLeft NOTIFY languageChanged)

public:
    explicit Localization(QString settingsFilePath, QObject *parent = nullptr);

    QString language() const;
    QVariantList languages() const;
    bool isRightToLeft() const;
    QStringList translationKeys() const;

    Q_INVOKABLE QString t(const QString &key) const;
    Q_INVOKABLE bool setLanguage(const QString &languageCode);
    Q_INVOKABLE int languageIndex() const;

    QString text(QStringView key) const;
    QString formatDateTitle(const QDate &date) const;
    QString formatYearLabel(int year) const;
    QString formatMonthLabel(int month) const;

signals:
    void languageChanged();

private:
    QString loadLanguage() const;
    bool isSupportedLanguage(const QString &languageCode) const;
    QLocale localeForCurrentLanguage() const;

    QString m_settingsFilePath;
    QString m_language;
};
```

Create `src/Localization.cpp` with static language metadata, full key coverage for all nine languages, `QSettings` persistence at `ui/language`, and formatting through `QLocale`.

Important implementation details:

```cpp
Localization::Localization(QString settingsFilePath, QObject *parent)
    : QObject(parent)
    , m_settingsFilePath(std::move(settingsFilePath))
    , m_language(loadLanguage())
{
}

QString Localization::t(const QString &key) const
{
    const auto translations = translationTable();
    const auto languageIt = translations.constFind(m_language);
    if (languageIt != translations.constEnd() && languageIt->contains(key)) {
        return languageIt->value(key);
    }
    const auto englishIt = translations.constFind(QStringLiteral("en"));
    if (englishIt != translations.constEnd() && englishIt->contains(key)) {
        return englishIt->value(key);
    }
    return key;
}
```

- [ ] **Step 6: Add Localization to CMake**

Add to `ABCNOTE_SOURCES`:

```cmake
    src/Localization.cpp
```

Add to `ABCNOTE_HEADERS`:

```cmake
    src/Localization.h
```

- [ ] **Step 7: Run test to verify it passes**

Run:

```powershell
tools\test-msvc-qt.cmd
```

Expected: the new localization tests pass. Older tests may still fail only where default Chinese labels changed in later tasks; if they fail now, update only the tests covered by this task.

- [ ] **Step 8: Commit**

```powershell
git add CMakeLists.txt src/Localization.h src/Localization.cpp tests/tst_abcnote.cpp
git commit -m "Add localization service"
```

---

### Task 2: Localize AppSettings and SummaryService

**Files:**
- Modify: `tests/tst_abcnote.cpp`
- Modify: `src/AppSettings.h`
- Modify: `src/AppSettings.cpp`
- Modify: `src/SummaryService.h`
- Modify: `src/SummaryService.cpp`

- [ ] **Step 1: Write failing tests**

Add private slots near the existing settings and summary tests:

```cpp
    // Verifies AppSettings status messages use the selected language.
    void appSettingsStatusMessagesUseLocalization();

    // Verifies SummaryService validation messages use the selected language.
    void summaryServiceValidationMessagesUseLocalization();
```

Add implementations near related tests:

```cpp
void ABCNoteTests::appSettingsStatusMessagesUseLocalization()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString settingsPath = QDir(dir.path()).filePath("settings.ini");
    const QString appDir = QDir(dir.path()).filePath("app");
    QVERIFY(QDir().mkpath(appDir));

    Localization localization(settingsPath);
    AppSettings settings(settingsPath, appDir, &localization);

    QVERIFY(settings.migrateAndSwitchDataRoot(settings.dataRoot()));
    QCOMPARE(settings.statusMessage(), QStringLiteral("Data directory unchanged"));

    QVERIFY(localization.setLanguage(QStringLiteral("fr")));
    QVERIFY(settings.saveAiSummarySettings(true,
                                           QStringLiteral("https://api.example.com/v1"),
                                           QStringLiteral("example-model"),
                                           QString()));
    QCOMPARE(settings.statusMessage(), QStringLiteral("Paramètres du résumé IA enregistrés"));
}

void ABCNoteTests::summaryServiceValidationMessagesUseLocalization()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Localization localization(QDir(dir.path()).filePath("settings.ini"));
    SummaryService summary(&localization);
    QSignalSpy failedSpy(&summary, &SummaryService::summaryFailed);

    SummaryService::RequestSettings settings;
    summary.requestSummary(QStringLiteral("2026-05-03"), QStringLiteral("Body"), settings);
    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(failedSpy.takeFirst().at(1).toString(), QStringLiteral("AI summary is disabled"));

    QVERIFY(localization.setLanguage(QStringLiteral("es")));
    summary.requestSummary(QStringLiteral("2026-05-03"), QStringLiteral("Body"), settings);
    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(failedSpy.takeFirst().at(1).toString(), QStringLiteral("El resumen de IA está desactivado"));
}
```

- [ ] **Step 2: Run tests to verify failure**

Run:

```powershell
tools\test-msvc-qt.cmd
```

Expected: constructor overloads or localized messages are missing.

- [ ] **Step 3: Inject Localization into AppSettings**

In `AppSettings.h`, forward declare `Localization`, update constructors, add member, and helper:

```cpp
class Localization;

explicit AppSettings(QObject *parent = nullptr);
AppSettings(QString settingsFilePath,
            QString applicationDirPath,
            Localization *localization = nullptr,
            QObject *parent = nullptr);

QString statusText(QStringView key) const;
Localization *m_localization = nullptr;
```

In `AppSettings.cpp`, include `Localization.h`, initialize `m_localization`, and replace status literals with `statusText(u"...")`.

Example:

```cpp
QString AppSettings::statusText(QStringView key) const
{
    return m_localization ? m_localization->text(key) : QString(key.toString());
}
```

Use these keys:

- `status.dataDirectoryUnchanged`
- `status.dataDirectoryMigrationFailed`
- `status.dataDirectorySaveFailed`
- `status.dataDirectorySwitchedWithExistingFiles`
- `status.dataDirectorySwitched`
- `status.aiSettingsSaveFailed`
- `status.aiKeySaveFailed`
- `status.aiSettingsSaved`
- `status.aiKeyClearFailed`
- `status.aiKeyCleared`

- [ ] **Step 4: Inject Localization into SummaryService**

In `SummaryService.h`, forward declare `Localization`, update constructor, add member:

```cpp
class Localization;
explicit SummaryService(Localization *localization = nullptr, QObject *parent = nullptr);
Localization *m_localization = nullptr;
QString statusText(QStringView key) const;
```

In `SummaryService.cpp`, include `Localization.h`, initialize member, and use:

```cpp
emit summaryFailed(noteId, statusText(u"status.aiDisabled"));
emit summaryFailed(noteId, statusText(u"status.aiMissingSettings"));
emit summaryFailed(noteId, statusText(u"status.aiEmptyBody"));
emit summaryFailed(noteId, statusText(u"status.aiInvalidBaseUrl"));
emit summaryFailed(noteId, statusText(u"status.aiNoSummaryReturned"));
```

For the system prompt:

```cpp
const QString prompt = m_localization
                           ? m_localization->t(QStringLiteral("ai.summaryPrompt"))
                           : QStringLiteral("You are ABCNote's note summary assistant. Summarize the user's note concisely.");
```

- [ ] **Step 5: Update existing summary/settings expectations**

Change assertions that currently look for Chinese default text to English default text. Example:

```cpp
QVERIFY(settings.statusMessage().contains(QStringLiteral("already exists")));
```

- [ ] **Step 6: Run tests to verify pass**

Run:

```powershell
tools\test-msvc-qt.cmd
```

Expected: all AppSettings and SummaryService tests pass.

- [ ] **Step 7: Commit**

```powershell
git add src/AppSettings.h src/AppSettings.cpp src/SummaryService.h src/SummaryService.cpp tests/tst_abcnote.cpp
git commit -m "Localize settings and summary messages"
```

---

### Task 3: Localize NoteController, NavigationModel, and NoteModel

**Files:**
- Modify: `tests/tst_abcnote.cpp`
- Modify: `src/NoteController.h`
- Modify: `src/NoteController.cpp`
- Modify: `src/NavigationModel.h`
- Modify: `src/NavigationModel.cpp`
- Modify: `src/NoteModel.h`
- Modify: `src/NoteModel.cpp`

- [ ] **Step 1: Write failing tests**

Add private slots:

```cpp
    // Verifies controller toolbar statuses use localization.
    void controllerStatusMessagesUseLocalization();

    // Verifies models refresh localized date/navigation labels after language changes.
    void modelsRefreshLocalizedLabelsWhenLanguageChanges();
```

Add implementations near existing controller/model tests:

```cpp
void ABCNoteTests::controllerStatusMessagesUseLocalization()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Localization localization(QDir(dir.path()).filePath("settings.ini"));
    NoteStorage storage(dir.path());
    SummaryService summary(&localization);
    NoteModel noteModel(&storage, &summary, &localization);
    NavigationModel navigationModel(&storage, &localization);
    NoteController controller(&storage, &summary, &noteModel, &navigationModel, nullptr, &localization);

    const QDate date(2026, 5, 3);
    controller.start(date);
    QCOMPARE(controller.statusMessage(), QStringLiteral("Today's note opened"));

    QVERIFY(localization.setLanguage(QStringLiteral("ko")));
    controller.updateNoteBody(date.toString(Qt::ISODate), QStringLiteral("Body"));
    QCOMPARE(controller.statusMessage(), QStringLiteral("자동 저장 중..."));
}

void ABCNoteTests::modelsRefreshLocalizedLabelsWhenLanguageChanges()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Localization localization(QDir(dir.path()).filePath("settings.ini"));
    NoteStorage storage(dir.path());
    SummaryService summary(&localization);

    Note note;
    note.date = QDate(2026, 5, 2);
    note.contentBody = "Persisted note";
    QVERIFY(storage.save(note));

    NoteModel noteModel(&storage, &summary, &localization);
    NavigationModel navigationModel(&storage, &localization);
    noteModel.initializeAroundDate(note.date);
    navigationModel.refresh();

    QCOMPARE(noteModel.data(noteModel.index(0, 0), NoteModel::DateStringRole).toString(),
             QStringLiteral("Saturday, May 2, 2026"));
    QCOMPARE(navigationModel.data(navigationModel.index(0, 0), NavigationModel::NavTextRole).toString(),
             QStringLiteral("2026"));
    QCOMPARE(navigationModel.data(navigationModel.index(1, 0), NavigationModel::NavTextRole).toString(),
             QStringLiteral("May"));

    QVERIFY(localization.setLanguage(QStringLiteral("zh-Hant")));
    QCOMPARE(noteModel.data(noteModel.index(0, 0), NoteModel::DateStringRole).toString(),
             QStringLiteral("2026年5月2日 星期六"));
    QCOMPARE(navigationModel.data(navigationModel.index(0, 0), NavigationModel::NavTextRole).toString(),
             QStringLiteral("2026年"));
    QCOMPARE(navigationModel.data(navigationModel.index(1, 0), NavigationModel::NavTextRole).toString(),
             QStringLiteral("05月"));
}
```

- [ ] **Step 2: Run tests to verify failure**

Run:

```powershell
tools\test-msvc-qt.cmd
```

Expected: constructors or localized refresh behavior are missing.

- [ ] **Step 3: Inject Localization into NoteController**

Update constructor signature:

```cpp
NoteController(NoteStorage *storage,
               SummaryService *summaryService,
               NoteModel *noteModel,
               NavigationModel *navigationModel,
               AppSettings *appSettings = nullptr,
               Localization *localization = nullptr,
               QObject *parent = nullptr);
```

Add `Localization *m_localization = nullptr;` and helper:

```cpp
QString statusText(QStringView key) const;
```

Replace hard-coded statuses with:

```cpp
setStatusMessage(statusText(u"status.aiUpdated"));
setStatusMessage(statusText(u"status.todayOpened"));
setStatusMessage(statusText(u"status.autosaving"));
setStatusMessage(statusText(u"status.aiGenerating"));
setStatusMessage(allSaved ? statusText(u"status.autosaved") : statusText(u"status.partialSaveFailed"));
```

- [ ] **Step 4: Inject Localization into NavigationModel**

Update constructor:

```cpp
explicit NavigationModel(NoteStorage *storage, Localization *localization = nullptr, QObject *parent = nullptr);
```

Connect language changes:

```cpp
if (m_localization) {
    connect(m_localization, &Localization::languageChanged, this, &NavigationModel::refresh);
}
```

Replace year/month labels:

```cpp
const QString yearLabel = m_localization ? m_localization->formatYearLabel(date.year()) : QString::number(date.year());
const QString monthLabel = m_localization ? m_localization->formatMonthLabel(date.month()) : QLocale(QLocale::English).monthName(date.month(), QLocale::LongFormat);
```

- [ ] **Step 5: Inject Localization into NoteModel**

Update constructor:

```cpp
explicit NoteModel(NoteStorage *storage,
                   SummaryService *summaryService,
                   Localization *localization = nullptr,
                   QObject *parent = nullptr);
```

Connect language changes to emit date string changes:

```cpp
if (m_localization) {
    connect(m_localization, &Localization::languageChanged, this, [this]() {
        if (!m_notes.isEmpty()) {
            emit dataChanged(index(0, 0), index(m_notes.size() - 1, 0), {DateStringRole});
        }
    });
}
```

Replace `formattedDate()` with:

```cpp
return m_localization ? m_localization->formatDateTitle(date)
                      : QLocale(QLocale::English).toString(date, QStringLiteral("dddd, MMMM d, yyyy"));
```

- [ ] **Step 6: Update old model expectations**

Update tests that expect Chinese year/month labels to default English:

```cpp
QCOMPARE(model.data(model.index(0, 0), navTextRole).toString(), QStringLiteral("2026"));
QCOMPARE(model.data(model.index(1, 0), navTextRole).toString(), QStringLiteral("May"));
```

- [ ] **Step 7: Run tests to verify pass**

Run:

```powershell
tools\test-msvc-qt.cmd
```

Expected: controller/model localization tests pass.

- [ ] **Step 8: Commit**

```powershell
git add src/NoteController.h src/NoteController.cpp src/NavigationModel.h src/NavigationModel.cpp src/NoteModel.h src/NoteModel.cpp tests/tst_abcnote.cpp
git commit -m "Localize note and navigation models"
```

---

### Task 4: Wire Runtime Services and Localize QML

**Files:**
- Modify: `src/main.cpp`
- Modify: `Layout/Main.qml`
- Modify: `Layout/NoteItemDelegate.qml`

- [ ] **Step 1: Wire Localization in main.cpp**

Include the header:

```cpp
#include "Localization.h"
```

Create a shared settings path and instantiate services:

```cpp
const QString settingsFilePath = QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("ABCNote.ini"));
const QString applicationDirPath = QCoreApplication::applicationDirPath();

Localization localization(settingsFilePath);
AppSettings appSettings(settingsFilePath, applicationDirPath, &localization);
SummaryService summaryService(&localization);
NoteModel noteModel(&storage, &summaryService, &localization);
NavigationModel navigationModel(&storage, &localization);
NoteController noteController(&storage, &summaryService, &noteModel, &navigationModel, &appSettings, &localization);
```

Expose:

```cpp
engine.rootContext()->setContextProperty(QStringLiteral("i18n"), &localization);
```

- [ ] **Step 2: Replace Main.qml strings**

Examples:

```qml
ToolTip.text: i18n.t("settings.title")
text: i18n.t("about.title")
text: i18n.t("about.close")
text: i18n.t("settings.dataDirectory")
text: i18n.t("settings.changeDirectory")
title: i18n.t("dialog.selectDataDirectory")
text: i18n.t("notes.dateNotes")
```

Add language selector near the Settings heading:

```qml
Label {
    text: i18n.t("settings.language")
    font.pixelSize: 13
    font.bold: true
    color: "#555555"
    Layout.fillWidth: true
}

ComboBox {
    id: languageCombo
    model: i18n.languages
    textRole: "nativeName"
    valueRole: "code"
    currentIndex: i18n.languageIndex()
    Layout.fillWidth: true

    onActivated: function(index) {
        if (index >= 0 && index < model.length) {
            i18n.setLanguage(model[index].code)
        }
    }

    Connections {
        target: i18n
        function onLanguageChanged() {
            languageCombo.currentIndex = i18n.languageIndex()
        }
    }
}
```

Set layout direction:

```qml
LayoutMirroring.enabled: i18n.rightToLeft
LayoutMirroring.childrenInherit: true
```

- [ ] **Step 3: Replace NoteItemDelegate.qml strings**

Replace:

```qml
placeholderText: i18n.t("notes.bodyPlaceholder")
text: i18n.t("notes.aiSummary")
text: i18n.t("notes.refresh")
```

- [ ] **Step 4: Build after QML changes**

Run:

```powershell
tools\test-msvc-qt.cmd
```

Expected: build succeeds and tests pass. This command validates C++ compilation, not QML runtime rendering.

- [ ] **Step 5: Commit**

```powershell
git add src/main.cpp Layout/Main.qml Layout/NoteItemDelegate.qml
git commit -m "Add language selector to settings"
```

---

### Task 5: Final Verification

**Files:**
- No planned code edits.

- [ ] **Step 1: Run full tests**

Run:

```powershell
tools\test-msvc-qt.cmd
```

Expected: CTest exits with code 0 and reports `100% tests passed`.

- [ ] **Step 2: Build app target if tests do not build it**

Run:

```powershell
D:\Program\dev\CMake\bin\cmake.exe --build build-msvc-qt3 --config Debug --target ABCNote
```

Expected: command exits with code 0.

- [ ] **Step 3: Inspect source for remaining default Chinese runtime strings**

Run:

```powershell
rg -n "[\p{Han}]|\\u[0-9a-fA-F]{4}" src Layout
```

Expected: remaining Chinese text appears only inside `Localization.cpp` translations or comments explaining translations. No hard-coded default Chinese runtime strings remain outside the localization table.

- [ ] **Step 4: Check git status**

Run:

```powershell
git status --short
```

Expected: only intentional changes from this feature are present. Do not revert unrelated pre-existing user files.

- [ ] **Step 5: Final commit if needed**

If Task 5 required small fixes:

```powershell
git add <fixed-files>
git commit -m "Verify multilingual settings"
```

---

## Self-Review

Spec coverage:

- English default: Task 1 tests and `Localization` default.
- Nine supported languages: Task 1 metadata and translation coverage test.
- Settings selector: Task 4 QML.
- QML text: Task 4.
- C++ status/error text: Tasks 2 and 3.
- Date/navigation labels: Task 3.
- AI prompt/errors: Task 2.
- Persistence: Task 1.
- Arabic RTL: Task 1 helper and Task 4 layout binding.
- Runtime data boundaries: covered by tests and no storage changes.

Placeholder scan: no task uses unresolved placeholders. Code snippets use concrete names and paths.

Type consistency: `Localization`, `i18n`, `language`, `languages`, `rightToLeft`, `languageIndex()`, `t()`, and constructor signatures are consistent across tasks.
