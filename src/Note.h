#pragma once

#include <QDate>
#include <QDateTime>
#include <QString>

// Stores all persisted fields for one calendar-day note.
struct Note
{
    // Calendar date that uniquely identifies this note.
    QDate date;

    // User-editable body text shown in the main editor.
    QString contentBody;

    // Short summary generated from contentBody by SummaryService.
    QString aiSummary;

    // Last time this note was changed or saved.
    QDateTime updatedAt;
};

