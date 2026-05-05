#pragma once

#include <QDate>
#include <QObject>
#include <QSet>
#include <QTimer>

class NavigationModel;
class NoteModel;
class NoteStorage;
class SummaryService;
class AppSettings;
class Localization;

// Coordinates startup, QML actions, autosave, summary refresh, and scrolling.
class NoteController : public QObject
{
    Q_OBJECT

    // Short user-facing save/open status shown in the top toolbar.
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

public:
    // Creates a controller over shared non-owning service/model pointers.
    explicit NoteController(NoteStorage *storage,
                            SummaryService *summaryService,
                            NoteModel *noteModel,
                            NavigationModel *navigationModel,
                            AppSettings *appSettings = nullptr,
                            Localization *localization = nullptr,
                            QObject *parent = nullptr);

    // Backward-compatible constructor for callers that do not inject localization.
    NoteController(NoteStorage *storage,
                   SummaryService *summaryService,
                   NoteModel *noteModel,
                   NavigationModel *navigationModel,
                   AppSettings *appSettings,
                   QObject *parent);

    // Returns the current toolbar status message.
    QString statusMessage() const;

    // Opens the requested date, creates it if missing, and centers the visible note window around it.
    Q_INVOKABLE void start(const QDate &date = QDate::currentDate());

    // Applies body text edits from QML and schedules an autosave.
    Q_INVOKABLE void updateNoteBody(const QString &noteId, const QString &contentBody);

    // Regenerates the local summary for one note and schedules an autosave.
    Q_INVOKABLE void refreshSummary(const QString &noteId);

    // Writes all dirty notes to disk immediately.
    Q_INVOKABLE void flush();

    // Loads earlier dates when the content list reaches the top.
    Q_INVOKABLE void loadPreviousDays(int count = 3);

    // Loads later dates when the content list reaches the bottom.
    Q_INVOKABLE void loadNextDays(int count = 3);

    // Jumps the content list to a date selected in the navigation.
    Q_INVOKABLE void jumpToDate(const QString &isoDate);

    // Reloads the note stream and navigation after the storage root changes.
    void reloadCurrentWindow(const QDate &date = QDate::currentDate());

signals:
    // Emitted whenever statusMessage changes.
    void statusMessageChanged();

    // Requests the QML content list to position itself at a model row.
    void scrollToIndexRequested(int index);

private:
    // Updates the status message and emits the property notification.
    void setStatusMessage(const QString &message);

    // Resolves a translated status message, falling back to English when localization is absent.
    QString statusText(QStringView key) const;

    // Marks one date as dirty and starts the debounce save timer.
    void markDirty(const QDate &date);

    // Parses a note id into a QDate; note ids are ISO date strings.
    QDate parseDate(const QString &noteId) const;

    // Non-owning pointer to note file storage.
    NoteStorage *m_storage = nullptr;

    // Non-owning pointer to the local summary generator.
    SummaryService *m_summaryService = nullptr;

    // Non-owning pointer to user AI settings.
    AppSettings *m_appSettings = nullptr;

    // Non-owning pointer to localization for toolbar status text.
    Localization *m_localization = nullptr;

    // Non-owning pointer to the right-side note stream model.
    NoteModel *m_noteModel = nullptr;

    // Non-owning pointer to the left-side date navigation model.
    NavigationModel *m_navigationModel = nullptr;

    // Debounce timer that delays disk writes while the user is typing.
    QTimer m_saveTimer;

    // Set of dates with unsaved changes.
    QSet<QDate> m_dirtyDates;

    // Cached value for the statusMessage QML property.
    QString m_statusMessage;
};
