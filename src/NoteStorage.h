#pragma once

#include "Note.h"

#include <QDate>
#include <QList>
#include <QString>

// Handles all filesystem persistence for date-based JSON note files.
class NoteStorage
{
public:
    // Creates storage rooted at the directory that contains the year/month/day tree.
    explicit NoteStorage(QString rootPath);

    // Returns the root directory used by this storage instance.
    QString rootPath() const;

    // Changes the root directory used by future load, save, and scan operations.
    void setRootPath(QString rootPath);

    // Converts a date into the required data/YYYY/MM/YYYY-MM-DD.json file path.
    QString filePathForDate(const QDate &date) const;

    // Loads an existing note or returns an unsaved temporary note for the requested date.
    Note load(const QDate &date) const;

    // Loads an existing note or returns an unsaved temporary note for older create-style callers.
    Note loadOrCreate(const QDate &date);

    // Returns whether a persisted JSON file exists for the requested date.
    bool exists(const QDate &date) const;

    // Deletes the persisted JSON file for the requested date when it exists.
    bool remove(const QDate &date);

    // Saves non-empty notes and deletes empty notes using content-aware persistence.
    bool save(Note note);

    // Scans the data tree and returns all dates that already have JSON files.
    QList<QDate> existingDates() const;

private:
    // Root directory that contains the first-level year folders.
    QString m_rootPath;
};
