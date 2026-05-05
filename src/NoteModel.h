#pragma once

#include "Note.h"

#include <QAbstractListModel>

class NoteStorage;
class SummaryService;
class Localization;

// Exposes the continuous chronological note stream to QML.
class NoteModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // QML role ids for each field consumed by NoteItemDelegate.qml.
    enum Role {
        // Stable string id for update calls; currently the ISO date.
        NoteIdRole = Qt::UserRole + 1,

        // ISO date string used by navigation and debug views.
        DateRole,

        // Human-readable localized date title.
        DateStringRole,

        // User-editable body text.
        ContentBodyRole,

        // Generated summary text.
        AiSummaryRole
    };

    // Creates a model backed by the shared storage and summary service.
    explicit NoteModel(NoteStorage *storage,
                       SummaryService *summaryService,
                       Localization *localization = nullptr,
                       QObject *parent = nullptr);

    // Backward-compatible constructor for callers that only pass a QObject parent.
    explicit NoteModel(NoteStorage *storage, SummaryService *summaryService, QObject *parent);

    // Returns the number of note rows currently loaded into the scroll window.
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // Returns a role value for the requested note row.
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Publishes role names so QML can use noteId, date, dateString, contentBody, and aiSummary.
    QHash<int, QByteArray> roleNames() const override;

    // Loads a centered date window around the provided date.
    Q_INVOKABLE void initializeAroundDate(const QDate &date, int radiusDays = 3);

    // Prepends earlier day notes to simulate infinite upward scrolling.
    Q_INVOKABLE void loadPreviousDays(int count = 3);

    // Appends later day notes to simulate infinite downward scrolling.
    Q_INVOKABLE void loadNextDays(int count = 3);

    // Finds the current row for an ISO date string; returns -1 when not loaded.
    Q_INVOKABLE int indexOfDate(const QString &isoDate) const;

    // Returns a note copy for tests and controller persistence.
    Note noteAt(int row) const;

    // Updates the body text for a loaded date and emits QML dataChanged.
    bool updateBody(const QDate &date, const QString &contentBody);

    // Updates the summary text for a loaded date and emits QML dataChanged.
    bool updateSummary(const QDate &date, const QString &aiSummary);

    // Returns a loaded note or loads it from storage when it is outside the current window.
    Note noteForDate(const QDate &date) const;

private:
    // Formats a date as the daily page title.
    QString formattedDate(const QDate &date) const;

    // Loads one note and fills an empty summary with the local generator.
    Note loadDate(const QDate &date) const;

    // Non-owning pointer to the storage service owned by main.cpp or tests.
    NoteStorage *m_storage = nullptr;

    // Non-owning pointer to the summary service owned by main.cpp or tests.
    SummaryService *m_summaryService = nullptr;

    // Non-owning pointer to localization for date headings.
    Localization *m_localization = nullptr;

    // Current chronological window shown by the right-side ListView.
    QList<Note> m_notes;
};
