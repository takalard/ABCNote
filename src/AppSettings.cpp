#include "AppSettings.h"

#include "Localization.h"
#include "NoteController.h"
#include "NoteStorage.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QCryptographicHash>
#include <QSettings>
#include <QUrl>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <wincred.h>
#endif

AppSettings::AppSettings(QObject *parent)
    : AppSettings(QDir(QCoreApplication::applicationDirPath()).filePath(QStringLiteral("ABCNote.ini")),
                  QCoreApplication::applicationDirPath(),
                  nullptr,
                  parent)
{
}

AppSettings::AppSettings(QString settingsFilePath,
                         QString applicationDirPath,
                         Localization *localization,
                         QObject *parent)
    : QObject(parent)
    , m_settingsFilePath(std::move(settingsFilePath))
    , m_localization(localization)
    , m_applicationDirPath(std::move(applicationDirPath))
    , m_dataRoot(loadDataRoot())
    , m_markdownPreviewMode(loadMarkdownPreviewMode())
{
    loadAiSummarySettings();
}

AppSettings::AppSettings(QString settingsFilePath, QString applicationDirPath, QObject *parent)
    : AppSettings(std::move(settingsFilePath), std::move(applicationDirPath), nullptr, parent)
{
}

QString AppSettings::dataRoot() const
{
    return m_dataRoot;
}

QString AppSettings::statusMessage() const
{
    return m_statusMessage;
}

bool AppSettings::markdownPreviewMode() const
{
    return m_markdownPreviewMode;
}

bool AppSettings::aiSummaryEnabled() const
{
    return m_aiSummaryEnabled;
}

QString AppSettings::aiBaseUrl() const
{
    return m_aiBaseUrl;
}

QString AppSettings::aiModel() const
{
    return m_aiModel;
}

bool AppSettings::hasAiApiKey() const
{
    return !readAiApiKey().isEmpty();
}

QString AppSettings::aiApiKey() const
{
    return readAiApiKey();
}

void AppSettings::setStorage(NoteStorage *storage)
{
    m_storage = storage;
}

void AppSettings::setController(NoteController *controller)
{
    m_controller = controller;
}

bool AppSettings::setDataRootForTesting(const QString &rootPath)
{
    return applyDataRoot(rootPath);
}

bool AppSettings::migrateAndSwitchDataRoot(const QString &targetRootPath)
{
    const QString cleanTarget = QDir::cleanPath(targetRootPath);
    if (cleanTarget.isEmpty()) {
        return false;
    }

    const QString cleanCurrent = QDir::cleanPath(m_dataRoot);
    if (QDir(cleanCurrent) == QDir(cleanTarget)) {
        setStatusMessage(statusText(u"status.dataDirectoryUnchanged"));
        return true;
    }

    if (m_controller) {
        m_controller->flush();
    }

    const CopyResult result = copyDirectoryContents(cleanCurrent, cleanTarget);
    if (!result.success) {
        setStatusMessage(statusText(u"status.dataDirectoryMigrationFailed"));
        return false;
    }

    if (!applyDataRoot(cleanTarget)) {
        setStatusMessage(statusText(u"status.dataDirectorySaveFailed"));
        return false;
    }

    if (m_controller) {
        m_controller->reloadCurrentWindow();
    }

    setStatusMessage(result.hadExistingFiles
                         ? statusText(u"status.dataDirectorySwitchedWithExistingFiles")
                         : statusText(u"status.dataDirectorySwitched"));
    return true;
}

bool AppSettings::migrateAndSwitchDataRootUrl(const QString &targetRootUrl)
{
    const QUrl url(targetRootUrl);
    const QString localPath = url.isLocalFile() ? url.toLocalFile() : targetRootUrl;
    return migrateAndSwitchDataRoot(localPath);
}

bool AppSettings::setMarkdownPreviewMode(bool enabled)
{
    QSettings settings(m_settingsFilePath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("ui/markdownPreviewMode"), enabled);
    settings.sync();
    if (settings.status() != QSettings::NoError) {
        return false;
    }

    if (m_markdownPreviewMode != enabled) {
        m_markdownPreviewMode = enabled;
        emit markdownPreviewModeChanged();
    }
    return true;
}

bool AppSettings::saveAiSummarySettings(bool enabled, const QString &baseUrl, const QString &model, const QString &apiKey)
{
    const QString cleanBaseUrl = baseUrl.trimmed();
    const QString cleanModel = model.trimmed();
    const QString cleanApiKey = apiKey.trimmed();

    QSettings settings(m_settingsFilePath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("aiSummary/enabled"), enabled);
    settings.setValue(QStringLiteral("aiSummary/baseUrl"), cleanBaseUrl);
    settings.setValue(QStringLiteral("aiSummary/model"), cleanModel);
    settings.sync();
    if (settings.status() != QSettings::NoError) {
        setStatusMessage(statusText(u"status.aiSettingsSaveFailed"));
        return false;
    }

    if (!cleanApiKey.isEmpty() && !writeAiApiKey(cleanApiKey)) {
        setStatusMessage(statusText(u"status.aiKeySaveFailed"));
        return false;
    }

    m_aiSummaryEnabled = enabled;
    m_aiBaseUrl = cleanBaseUrl;
    m_aiModel = cleanModel;
    emit aiSummarySettingsChanged();
    setStatusMessage(statusText(u"status.aiSettingsSaved"));
    return true;
}

bool AppSettings::clearAiApiKey()
{
    if (!deleteAiApiKey()) {
        setStatusMessage(statusText(u"status.aiKeyClearFailed"));
        return false;
    }

    emit aiSummarySettingsChanged();
    setStatusMessage(statusText(u"status.aiKeyCleared"));
    return true;
}

QString AppSettings::defaultDataRoot() const
{
    return QDir(m_applicationDirPath).filePath(QStringLiteral("data"));
}

QString AppSettings::loadDataRoot() const
{
    QSettings settings(m_settingsFilePath, QSettings::IniFormat);
    const QString persisted = settings.value(QStringLiteral("dataRoot")).toString();
    if (!persisted.trimmed().isEmpty()) {
        return QDir::cleanPath(persisted);
    }
    return QDir::cleanPath(defaultDataRoot());
}

bool AppSettings::loadMarkdownPreviewMode() const
{
    QSettings settings(m_settingsFilePath, QSettings::IniFormat);
    return settings.value(QStringLiteral("ui/markdownPreviewMode"), false).toBool();
}

void AppSettings::loadAiSummarySettings()
{
    QSettings settings(m_settingsFilePath, QSettings::IniFormat);
    m_aiSummaryEnabled = settings.value(QStringLiteral("aiSummary/enabled"), false).toBool();
    m_aiBaseUrl = settings.value(QStringLiteral("aiSummary/baseUrl")).toString().trimmed();
    m_aiModel = settings.value(QStringLiteral("aiSummary/model")).toString().trimmed();
}

bool AppSettings::applyDataRoot(const QString &rootPath)
{
    const QString cleanRoot = QDir::cleanPath(rootPath);
    if (cleanRoot.isEmpty()) {
        return false;
    }

    QSettings settings(m_settingsFilePath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("dataRoot"), cleanRoot);
    settings.sync();
    if (settings.status() != QSettings::NoError) {
        return false;
    }

    if (m_dataRoot != cleanRoot) {
        m_dataRoot = cleanRoot;
        emit dataRootChanged();
    }

    if (m_storage) {
        m_storage->setRootPath(cleanRoot);
    }
    return true;
}

AppSettings::CopyResult AppSettings::copyDirectoryContents(const QString &sourcePath, const QString &targetPath) const
{
    CopyResult result;

    QDir targetDir(targetPath);
    if (!targetDir.exists() && !targetDir.mkpath(QStringLiteral("."))) {
        result.success = false;
        return result;
    }

    QDir sourceDir(sourcePath);
    if (!sourceDir.exists()) {
        return result;
    }

    const QFileInfoList entries = sourceDir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);
    for (const QFileInfo &entry : entries) {
        const QString targetEntryPath = targetDir.filePath(entry.fileName());
        if (entry.isDir()) {
            const CopyResult child = copyDirectoryContents(entry.absoluteFilePath(), targetEntryPath);
            result.success = result.success && child.success;
            result.hadExistingFiles = result.hadExistingFiles || child.hadExistingFiles;
            if (!result.success) {
                return result;
            }
            continue;
        }

        if (QFile::exists(targetEntryPath)) {
            result.hadExistingFiles = true;
            continue;
        }

        if (!QFile::copy(entry.absoluteFilePath(), targetEntryPath)) {
            result.success = false;
            return result;
        }
    }

    return result;
}

void AppSettings::setStatusMessage(const QString &message)
{
    if (m_statusMessage == message) {
        return;
    }

    m_statusMessage = message;
    emit statusMessageChanged();
}

QString AppSettings::statusText(QStringView key) const
{
    return m_localization ? m_localization->text(key) : Localization::englishText(key);
}

QString AppSettings::aiCredentialTargetName() const
{
    const QByteArray digest = QCryptographicHash::hash(m_settingsFilePath.toUtf8(), QCryptographicHash::Sha256).toHex();
    return QStringLiteral("ABCNote/AI/APIKey/%1").arg(QString::fromLatin1(digest.left(16)));
}

bool AppSettings::writeAiApiKey(const QString &apiKey)
{
#ifdef Q_OS_WIN
    const std::wstring target = aiCredentialTargetName().toStdWString();
    const QByteArray secretBytes = apiKey.toUtf8();

    CREDENTIALW credential = {};
    credential.Type = CRED_TYPE_GENERIC;
    credential.TargetName = const_cast<LPWSTR>(target.c_str());
    credential.CredentialBlobSize = static_cast<DWORD>(secretBytes.size());
    credential.CredentialBlob = reinterpret_cast<LPBYTE>(const_cast<char *>(secretBytes.constData()));
    credential.Persist = CRED_PERSIST_LOCAL_MACHINE;
    credential.UserName = const_cast<LPWSTR>(L"ABCNote");

    return CredWriteW(&credential, 0) == TRUE;
#else
    QSettings settings(m_settingsFilePath, QSettings::IniFormat);
    settings.setValue(QStringLiteral("aiSummary/apiKey"), apiKey);
    settings.sync();
    return settings.status() == QSettings::NoError;
#endif
}

QString AppSettings::readAiApiKey() const
{
#ifdef Q_OS_WIN
    const std::wstring target = aiCredentialTargetName().toStdWString();
    PCREDENTIALW credential = nullptr;
    if (CredReadW(target.c_str(), CRED_TYPE_GENERIC, 0, &credential) != TRUE || credential == nullptr) {
        return {};
    }

    const QByteArray secretBytes(reinterpret_cast<const char *>(credential->CredentialBlob),
                                 static_cast<int>(credential->CredentialBlobSize));
    const QString apiKey = QString::fromUtf8(secretBytes);
    CredFree(credential);
    return apiKey;
#else
    QSettings settings(m_settingsFilePath, QSettings::IniFormat);
    return settings.value(QStringLiteral("aiSummary/apiKey")).toString();
#endif
}

bool AppSettings::deleteAiApiKey()
{
#ifdef Q_OS_WIN
    const std::wstring target = aiCredentialTargetName().toStdWString();
    if (CredDeleteW(target.c_str(), CRED_TYPE_GENERIC, 0) == TRUE) {
        return true;
    }
    return GetLastError() == ERROR_NOT_FOUND;
#else
    QSettings settings(m_settingsFilePath, QSettings::IniFormat);
    settings.remove(QStringLiteral("aiSummary/apiKey"));
    settings.sync();
    return settings.status() == QSettings::NoError;
#endif
}
