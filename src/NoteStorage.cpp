#include "NoteStorage.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

#include <algorithm>
#include <utility>

namespace {

constexpr int kSchemaVersion = 2;
constexpr qsizetype kCompressionLevel = 9;
const char kPayloadPrefix[] = "ABCN1.";
const char kPayloadEncoding[] = "abcnote-packed-v1";
const char kBodyFormat[] = "markdown";

QByteArray payloadKey()
{
    return QByteArrayLiteral("ABCNote::PackedNotePayload::v1");
}

QByteArray applyPayloadMask(QByteArray bytes)
{
    const QByteArray key = payloadKey();
    for (qsizetype i = 0; i < bytes.size(); ++i) {
        bytes[i] = static_cast<char>(bytes.at(i) ^ key.at(i % key.size()));
    }
    return bytes;
}

QString packPayload(const Note &note)
{
    QJsonObject payloadObject;
    payloadObject.insert(QStringLiteral("contentBody"), note.contentBody);
    payloadObject.insert(QStringLiteral("aiSummary"), note.aiSummary);

    const QByteArray jsonBytes = QJsonDocument(payloadObject).toJson(QJsonDocument::Compact);
    const QByteArray compressedBytes = qCompress(jsonBytes, kCompressionLevel);
    const QByteArray packedBytes = applyPayloadMask(compressedBytes);
    const QString encoded = QString::fromLatin1(packedBytes.toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));
    return QString::fromLatin1(kPayloadPrefix) + encoded;
}

bool unpackPayload(const QString &packedPayload, Note *note)
{
    if (note == nullptr || !packedPayload.startsWith(QLatin1String(kPayloadPrefix))) {
        return false;
    }

    const QString encoded = packedPayload.mid(QString::fromLatin1(kPayloadPrefix).size());
    const QByteArray packedBytes = QByteArray::fromBase64(encoded.toLatin1(), QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals);
    const QByteArray compressedBytes = applyPayloadMask(packedBytes);
    const QByteArray jsonBytes = qUncompress(compressedBytes);
    if (jsonBytes.isEmpty()) {
        return false;
    }

    const QJsonDocument document = QJsonDocument::fromJson(jsonBytes);
    if (!document.isObject()) {
        return false;
    }

    const QJsonObject object = document.object();
    note->contentBody = object.value(QStringLiteral("contentBody")).toString();
    note->aiSummary = object.value(QStringLiteral("aiSummary")).toString();
    return true;
}

} // namespace

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

        // note is populated from the versioned ABCNote payload format.
        Note note;
        note.date = QDate::fromString(object.value(QStringLiteral("date")).toString(), Qt::ISODate);
        if (!note.date.isValid()) {
            // Fall back to the requested date rather than returning an invalid note.
            note.date = date;
        }
        note.updatedAt = QDateTime::fromString(object.value(QStringLiteral("updatedAt")).toString(), Qt::ISODate);
        if (object.value(QStringLiteral("schemaVersion")).toInt() != kSchemaVersion
            || object.value(QStringLiteral("payloadEncoding")).toString() != QLatin1String(kPayloadEncoding)
            || !unpackPayload(object.value(QStringLiteral("payload")).toString(), &note)) {
            note.contentBody.clear();
            note.aiSummary.clear();
        }
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

    // object keeps only metadata in plaintext; note text lives in the packed payload.
    QJsonObject object;
    object.insert(QStringLiteral("schemaVersion"), kSchemaVersion);
    object.insert(QStringLiteral("date"), note.date.toString(Qt::ISODate));
    object.insert(QStringLiteral("bodyFormat"), QLatin1String(kBodyFormat));
    object.insert(QStringLiteral("updatedAt"), note.updatedAt.toString(Qt::ISODate));
    object.insert(QStringLiteral("payloadEncoding"), QLatin1String(kPayloadEncoding));
    object.insert(QStringLiteral("payload"), packPayload(note));

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
