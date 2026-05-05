#pragma once

#include <QObject>
#include <QString>

class Localization;

// Sends user-authorized AI summary requests through an OpenAI-compatible API.
class SummaryService : public QObject
{
    Q_OBJECT

public:
    struct RequestSettings
    {
        bool enabled = false;
        QString baseUrl;
        QString model;
        QString apiKey;
    };

    explicit SummaryService(Localization *localization = nullptr, QObject *parent = nullptr);

    // Backward-compatible constructor for callers that only pass a QObject parent.
    explicit SummaryService(QObject *parent);

    // Requests a summary for one note and reports completion through signals.
    void requestSummary(const QString &noteId, const QString &contentBody, const RequestSettings &settings);

signals:
    // Emitted when the remote model returns summary text.
    void summaryReady(const QString &noteId, const QString &summary);

    // Emitted when a request cannot be made or completed.
    void summaryFailed(const QString &noteId, const QString &message);

private:
    // Resolves a translated status message, falling back to the key when localization is absent.
    QString statusText(QStringView key) const;

    // Non-owning pointer to localization for validation messages and the AI prompt.
    Localization *m_localization = nullptr;
};
