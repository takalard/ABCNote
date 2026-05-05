#pragma once

#include <QAbstractListModel>
#include <QDate>
#include <QSet>

class NoteStorage;
class Localization;

// Exposes the left-side year/month/day navigation rows to QML.
class NavigationModel : public QAbstractListModel
{
    Q_OBJECT

public:
    // QML role ids for displaying, indenting, and expanding date tree rows.
    enum Role {
        // Text shown in the navigation delegate.
        DisplayRole = Qt::UserRole + 1,

        // QML-safe text role used to avoid ItemDelegate.display property conflicts.
        NavTextRole,

        // Tree depth: 0 for year, 1 for month, 2 for day.
        LevelRole,

        // ISO date for clickable day rows; empty for year and month rows.
        DateRole,

        // True for year and month rows that can be toggled.
        ExpandableRole,

        // True when a year or month group is currently expanded.
        ExpandedRole
    };

    // Creates a navigation model backed by the shared note storage.
    explicit NavigationModel(NoteStorage *storage, Localization *localization = nullptr, QObject *parent = nullptr);

    // Backward-compatible constructor for callers that only pass a QObject parent.
    explicit NavigationModel(NoteStorage *storage, QObject *parent);

    // Returns the number of flattened visible tree rows.
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    // Returns display text, level, date, and expansion state for one navigation row.
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    // Publishes role names so QML can use display, level, date, expandable, and expanded.
    QHash<int, QByteArray> roleNames() const override;

    // Rebuilds the flattened navigation rows by scanning existing non-empty note files.
    Q_INVOKABLE void refresh();

    // Toggles the expanded state for a visible year or month row.
    Q_INVOKABLE void toggleExpanded(int row);

private:
    // One flattened navigation row in the year/month/day tree.
    struct Row
    {
        // Text rendered in the left navigation list.
        QString display;

        // Indentation level for the row.
        int level = 0;

        // Click target date for day rows; invalid for year and month rows.
        QDate date;

        // Stable expansion key for year/month rows.
        QString key;

        // True for year/month rows that can hide children.
        bool expandable = false;
    };

    // Returns whether the key is expanded, defaulting unseen groups to expanded.
    bool isExpanded(const QString &key) const;

    // Builds a localized label for a year row.
    QString yearLabel(int year) const;

    // Builds a localized label for a month row.
    QString monthLabel(int month) const;

    // Returns a stable key for a year row.
    QString yearKey(int year) const;

    // Returns a stable key for a month row.
    QString monthKey(const QDate &date) const;

    // Non-owning pointer to the storage service owned by main.cpp or tests.
    NoteStorage *m_storage = nullptr;

    // Non-owning pointer to localization for translated labels.
    Localization *m_localization = nullptr;

    // Flattened visible rows shown by the QML ListView.
    QList<Row> m_rows;

    // Keys explicitly collapsed by the user.
    QSet<QString> m_collapsedKeys;
};
