#include "NoteModel.h"

#include "Localization.h"
#include "NoteStorage.h"
#include "SummaryService.h"

#include <QDateTime>
#include <QLocale>
#include <algorithm>

NoteModel::NoteModel(NoteStorage *storage,
                     SummaryService *summaryService,
                     Localization *localization,
                     QObject *parent)
    // QAbstractListModel keeps the parent for Qt object ownership.
    : QAbstractListModel(parent)
    // m_storage is non-owning because main.cpp or tests own the storage service lifetime.
    , m_storage(storage)
    // m_summaryService is non-owning because main.cpp or tests own the summary service lifetime.
    , m_summaryService(summaryService)
    // m_localization is non-owning because main.cpp or tests own language state.
    , m_localization(localization)
{
    if (m_localization) {
        connect(m_localization, &Localization::languageChanged, this, [this]() {
            if (!m_notes.isEmpty()) {
                emit dataChanged(index(0, 0), index(m_notes.size() - 1, 0), {DateStringRole});
            }
        });
    }
}

NoteModel::NoteModel(NoteStorage *storage, SummaryService *summaryService, QObject *parent)
    : NoteModel(storage, summaryService, nullptr, parent)
{
}

int NoteModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        // This is a flat list model, so child indexes never have rows.
        return 0;
    }

    // m_notes is the currently loaded chronological date window.
    return m_notes.size();
}

QVariant NoteModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_notes.size()) {
        // Invalid indexes return an empty QVariant as Qt models expect.
        return {};
    }

    // note is the row object that supplies all QML role values.
    const Note &note = m_notes.at(index.row());
    switch (role) {
    case NoteIdRole:
    case DateRole:
        // The ISO date is both a display-safe date value and a stable note id.
        return note.date.toString(Qt::ISODate);
    case DateStringRole:
        // The formatted title is localized for the note page header.
        return formattedDate(note.date);
    case ContentBodyRole:
        return note.contentBody;
    case AiSummaryRole:
        return note.aiSummary;
    default:
        return {};
    }
}

QHash<int, QByteArray> NoteModel::roleNames() const
{
    // Role names are the public contract used by QML delegates.
    return {
        {NoteIdRole, "noteId"},
        {DateRole, "date"},
        {DateStringRole, "dateString"},
        {ContentBodyRole, "contentBody"},
        {AiSummaryRole, "aiSummary"},
    };
}

void NoteModel::initializeAroundDate(const QDate &date, int radiusDays)
{
    Q_UNUSED(radiusDays);

    // Reset because the full visible window is being replaced.
    beginResetModel();
    m_notes.clear();

    // dates starts with persisted non-empty note dates discovered by storage.
    QList<QDate> dates = m_storage->existingDates();

    // The requested date is always visible as the current temporary or persisted page.
    if (!dates.contains(date)) {
        dates.append(date);
    }

    // Sort and deduplicate dates so the right content area remains chronological.
    std::sort(dates.begin(), dates.end());
    dates.erase(std::unique(dates.begin(), dates.end()), dates.end());

    // Load only the requested date and dates with real persisted content.
    for (const QDate &visibleDate : dates) {
        m_notes.append(loadDate(visibleDate));
    }
    endResetModel();
}

void NoteModel::loadPreviousDays(int count)
{
    Q_UNUSED(count);

    // The visible stream is intentionally limited to today/target date plus persisted non-empty notes.
}

void NoteModel::loadNextDays(int count)
{
    Q_UNUSED(count);

    // The visible stream is intentionally limited to today/target date plus persisted non-empty notes.
}

int NoteModel::indexOfDate(const QString &isoDate) const
{
    // date is parsed from the QML-facing ISO note id.
    const QDate date = QDate::fromString(isoDate, Qt::ISODate);

    // row scans the currently loaded date window.
    for (int row = 0; row < m_notes.size(); ++row) {
        if (m_notes.at(row).date == date) {
            return row;
        }
    }
    return -1;
}

Note NoteModel::noteAt(int row) const
{
    if (row < 0 || row >= m_notes.size()) {
        // Return a default note for out-of-range test or controller requests.
        return {};
    }

    // Return a copy so callers cannot mutate the model without dataChanged.
    return m_notes.at(row);
}

bool NoteModel::updateBody(const QDate &date, const QString &contentBody)
{
    // row scans loaded notes until the matching date is found.
    for (int row = 0; row < m_notes.size(); ++row) {
        if (m_notes[row].date == date) {
            if (m_notes[row].contentBody == contentBody) {
                // No signal is needed when the value is unchanged.
                return true;
            }

            // Update the in-memory note before notifying QML.
            m_notes[row].contentBody = contentBody;
            m_notes[row].updatedAt = QDateTime::currentDateTime();

            // modelIndex identifies the changed row for QML bindings.
            const QModelIndex modelIndex = index(row, 0);
            emit dataChanged(modelIndex, modelIndex, {ContentBodyRole});
            return true;
        }
    }
    return false;
}

bool NoteModel::updateSummary(const QDate &date, const QString &aiSummary)
{
    // row scans loaded notes until the matching date is found.
    for (int row = 0; row < m_notes.size(); ++row) {
        if (m_notes[row].date == date) {
            // Update the summary and timestamp in memory first.
            m_notes[row].aiSummary = aiSummary;
            m_notes[row].updatedAt = QDateTime::currentDateTime();

            // modelIndex identifies the changed row for QML bindings.
            const QModelIndex modelIndex = index(row, 0);
            emit dataChanged(modelIndex, modelIndex, {AiSummaryRole});
            return true;
        }
    }
    return false;
}

Note NoteModel::noteForDate(const QDate &date) const
{
    // note is copied from the current model window when already loaded.
    for (const Note &note : m_notes) {
        if (note.date == date) {
            return note;
        }
    }

    // Load from storage for dates outside the current visible range.
    return loadDate(date);
}

QString NoteModel::formattedDate(const QDate &date) const
{
    if (m_localization) {
        return m_localization->formatDateTitle(date);
    }
    return QLocale(QLocale::English, QLocale::UnitedStates).toString(date, QLocale::LongFormat);
}

Note NoteModel::loadDate(const QDate &date) const
{
    // note is loaded from disk or returned as an in-memory temporary note.
    return m_storage->load(date);
}
