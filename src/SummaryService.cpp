#include "SummaryService.h"

#include "Localization.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

SummaryService::SummaryService(Localization *localization, QObject *parent)
    : QObject(parent)
    , m_localization(localization)
{
}

SummaryService::SummaryService(QObject *parent)
    : SummaryService(nullptr, parent)
{
}

void SummaryService::requestSummary(const QString &noteId, const QString &contentBody, const RequestSettings &settings)
{
    const QString cleanBody = contentBody.trimmed();
    const QString cleanBaseUrl = settings.baseUrl.trimmed();
    const QString cleanModel = settings.model.trimmed();
    const QString cleanApiKey = settings.apiKey.trimmed();

    if (!settings.enabled) {
        emit summaryFailed(noteId, statusText(u"status.aiDisabled"));
        return;
    }

    if (cleanBaseUrl.isEmpty() || cleanModel.isEmpty() || cleanApiKey.isEmpty()) {
        emit summaryFailed(noteId, statusText(u"status.aiMissingSettings"));
        return;
    }

    if (cleanBody.isEmpty()) {
        emit summaryFailed(noteId, statusText(u"status.aiEmptyBody"));
        return;
    }

    QUrl endpoint(cleanBaseUrl);
    QString path = endpoint.path();
    if (path.endsWith(QLatin1Char('/'))) {
        path.chop(1);
    }
    path += QStringLiteral("/chat/completions");
    endpoint.setPath(path);

    if (!endpoint.isValid() || endpoint.scheme().isEmpty() || endpoint.host().isEmpty()) {
        emit summaryFailed(noteId, statusText(u"status.aiInvalidBaseUrl"));
        return;
    }

    QJsonArray messages;
    messages.append(QJsonObject{
        {QStringLiteral("role"), QStringLiteral("system")},
        {QStringLiteral("content"), m_localization ? m_localization->t(QStringLiteral("ai.summaryPrompt"))
                                                    : QStringLiteral("You are ABCNote's note summary assistant. Summarize the user's note concisely.")},
    });
    messages.append(QJsonObject{
        {QStringLiteral("role"), QStringLiteral("user")},
        {QStringLiteral("content"), cleanBody},
    });

    QJsonObject payload;
    payload.insert(QStringLiteral("model"), cleanModel);
    payload.insert(QStringLiteral("messages"), messages);
    payload.insert(QStringLiteral("temperature"), 0.2);

    QNetworkRequest request(endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));
    request.setRawHeader("Authorization", "Bearer " + cleanApiKey.toUtf8());

    auto *manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->post(request, QJsonDocument(payload).toJson(QJsonDocument::Compact));
    connect(reply, &QNetworkReply::finished, this, [this, reply, manager, noteId]() {
        const QByteArray body = reply->readAll();
        const QNetworkReply::NetworkError error = reply->error();
        const QString errorMessage = reply->errorString();
        reply->deleteLater();
        manager->deleteLater();

        if (error != QNetworkReply::NoError) {
            emit summaryFailed(noteId, errorMessage);
            return;
        }

        const QJsonDocument document = QJsonDocument::fromJson(body);
        const QJsonArray choices = document.object().value(QStringLiteral("choices")).toArray();
        const QString summary = choices.isEmpty()
                                    ? QString()
                                    : choices.first().toObject()
                                          .value(QStringLiteral("message"))
                                          .toObject()
                                          .value(QStringLiteral("content"))
                                          .toString()
                                          .trimmed();
        if (summary.isEmpty()) {
            emit summaryFailed(noteId, statusText(u"status.aiNoSummaryReturned"));
            return;
        }

        emit summaryReady(noteId, summary);
    });
}

QString SummaryService::statusText(QStringView key) const
{
    return m_localization ? m_localization->text(key) : Localization::englishText(key);
}
