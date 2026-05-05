#include "NavigationModel.h"

#include "Localization.h"
#include "NoteStorage.h"

#include <QLocale>

NavigationModel::NavigationModel(NoteStorage *storage, Localization *localization, QObject *parent)
    // QAbstractListModel keeps the parent for Qt object ownership.
    : QAbstractListModel(parent)
    // m_storage is non-owning because main.cpp or tests own the storage service lifetime.
    , m_storage(storage)
    // m_localization is non-owning because main.cpp or tests own language state.
    , m_localization(localization)
{
    if (m_localization) {
        connect(m_localization, &Localization::languageChanged, this, &NavigationModel::refresh);
    }
}

NavigationModel::NavigationModel(NoteStorage *storage, QObject *parent)
    : NavigationModel(storage, nullptr, parent)
{
}

int NavigationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        // This is a flat list model, so child indexes never have rows.
        return 0;
    }

    // m_rows contains the currently visible flattened year/month/day navigation tree.
    return m_rows.size();
}

QVariant NavigationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_rows.size()) {
        // Invalid indexes return an empty QVariant as Qt models expect.
        return {};
    }

    // row is the flattened navigation item for the requested model index.
    const Row &row = m_rows.at(index.row());
    switch (role) {
    case DisplayRole:
    case NavTextRole:
        return row.display;
    case LevelRole:
        return row.level;
    case DateRole:
        // Only day rows have a valid click target date.
        return row.date.isValid() ? row.date.toString(Qt::ISODate) : QString();
    case ExpandableRole:
        return row.expandable;
    case ExpandedRole:
        return row.expandable && isExpanded(row.key);
    default:
        return {};
    }
}

QHash<int, QByteArray> NavigationModel::roleNames() const
{
    // Role names are the public contract used by Main.qml.
    return {
        {DisplayRole, "display"},
        {NavTextRole, "navText"},
        {LevelRole, "level"},
        {DateRole, "date"},
        {ExpandableRole, "expandable"},
        {ExpandedRole, "expanded"},
    };
}

void NavigationModel::refresh()
{
    // dates is the sorted list of existing note files discovered by storage.
    const QList<QDate> dates = m_storage->existingDates();

    // Reset because the whole visible tree may change after save/delete/toggle.
    beginResetModel();
    m_rows.clear();

    // seenYears prevents duplicate year header rows.
    QSet<int> seenYears;

    // seenMonths prevents duplicate month header rows inside a year.
    QSet<QString> seenMonths;

    for (const QDate &date : dates) {
        // currentYearKey identifies the parent year group.
        const QString currentYearKey = yearKey(date.year());
        if (!seenYears.contains(date.year())) {
            seenYears.insert(date.year());

            // yearLabel is rendered as a level-0 navigation header.
            m_rows.append({yearLabel(date.year()), 0, {}, currentYearKey, true});
        }

        if (!isExpanded(currentYearKey)) {
            // Collapsed years hide all month and day children.
            continue;
        }

        // currentMonthKey combines year and month so January in different years stays distinct.
        const QString currentMonthKey = monthKey(date);
        if (!seenMonths.contains(currentMonthKey)) {
            seenMonths.insert(currentMonthKey);

            // monthLabel is rendered as a level-1 navigation header.
            m_rows.append({monthLabel(date.month()), 1, {}, currentMonthKey, true});
        }

        if (!isExpanded(currentMonthKey)) {
            // Collapsed months hide day rows while keeping the month row visible.
            continue;
        }

        // Day rows are level 2 and carry the actual ISO date click target.
        m_rows.append({date.toString(Qt::ISODate), 2, date, {}, false});
    }

    endResetModel();
}

void NavigationModel::toggleExpanded(int row)
{
    if (row < 0 || row >= m_rows.size()) {
        // Ignore invalid QML row requests.
        return;
    }

    // item is the visible row the user clicked.
    const Row item = m_rows.at(row);
    if (!item.expandable) {
        // Day rows are navigation targets and cannot be expanded.
        return;
    }

    if (isExpanded(item.key)) {
        // Store only collapsed keys so newly discovered groups default to expanded.
        m_collapsedKeys.insert(item.key);
    } else {
        m_collapsedKeys.remove(item.key);
    }

    refresh();
}

bool NavigationModel::isExpanded(const QString &key) const
{
    // Empty keys are used by day rows and are not expandable.
    return !key.isEmpty() && !m_collapsedKeys.contains(key);
}

QString NavigationModel::yearLabel(int year) const
{
    return m_localization ? m_localization->formatYearLabel(year) : QString::number(year);
}

QString NavigationModel::monthLabel(int month) const
{
    if (m_localization) {
        return m_localization->formatMonthLabel(month);
    }
    return QLocale(QLocale::English, QLocale::UnitedStates).monthName(month, QLocale::LongFormat);
}

QString NavigationModel::yearKey(int year) const
{
    // Prefix keeps year keys distinct from month keys.
    return QStringLiteral("year:%1").arg(year);
}

QString NavigationModel::monthKey(const QDate &date) const
{
    // Prefix keeps month keys distinct from year keys.
    return QStringLiteral("month:%1-%2").arg(date.year()).arg(date.month(), 2, 10, QLatin1Char('0'));
}
