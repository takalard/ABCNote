#include "NoteController.h"

#include "AppSettings.h"
#include "Localization.h"
#include "NavigationModel.h"
#include "NoteModel.h"
#include "NoteStorage.h"
#include "SummaryService.h"

#include <QDateTime>

NoteController::NoteController(NoteStorage *storage,
                               SummaryService *summaryService,
                               NoteModel *noteModel,
                               NavigationModel *navigationModel,
                               AppSettings *appSettings,
                               Localization *localization,
                               QObject *parent)
    // QObject keeps the parent for Qt object ownership.
    : QObject(parent)
    // m_storage is non-owning because main.cpp or tests own the storage service lifetime.
    , m_storage(storage)
    // m_summaryService is non-owning because main.cpp or tests own the summary service lifetime.
    , m_summaryService(summaryService)
    // m_appSettings is non-owning because main.cpp owns user preferences.
    , m_appSettings(appSettings)
    // m_localization is non-owning because main.cpp or tests own language state.
    , m_localization(localization)
    // m_noteModel is non-owning because main.cpp or tests own the model lifetime.
    , m_noteModel(noteModel)
    // m_navigationModel is non-owning because main.cpp or tests own the model lifetime.
    , m_navigationModel(navigationModel)
{
    // m_saveTimer waits briefly after edits so typing does not write every keystroke.
    m_saveTimer.setInterval(800);
    m_saveTimer.setSingleShot(true);

    // The timeout commits any dirty notes that accumulated during typing.
    connect(&m_saveTimer, &QTimer::timeout, this, &NoteController::flush);

    if (m_summaryService) {
        connect(m_summaryService, &SummaryService::summaryReady, this, [this](const QString &noteId, const QString &summary) {
            const QDate date = parseDate(noteId);
            if (!date.isValid()) {
                return;
            }

            if (m_noteModel->updateSummary(date, summary)) {
                markDirty(date);
                flush();
                setStatusMessage(statusText(u"status.aiUpdated"));
            }
        });

        connect(m_summaryService, &SummaryService::summaryFailed, this, [this](const QString &, const QString &message) {
            setStatusMessage(message);
        });
    }
}

NoteController::NoteController(NoteStorage *storage,
                               SummaryService *summaryService,
                               NoteModel *noteModel,
                               NavigationModel *navigationModel,
                               AppSettings *appSettings,
                               QObject *parent)
    : NoteController(storage, summaryService, noteModel, navigationModel, appSettings, nullptr, parent)
{
}

QString NoteController::statusMessage() const
{
    // m_statusMessage is the cached value exposed to QML.
    return m_statusMessage;
}

void NoteController::start(const QDate &date)
{
    // Load a small scroll window centered on the startup date.
    m_noteModel->initializeAroundDate(date, 3);

    // Rebuild navigation so the newly created date appears on the left.
    m_navigationModel->refresh();
    setStatusMessage(statusText(u"status.todayOpened"));

    // index is the startup date row inside the loaded note window.
    const int index = m_noteModel->indexOfDate(date.toString(Qt::ISODate));
    if (index >= 0) {
        emit scrollToIndexRequested(index);
    }
}

void NoteController::updateNoteBody(const QString &noteId, const QString &contentBody)
{
    // date is parsed from the QML-facing note id.
    const QDate date = parseDate(noteId);
    if (!date.isValid()) {
        return;
    }

    if (m_noteModel->updateBody(date, contentBody)) {
        // Dirty tracking is separate from model updates so saves can be debounced.
        markDirty(date);
        setStatusMessage(statusText(u"status.autosaving"));
    }
}

void NoteController::refreshSummary(const QString &noteId)
{
    // date is parsed from the QML-facing note id.
    const QDate date = parseDate(noteId);
    if (!date.isValid()) {
        return;
    }

    // note is copied from the model so the generator sees the latest body text.
    const Note note = m_noteModel->noteForDate(date);

    SummaryService::RequestSettings settings;
    if (m_appSettings) {
        settings.enabled = m_appSettings->aiSummaryEnabled();
        settings.baseUrl = m_appSettings->aiBaseUrl();
        settings.model = m_appSettings->aiModel();
        settings.apiKey = m_appSettings->aiApiKey();
    }

    setStatusMessage(statusText(u"status.aiGenerating"));
    m_summaryService->requestSummary(noteId, note.contentBody, settings);
}

void NoteController::flush()
{
    if (m_dirtyDates.isEmpty()) {
        // Nothing changed since the last save.
        return;
    }

    // dates copies the dirty set so saved dates can be removed during iteration.
    const QList<QDate> dates = m_dirtyDates.values();

    // allSaved tracks whether the status message should report a partial failure.
    bool allSaved = true;
    for (const QDate &date : dates) {
        // note is the latest model copy for the dirty date.
        Note note = m_noteModel->noteForDate(date);
        note.updatedAt = QDateTime::currentDateTime();

        if (!m_storage->save(note)) {
            allSaved = false;
            continue;
        }

        // Remove only dates that were actually written successfully.
        m_dirtyDates.remove(date);
    }

    // A new note file may have appeared, so rebuild the left navigation.
    m_navigationModel->refresh();
    setStatusMessage(allSaved
                         ? statusText(u"status.autosaved")
                         : statusText(u"status.partialSaveFailed"));
}

void NoteController::loadPreviousDays(int count)
{
    // Refresh the filtered note stream without creating empty calendar pages.
    m_noteModel->loadPreviousDays(count);

    // Navigation still refreshes in case persisted notes changed before this scroll event.
    m_navigationModel->refresh();
}

void NoteController::loadNextDays(int count)
{
    // Refresh the filtered note stream without creating empty calendar pages.
    m_noteModel->loadNextDays(count);

    // Navigation still refreshes in case persisted notes changed before this scroll event.
    m_navigationModel->refresh();
}

void NoteController::jumpToDate(const QString &isoDate)
{
    // date is the navigation target requested by QML.
    const QDate date = parseDate(isoDate);
    if (!date.isValid()) {
        return;
    }

    // row is looked up first in the current visible window.
    int row = m_noteModel->indexOfDate(isoDate);
    if (row < 0) {
        // Reload around the target if it is outside the current window.
        m_noteModel->initializeAroundDate(date, 3);
        m_navigationModel->refresh();
        row = m_noteModel->indexOfDate(isoDate);
    }

    if (row >= 0) {
        emit scrollToIndexRequested(row);
    }
}

void NoteController::reloadCurrentWindow(const QDate &date)
{
    // Rebuild both models against the storage root currently configured in NoteStorage.
    m_noteModel->initializeAroundDate(date, 3);
    m_navigationModel->refresh();

    const int index = m_noteModel->indexOfDate(date.toString(Qt::ISODate));
    if (index >= 0) {
        emit scrollToIndexRequested(index);
    }
}

void NoteController::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message) {
        // Avoid redundant QML property notifications.
        return;
    }

    // m_statusMessage stores the toolbar text until the next state change.
    m_statusMessage = message;
    emit statusMessageChanged();
}

QString NoteController::statusText(QStringView key) const
{
    return m_localization ? m_localization->text(key) : Localization::englishText(key);
}

void NoteController::markDirty(const QDate &date)
{
    // m_dirtyDates keeps one entry per edited date no matter how many keystrokes occur.
    m_dirtyDates.insert(date);

    // Restarting the timer implements debounce behavior while the user is typing.
    m_saveTimer.start();
}

QDate NoteController::parseDate(const QString &noteId) const
{
    // noteId is currently the ISO date string exposed by NoteModel::NoteIdRole.
    return QDate::fromString(noteId, Qt::ISODate);
}
