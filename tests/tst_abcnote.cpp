#include <QtTest/QtTest>

#include <QDir>
#include <QFile>
#include <QSignalSpy>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTemporaryDir>

#include "AppSettings.h"
#include "BuildInfo.h"
#include "Localization.h"
#include "NavigationModel.h"
#include "NoteController.h"
#include "NoteModel.h"
#include "NoteStorage.h"
#include "SummaryService.h"

// Verifies ABCNote's storage, summary, model, and controller behavior.
class ABCNoteTests : public QObject
{
    Q_OBJECT

private slots:
    // Verifies the required data/YYYY/MM/YYYY-MM-DD.json path shape.
    void storageMapsDatesToThreeLevelPaths();

    // Verifies that first opening a date returns a temporary note without creating a file.
    void storageLoadsMissingNotesAsTemporary();

    // Verifies that JSON persistence round-trips note fields.
    void storageSavesAndLoadsJson();

    // Verifies that saving empty notes does not leave persisted JSON files behind.
    void storageDeletesEmptyNotes();

    // Verifies that legacy empty JSON files are ignored by date discovery.
    void storageExistingDatesIgnoresEmptyJsonFiles();

    // Verifies missing AI configuration refuses to send summary requests.
    void summaryServiceRejectsMissingConfiguration();

    // Verifies AI summary requests use the OpenAI-compatible chat completions shape.
    void summaryServiceCallsOpenAiCompatibleEndpoint();

    // Verifies SummaryService validation messages use the selected language.
    void summaryServiceValidationMessagesUseLocalization();

    // Verifies localization defaults to English and exposes all supported languages.
    void localizationDefaultsToEnglishAndListsLanguages();

    // Verifies official release metadata exposed to the app.
    void buildInfoExposesReleaseVersion();

    // Verifies localization persists the selected language.
    void localizationPersistsLanguageSelection();

    // Verifies unsupported language codes are rejected without changing state.
    void localizationRejectsUnsupportedLanguage();

    // Verifies every supported language has translations for registered keys.
    void localizationAllSupportedLanguagesCoverRegisteredKeys();

    // Verifies localized date, year, and month formatting.
    void localizationFormatsDatesAndNavigationLabels();

    // Verifies the continuous note stream loads previous/current/next dates.
    void noteModelLoadsDateWindow();

    // Verifies controller toolbar statuses use localization.
    void controllerStatusMessagesUseLocalization();

    // Verifies models refresh localized date/navigation labels after language changes.
    void modelsRefreshLocalizedLabelsWhenLanguageChanges();

    // Verifies the left navigation model exposes year, month, and day levels.
    void navigationModelExposesThreeLevels();

    // Verifies the left navigation model exposes a QML-safe text role that avoids ItemDelegate conflicts.
    void navigationModelExposesQmlSafeTextRole();

    // Verifies the left navigation model can collapse and expand year/month rows.
    void navigationModelCollapsesAndExpandsGroups();

    // Verifies controller startup keeps today's empty note temporary.
    void controllerStartsTodayAsTemporary();

    // Verifies controller persists non-empty edits and deletes cleared notes.
    void controllerPersistsNonEmptyAndDeletesClearedNotes();

    // Verifies settings default to an app-adjacent data folder.
    void appSettingsDefaultsToApplicationAdjacentDataFolder();

    // Verifies settings persist a user-selected data folder.
    void appSettingsPersistsDataRoot();

    // Verifies changing the data folder copies existing note files and switches storage.
    void appSettingsMigratesFilesAndSwitchesStorage();

    // Verifies migration keeps target files when a same-name file already exists.
    void appSettingsMigrationDoesNotOverwriteExistingFiles();

    // Verifies migration failure leaves storage pointed at the old folder.
    void appSettingsMigrationFailureKeepsOldStorageRoot();

    // Verifies AppSettings status messages use the selected language.
    void appSettingsStatusMessagesUseLocalization();

    // Verifies AI summary settings persist non-secret fields and track a saved user API key.
    void appSettingsPersistsAiSummarySettings();

    // Verifies clearing the AI API key makes AI summary unavailable.
    void appSettingsClearsAiApiKey();
};

void ABCNoteTests::storageMapsDatesToThreeLevelPaths()
{
    // dir isolates this test's generated files from the real app data directory.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // storage maps dates under the temporary root.
    NoteStorage storage(dir.path());

    // date is the sample day whose final path is asserted.
    const QDate date(2026, 5, 2);

    // expected is the documented three-level path for the sample date.
    const QString expected = QDir(dir.path()).filePath("2026/05/2026-05-02.json");

    QCOMPARE(QDir::toNativeSeparators(storage.filePathForDate(date)), QDir::toNativeSeparators(expected));
}

void ABCNoteTests::storageLoadsMissingNotesAsTemporary()
{
    // dir isolates this test's generated files from the real app data directory.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // storage loads temporary notes inside the temporary root.
    NoteStorage storage(dir.path());

    // date is missing before load runs.
    const QDate date(2026, 5, 2);

    // note is expected to be returned in memory without creating a file.
    const Note note = storage.load(date);

    QCOMPARE(note.date, date);
    QVERIFY(note.contentBody.isEmpty());
    QVERIFY(note.aiSummary.isEmpty());
    QVERIFY(!QFile::exists(storage.filePathForDate(date)));
}

void ABCNoteTests::storageSavesAndLoadsJson()
{
    // dir isolates this test's generated files from the real app data directory.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // storage writes and reads the note JSON file.
    NoteStorage storage(dir.path());

    // note contains every persisted field used by the JSON schema.
    Note note;
    note.date = QDate(2026, 5, 2);
    note.contentBody = "ABCNote body";
    note.aiSummary = "ABCNote summary";
    note.updatedAt = QDateTime(QDate(2026, 5, 2), QTime(13, 14, 15));

    QVERIFY(storage.save(note));

    // loaded should match the saved content after reading from disk.
    const Note loaded = storage.load(note.date);
    QCOMPARE(loaded.date, note.date);
    QCOMPARE(loaded.contentBody, note.contentBody);
    QCOMPARE(loaded.aiSummary, note.aiSummary);
}

void ABCNoteTests::storageDeletesEmptyNotes()
{
    // dir isolates this test's generated files from the real app data directory.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // storage writes and removes the note JSON file.
    NoteStorage storage(dir.path());

    // note starts with body content so the first save creates a persisted file.
    Note note;
    note.date = QDate(2026, 5, 2);
    note.contentBody = "ABCNote body";
    QVERIFY(storage.save(note));
    QVERIFY(QFile::exists(storage.filePathForDate(note.date)));

    // emptyNote represents the user clearing the note body.
    Note emptyNote = note;
    emptyNote.contentBody.clear();
    QVERIFY(storage.save(emptyNote));
    QVERIFY(!QFile::exists(storage.filePathForDate(note.date)));
}

void ABCNoteTests::storageExistingDatesIgnoresEmptyJsonFiles()
{
    // dir isolates this test's generated files from the real app data directory.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // storage scans the temporary data tree for persisted note dates.
    NoteStorage storage(dir.path());

    // emptyNote is written manually to simulate legacy files created before temp-note persistence existed.
    const QDate emptyDate(2026, 5, 2);
    const QString emptyPath = storage.filePathForDate(emptyDate);
    QVERIFY(QDir().mkpath(QFileInfo(emptyPath).absolutePath()));
    QFile emptyFile(emptyPath);
    QVERIFY(emptyFile.open(QIODevice::WriteOnly));
    emptyFile.write(R"({
    "aiSummary": "",
    "contentBody": "",
    "date": "2026-05-02",
    "updatedAt": "2026-05-02T14:01:35"
})");
    emptyFile.close();

    // persistedNote has real user content and should remain visible in navigation discovery.
    Note persistedNote;
    persistedNote.date = QDate(2026, 5, 3);
    persistedNote.contentBody = "Real persisted note";
    QVERIFY(storage.save(persistedNote));

    // dates should include only non-empty notes so stale empty files cannot pollute navigation.
    const QList<QDate> dates = storage.existingDates();
    QVERIFY(!dates.contains(emptyDate));
    QVERIFY(dates.contains(persistedNote.date));
}

void ABCNoteTests::summaryServiceRejectsMissingConfiguration()
{
    SummaryService summary;
    QSignalSpy failedSpy(&summary, &SummaryService::summaryFailed);
    QSignalSpy readySpy(&summary, &SummaryService::summaryReady);

    SummaryService::RequestSettings settings;
    settings.enabled = false;
    settings.baseUrl = QStringLiteral("http://127.0.0.1:1/v1");
    settings.model = QStringLiteral("example-model");
    settings.apiKey = QStringLiteral("secret");

    summary.requestSummary(QStringLiteral("2026-05-03"), QStringLiteral("需要总结的内容"), settings);

    QCOMPARE(readySpy.count(), 0);
    QCOMPARE(failedSpy.count(), 1);
    QCOMPARE(failedSpy.takeFirst().at(0).toString(), QStringLiteral("2026-05-03"));
}

void ABCNoteTests::summaryServiceCallsOpenAiCompatibleEndpoint()
{
    QTcpServer server;
    QVERIFY(server.listen(QHostAddress::LocalHost));

    QByteArray capturedRequest;
    connect(&server, &QTcpServer::newConnection, &server, [&server, &capturedRequest]() {
        QTcpSocket *socket = server.nextPendingConnection();
        QObject::connect(socket, &QTcpSocket::readyRead, socket, [socket, &capturedRequest]() {
            capturedRequest += socket->readAll();
            if (!capturedRequest.contains("\r\n\r\n")) {
                return;
            }

            const QByteArray responseBody = R"({"choices":[{"message":{"content":"AI summary text"}}]})";
            socket->write("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: "
                          + QByteArray::number(responseBody.size()) + "\r\n\r\n" + responseBody);
            socket->disconnectFromHost();
        });
    });

    SummaryService summary;
    QSignalSpy readySpy(&summary, &SummaryService::summaryReady);
    QSignalSpy failedSpy(&summary, &SummaryService::summaryFailed);

    SummaryService::RequestSettings settings;
    settings.enabled = true;
    settings.baseUrl = QStringLiteral("http://127.0.0.1:%1/v1").arg(server.serverPort());
    settings.model = QStringLiteral("example-model");
    settings.apiKey = QStringLiteral("secret-key");

    summary.requestSummary(QStringLiteral("2026-05-03"), QStringLiteral("今天完成了很多工作。"), settings);

    QVERIFY(readySpy.wait(3000));
    QCOMPARE(failedSpy.count(), 0);
    const QList<QVariant> arguments = readySpy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), QStringLiteral("2026-05-03"));
    QCOMPARE(arguments.at(1).toString(), QStringLiteral("AI summary text"));
    QVERIFY(capturedRequest.contains("POST /v1/chat/completions HTTP/1.1"));
    QVERIFY(capturedRequest.toLower().contains("authorization: bearer secret-key"));
    QVERIFY(capturedRequest.contains("\"model\":\"example-model\""));
    QVERIFY(capturedRequest.contains("\u4eca\u5929\u5b8c\u6210\u4e86\u5f88\u591a\u5de5\u4f5c"));
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

void ABCNoteTests::localizationDefaultsToEnglishAndListsLanguages()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    Localization localization(QDir(dir.path()).filePath("settings.ini"));

    QCOMPARE(localization.language(), QStringLiteral("en"));
    QCOMPARE(localization.t(QStringLiteral("settings.title")), QStringLiteral("Settings"));
    QCOMPARE(localization.t(QStringLiteral("about.version")), QStringLiteral("Version"));
    QCOMPARE(localization.t(QStringLiteral("about.repository")), QStringLiteral("Repository"));
    QCOMPARE(localization.t(QStringLiteral("about.author")), QStringLiteral("Author"));
    QCOMPARE(localization.t(QStringLiteral("about.email")), QStringLiteral("Email"));
    QCOMPARE(localization.t(QStringLiteral("about.wechat")), QStringLiteral("WeChat"));
    QCOMPARE(localization.t(QStringLiteral("notes.bodyPlaceholder")), QStringLiteral("Start writing today..."));

    const QVariantList languages = localization.languages();
    QCOMPARE(languages.size(), 9);
    QCOMPARE(languages.first().toMap().value(QStringLiteral("code")).toString(), QStringLiteral("en"));
    QVERIFY(languages.first().toMap().contains(QStringLiteral("nativeName")));
    QVERIFY(languages.first().toMap().contains(QStringLiteral("rtl")));
}

void ABCNoteTests::buildInfoExposesReleaseVersion()
{
    QCOMPARE(QString::fromLatin1(BuildInfo::version()), QStringLiteral("1.0.1.20260505"));
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

void ABCNoteTests::noteModelLoadsDateWindow()
{
    // dir isolates this test's generated files from the real app data directory.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // storage supplies persisted notes to the model.
    NoteStorage storage(dir.path());
    Note previousNote;
    previousNote.date = QDate(2026, 4, 30);
    previousNote.contentBody = "Previous persisted note";
    QVERIFY(storage.save(previousNote));
    Note futureNote;
    futureNote.date = QDate(2026, 5, 4);
    futureNote.contentBody = "Future persisted note";
    QVERIFY(storage.save(futureNote));

    // summary supplies placeholder summaries for empty note summaries.
    SummaryService summary;

    // model is the continuous right-side note stream.
    NoteModel model(&storage, &summary);

    model.initializeAroundDate(QDate(2026, 5, 2), 1);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.noteAt(0).date, QDate(2026, 4, 30));
    QCOMPARE(model.noteAt(1).date, QDate(2026, 5, 2));
    QCOMPARE(model.noteAt(2).date, QDate(2026, 5, 4));

    model.loadPreviousDays(1);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.noteAt(0).date, QDate(2026, 4, 30));

    model.loadNextDays(1);
    QCOMPARE(model.rowCount(), 3);
    QCOMPARE(model.noteAt(2).date, QDate(2026, 5, 4));
}

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

void ABCNoteTests::navigationModelExposesThreeLevels()
{
    // dir isolates this test's generated files from the real app data directory.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // storage creates one existing note so navigation has rows to expose.
    NoteStorage storage(dir.path());
    Note note;
    note.date = QDate(2026, 5, 2);
    note.contentBody = "Persisted note";
    QVERIFY(storage.save(note));

    // model flattens the date tree into rows for QML.
    NavigationModel model(&storage);
    model.refresh();

    QVERIFY(model.rowCount() >= 3);
    QCOMPARE(model.data(model.index(0, 0), NavigationModel::LevelRole).toInt(), 0);
    QCOMPARE(model.data(model.index(1, 0), NavigationModel::LevelRole).toInt(), 1);
    QCOMPARE(model.data(model.index(2, 0), NavigationModel::LevelRole).toInt(), 2);
    QCOMPARE(model.data(model.index(2, 0), NavigationModel::DateRole).toString(), QStringLiteral("2026-05-02"));
}

void ABCNoteTests::navigationModelExposesQmlSafeTextRole()
{
    // dir isolates this test's generated files from the real app data directory.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // storage creates one existing note so navigation has rows to expose.
    NoteStorage storage(dir.path());
    Note note;
    note.date = QDate(2026, 5, 2);
    note.contentBody = "Persisted note";
    QVERIFY(storage.save(note));

    // model exposes role names for QML delegates.
    NavigationModel model(&storage);
    model.refresh();

    // roleNames should include navText so QML does not collide with ItemDelegate.display.
    const QHash<int, QByteArray> names = model.roleNames();
    const int navTextRole = names.key(QByteArrayLiteral("navText"), -1);
    QVERIFY(navTextRole >= 0);
    QCOMPARE(model.data(model.index(0, 0), navTextRole).toString(), QStringLiteral("2026"));
    QCOMPARE(model.data(model.index(1, 0), navTextRole).toString(), QStringLiteral("May"));
    QCOMPARE(model.data(model.index(2, 0), navTextRole).toString(), QStringLiteral("2026-05-02"));
}

void ABCNoteTests::navigationModelCollapsesAndExpandsGroups()
{
    // dir isolates this test's generated files from the real app data directory.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // storage creates two persisted notes in the same year/month group.
    NoteStorage storage(dir.path());
    Note first;
    first.date = QDate(2026, 5, 2);
    first.contentBody = "First persisted note";
    QVERIFY(storage.save(first));
    Note second;
    second.date = QDate(2026, 5, 3);
    second.contentBody = "Second persisted note";
    QVERIFY(storage.save(second));

    // model starts expanded so year, month, and day rows are visible.
    NavigationModel model(&storage);
    model.refresh();
    QCOMPARE(model.rowCount(), 4);
    QVERIFY(model.data(model.index(0, 0), NavigationModel::ExpandableRole).toBool());
    QVERIFY(model.data(model.index(1, 0), NavigationModel::ExpandableRole).toBool());
    QVERIFY(model.data(model.index(0, 0), NavigationModel::ExpandedRole).toBool());

    // Collapsing the year hides month and day rows.
    model.toggleExpanded(0);
    QCOMPARE(model.rowCount(), 1);
    QVERIFY(!model.data(model.index(0, 0), NavigationModel::ExpandedRole).toBool());

    // Expanding the year restores the visible month and day rows.
    model.toggleExpanded(0);
    QCOMPARE(model.rowCount(), 4);
    QVERIFY(model.data(model.index(0, 0), NavigationModel::ExpandedRole).toBool());

    // Collapsing the month hides day rows while keeping the year and month visible.
    model.toggleExpanded(1);
    QCOMPARE(model.rowCount(), 2);
    QVERIFY(!model.data(model.index(1, 0), NavigationModel::ExpandedRole).toBool());
}

void ABCNoteTests::controllerStartsTodayAsTemporary()
{
    // dir isolates this test's generated files from the real app data directory.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // storage is shared by the controller and models.
    NoteStorage storage(dir.path());

    // summary is shared by the controller and note model.
    SummaryService summary;

    // noteModel represents the right-side content stream.
    NoteModel noteModel(&storage, &summary);

    // navigationModel represents the left-side date tree.
    NavigationModel navigationModel(&storage);

    // controller coordinates startup, editing, summary, and save operations.
    NoteController controller(&storage, &summary, &noteModel, &navigationModel);

    // today is injected to keep the startup test aligned with the actual runtime date.
    const QDate today = QDate::currentDate();
    controller.start(today);
    QVERIFY(!QFile::exists(storage.filePathForDate(today)));
    QCOMPARE(navigationModel.rowCount(), 0);
}

void ABCNoteTests::controllerPersistsNonEmptyAndDeletesClearedNotes()
{
    // dir isolates this test's generated files from the real app data directory.
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    // storage is shared by the controller and models.
    NoteStorage storage(dir.path());

    // summary is shared by the controller and note model.
    SummaryService summary;

    // noteModel represents the right-side content stream.
    NoteModel noteModel(&storage, &summary);

    // navigationModel represents the left-side date tree.
    NavigationModel navigationModel(&storage);

    // controller coordinates startup, editing, summary, and save operations.
    NoteController controller(&storage, &summary, &noteModel, &navigationModel);

    // today is injected to keep the startup test aligned with the actual runtime date.
    const QDate today = QDate::currentDate();
    controller.start(today);

    // bodyText is the edit that should be persisted by flush.
    const QString bodyText = QStringLiteral("\u4eca\u5929\u5b8c\u6210 ABCNote \u5de5\u7a0b\u642d\u5efa\u3002");
    controller.updateNoteBody(today.toString(Qt::ISODate), bodyText);
    controller.flush();

    // loaded verifies the final persisted JSON state.
    const Note loaded = storage.load(today);
    QCOMPARE(loaded.contentBody, bodyText);
    QVERIFY(loaded.aiSummary.isEmpty());
    QVERIFY(QFile::exists(storage.filePathForDate(today)));
    QVERIFY(navigationModel.rowCount() >= 3);

    // Clearing the body should remove the persisted file and navigation entry.
    controller.updateNoteBody(today.toString(Qt::ISODate), QString());
    controller.flush();
    QVERIFY(!QFile::exists(storage.filePathForDate(today)));
    QCOMPARE(navigationModel.rowCount(), 0);
}

void ABCNoteTests::appSettingsDefaultsToApplicationAdjacentDataFolder()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString settingsPath = QDir(dir.path()).filePath("settings.ini");
    const QString appDir = QDir(dir.path()).filePath("app");
    QVERIFY(QDir().mkpath(appDir));

    AppSettings settings(settingsPath, appDir);

    const QString expected = QDir(appDir).filePath("data");
    QCOMPARE(QDir::toNativeSeparators(settings.dataRoot()), QDir::toNativeSeparators(expected));
}

void ABCNoteTests::appSettingsPersistsDataRoot()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString settingsPath = QDir(dir.path()).filePath("settings.ini");
    const QString appDir = QDir(dir.path()).filePath("app");
    const QString customRoot = QDir(dir.path()).filePath("custom-data");
    QVERIFY(QDir().mkpath(appDir));

    AppSettings first(settingsPath, appDir);
    QVERIFY(first.setDataRootForTesting(customRoot));

    AppSettings second(settingsPath, appDir);
    QCOMPARE(QDir::toNativeSeparators(second.dataRoot()), QDir::toNativeSeparators(customRoot));
}

void ABCNoteTests::appSettingsMigratesFilesAndSwitchesStorage()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString oldRoot = QDir(dir.path()).filePath("old-data");
    const QString newRoot = QDir(dir.path()).filePath("new-data");
    NoteStorage storage(oldRoot);

    Note note;
    note.date = QDate(2026, 5, 3);
    note.contentBody = "Migrated body";
    QVERIFY(storage.save(note));

    AppSettings settings(QDir(dir.path()).filePath("settings.ini"), QDir(dir.path()).filePath("app"));
    settings.setStorage(&storage);
    QVERIFY(settings.setDataRootForTesting(oldRoot));

    QVERIFY(settings.migrateAndSwitchDataRoot(newRoot));
    QCOMPARE(QDir::toNativeSeparators(storage.rootPath()), QDir::toNativeSeparators(newRoot));
    QVERIFY(QFile::exists(QDir(newRoot).filePath("2026/05/2026-05-03.json")));
}

void ABCNoteTests::appSettingsMigrationDoesNotOverwriteExistingFiles()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString oldRoot = QDir(dir.path()).filePath("old-data");
    const QString newRoot = QDir(dir.path()).filePath("new-data");

    NoteStorage oldStorage(oldRoot);
    Note oldNote;
    oldNote.date = QDate(2026, 5, 3);
    oldNote.contentBody = "Old body";
    QVERIFY(oldStorage.save(oldNote));

    NoteStorage newStorage(newRoot);
    Note targetNote;
    targetNote.date = oldNote.date;
    targetNote.contentBody = "Target body";
    QVERIFY(newStorage.save(targetNote));

    AppSettings settings(QDir(dir.path()).filePath("settings.ini"), QDir(dir.path()).filePath("app"));
    settings.setStorage(&oldStorage);
    QVERIFY(settings.setDataRootForTesting(oldRoot));

    QVERIFY(settings.migrateAndSwitchDataRoot(newRoot));
    QCOMPARE(newStorage.load(targetNote.date).contentBody, targetNote.contentBody);
    QVERIFY(settings.statusMessage().contains(QStringLiteral("already existed")));
}

void ABCNoteTests::appSettingsMigrationFailureKeepsOldStorageRoot()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString oldRoot = QDir(dir.path()).filePath("old-data");
    const QString blockedRoot = QDir(dir.path()).filePath("blocked");

    NoteStorage storage(oldRoot);
    Note note;
    note.date = QDate(2026, 5, 3);
    note.contentBody = "Body";
    QVERIFY(storage.save(note));

    QFile blocker(blockedRoot);
    QVERIFY(blocker.open(QIODevice::WriteOnly));
    blocker.write("not a directory");
    blocker.close();

    AppSettings settings(QDir(dir.path()).filePath("settings.ini"), QDir(dir.path()).filePath("app"));
    settings.setStorage(&storage);
    QVERIFY(settings.setDataRootForTesting(oldRoot));

    QVERIFY(!settings.migrateAndSwitchDataRoot(blockedRoot));
    QCOMPARE(QDir::toNativeSeparators(storage.rootPath()), QDir::toNativeSeparators(oldRoot));
    QCOMPARE(QDir::toNativeSeparators(settings.dataRoot()), QDir::toNativeSeparators(oldRoot));
}

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
    QCOMPARE(settings.statusMessage(), QStringLiteral("Réglages IA enregistrés"));
}

void ABCNoteTests::appSettingsPersistsAiSummarySettings()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString settingsPath = QDir(dir.path()).filePath("settings.ini");
    const QString appDir = QDir(dir.path()).filePath("app");
    QVERIFY(QDir().mkpath(appDir));

    AppSettings first(settingsPath, appDir);
    QVERIFY(first.saveAiSummarySettings(true,
                                        QStringLiteral("https://api.example.com/v1"),
                                        QStringLiteral("example-model"),
                                        QStringLiteral("abc-secret-key")));
    QVERIFY(first.aiSummaryEnabled());
    QCOMPARE(first.aiBaseUrl(), QStringLiteral("https://api.example.com/v1"));
    QCOMPARE(first.aiModel(), QStringLiteral("example-model"));
    QVERIFY(first.hasAiApiKey());
    QCOMPARE(first.aiApiKey(), QStringLiteral("abc-secret-key"));

    AppSettings second(settingsPath, appDir);
    QVERIFY(second.aiSummaryEnabled());
    QCOMPARE(second.aiBaseUrl(), QStringLiteral("https://api.example.com/v1"));
    QCOMPARE(second.aiModel(), QStringLiteral("example-model"));
    QVERIFY(second.hasAiApiKey());
    QCOMPARE(second.aiApiKey(), QStringLiteral("abc-secret-key"));

    QVERIFY(second.clearAiApiKey());
}

void ABCNoteTests::appSettingsClearsAiApiKey()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    const QString settingsPath = QDir(dir.path()).filePath("settings.ini");
    const QString appDir = QDir(dir.path()).filePath("app");
    QVERIFY(QDir().mkpath(appDir));

    AppSettings settings(settingsPath, appDir);
    QVERIFY(settings.saveAiSummarySettings(true,
                                           QStringLiteral("https://api.example.com/v1"),
                                           QStringLiteral("example-model"),
                                           QStringLiteral("abc-secret-key")));
    QVERIFY(settings.hasAiApiKey());

    QVERIFY(settings.clearAiApiKey());
    QVERIFY(!settings.hasAiApiKey());
    QVERIFY(settings.aiApiKey().isEmpty());
}

QTEST_MAIN(ABCNoteTests)

#include "tst_abcnote.moc"
