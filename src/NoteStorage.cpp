#include "NoteStorage.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include <algorithm>
#include <utility>

NoteStorage::NoteStorage(QString rootPath)
    // Move the caller-provided root path because the storage object owns this path value.
    : m_rootPath(std::move(rootPath))
{
}

QString NoteStorage::rootPath() const
{
    // Return the configured data root without normalizing it so callers can compare exact paths.
    return m_rootPath;
}

void NoteStorage::setRootPath(QString rootPath)
{
    // The new root is used for all future filesystem operations.
    m_rootPath = std::move(rootPath);
}

QString NoteStorage::filePathForDate(const QDate &date) const
{
    // root is the base directory that contains year folders.
    QDir root(m_rootPath);

    // relativePath keeps the required three-level date structure.
    const QString relativePath = QStringLiteral("%1/%2/%3.json")
                                     .arg(date.toString(QStringLiteral("yyyy")))
                                     .arg(date.toString(QStringLiteral("MM")))
                                     .arg(date.toString(Qt::ISODate));

    // filePath joins the root path using Qt's platform-aware path handling.
    return root.filePath(relativePath);
}

Note NoteStorage::load(const QDate &date) const
{
    // path is the exact JSON file location for the requested calendar day.
    const QString path = filePathForDate(date);

    // file is used only for reading existing JSON content.
    QFile file(path);
    if (file.exists() && file.open(QIODevice::ReadOnly)) {
        // document parses the note JSON file into a Qt JSON tree.
        const QJsonDocument document = QJsonDocument::fromJson(file.readAll());

        // object contains the persisted note fields.
        const QJsonObject object = document.object();

        // note is populated field-by-field so malformed dates can be repaired in memory.
        Note note;
        note.date = QDate::fromString(object.value(QStringLiteral("date")).toString(), Qt::ISODate);
        if (!note.date.isValid()) {
            // Fall back to the requested date rather than returning an invalid note.
            note.date = date;
        }
        note.contentBody = object.value(QStringLiteral("contentBody")).toString();
        note.aiSummary = object.value(QStringLiteral("aiSummary")).toString();
        note.updatedAt = QDateTime::fromString(object.value(QStringLiteral("updatedAt")).toString(), Qt::ISODate);
        return note;
    }

    // note is an in-memory temporary note for missing dates.
    Note note;
    note.date = date;
    note.updatedAt = QDateTime::currentDateTime();
    return note;
}

Note NoteStorage::loadOrCreate(const QDate &date)
{
    // Creation now means creating an in-memory temporary note, not a persisted empty file.
    return load(date);
}

bool NoteStorage::exists(const QDate &date) const
{
    // QFile::exists checks whether this date has a persisted JSON file.
    return QFile::exists(filePathForDate(date));
}

bool NoteStorage::remove(const QDate &date)
{
    // path is the persisted file that should be removed.
    const QString path = filePathForDate(date);
    if (!QFile::exists(path)) {
        return true;
    }

    // QFile::remove deletes only the date file, leaving parent year/month folders intact.
    return QFile::remove(path);
}

bool NoteStorage::save(Note note)
{
    if (!note.date.isValid()) {
        // An invalid date cannot be mapped into the required year/month/day path.
        return false;
    }

    if (note.contentBody.trimmed().isEmpty()) {
        // Empty notes are temporary and should not leave a persisted JSON file.
        return remove(note.date);
    }

    if (!note.updatedAt.isValid()) {
        // updatedAt records when this save request reached storage.
        note.updatedAt = QDateTime::currentDateTime();
    }

    // path is the final JSON file that should receive this note.
    const QString path = filePathForDate(note.date);

    // dir is the YYYY/MM directory that must exist before opening QSaveFile.
    QDir dir = QFileInfo(path).absoluteDir();
    if (!dir.exists() && !dir.mkpath(QStringLiteral("."))) {
        return false;
    }

    // object mirrors the documented JSON schema.
    QJsonObject object;
    object.insert(QStringLiteral("date"), note.date.toString(Qt::ISODate));
    object.insert(QStringLiteral("contentBody"), note.contentBody);
    object.insert(QStringLiteral("aiSummary"), note.aiSummary);
    object.insert(QStringLiteral("updatedAt"), note.updatedAt.toString(Qt::ISODate));

    // file writes to a temporary file first and commits atomically where the platform supports it.
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    // jsonBytes is the human-readable representation of the note file.
    const QByteArray jsonBytes = QJsonDocument(object).toJson(QJsonDocument::Indented);
    file.write(jsonBytes);
    return file.commit();
}

QList<QDate> NoteStorage::existingDates() const
{
    // dates accumulates all valid note file dates found under the data root.
    QList<QDate> dates;

    // root is scanned for year-level folders.
    QDir root(m_rootPath);
    const QStringList yearDirs = root.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QString &year : yearDirs) {
        // yearDir points to one first-level year directory.
        QDir yearDir(root.filePath(year));

        // monthDirs are the second-level month directories inside the year.
        const QStringList monthDirs = yearDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
        for (const QString &month : monthDirs) {
            // monthDir contains third-level day JSON files.
            QDir monthDir(yearDir.filePath(month));

            // files are candidate note JSON files named YYYY-MM-DD.json.
            const QStringList files = monthDir.entryList(QStringList() << QStringLiteral("*.json"), QDir::Files, QDir::Name);
            for (const QString &fileName : files) {
                // baseName removes .json so it can be parsed as an ISO date.
                const QString baseName = QFileInfo(fileName).completeBaseName();

                // date is accepted only when the filename is a valid ISO calendar date.
                const QDate date = QDate::fromString(baseName, Qt::ISODate);
                if (date.isValid()) {
                    // note reads the candidate file so legacy empty JSON files can be treated as temporary notes.
                    const Note note = load(date);

                    // bodyHasContent mirrors save(): only notes with non-empty body text are persisted navigation entries.
                    const bool bodyHasContent = !note.contentBody.trimmed().isEmpty();
                    if (bodyHasContent) {
                        dates.append(date);
                    }
                }
            }
        }
    }

    // Sort dates so navigation rows are deterministic and chronological.
    std::sort(dates.begin(), dates.end());
    return dates;
}
