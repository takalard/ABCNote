#pragma once

#include <QObject>
#include <QString>

class NoteController;
class NoteStorage;
class Localization;

// Owns user-facing application settings and coordinates data-directory changes.
class AppSettings : public QObject
{
    Q_OBJECT

    // Current note data root shown in the settings popup.
    Q_PROPERTY(QString dataRoot READ dataRoot NOTIFY dataRootChanged)

    // Last settings operation status shown in the settings popup.
    Q_PROPERTY(QString statusMessage READ statusMessage NOTIFY statusMessageChanged)

    // Whether user-enabled AI summary requests are allowed.
    Q_PROPERTY(bool aiSummaryEnabled READ aiSummaryEnabled NOTIFY aiSummarySettingsChanged)

    // OpenAI-compatible endpoint root configured by the user.
    Q_PROPERTY(QString aiBaseUrl READ aiBaseUrl NOTIFY aiSummarySettingsChanged)

    // Model id configured by the user.
    Q_PROPERTY(QString aiModel READ aiModel NOTIFY aiSummarySettingsChanged)

    // True when an API key is stored for AI summary requests.
    Q_PROPERTY(bool hasAiApiKey READ hasAiApiKey NOTIFY aiSummarySettingsChanged)

public:
    // Creates settings using the runtime application directory and default QSettings location.
    explicit AppSettings(QObject *parent = nullptr);

    // Creates settings with injectable paths for deterministic tests.
    AppSettings(QString settingsFilePath,
                QString applicationDirPath,
                Localization *localization = nullptr,
                QObject *parent = nullptr);

    // Backward-compatible constructor for tests and callers without localization.
    AppSettings(QString settingsFilePath, QString applicationDirPath, QObject *parent);

    // Returns the current note data root.
    QString dataRoot() const;

    // Returns the latest settings operation status text.
    QString statusMessage() const;

    // Returns whether AI summary requests are user-enabled.
    bool aiSummaryEnabled() const;

    // Returns the configured OpenAI-compatible endpoint root.
    QString aiBaseUrl() const;

    // Returns the configured model id.
    QString aiModel() const;

    // Returns whether a user API key is stored.
    bool hasAiApiKey() const;

    // Returns the stored API key for request code.
    QString aiApiKey() const;

    // Supplies the storage service that should switch roots after migration.
    void setStorage(NoteStorage *storage);

    // Supplies the controller that should flush before and reload after migration.
    void setController(NoteController *controller);

    // Test helper that persists and applies a data root without migration.
    bool setDataRootForTesting(const QString &rootPath);

    // Copies data from the old root to the target root, then switches services to the target root.
    Q_INVOKABLE bool migrateAndSwitchDataRoot(const QString &targetRootPath);

    // Accepts a QML FolderDialog URL and switches to the selected local folder.
    Q_INVOKABLE bool migrateAndSwitchDataRootUrl(const QString &targetRootUrl);

    // Saves AI summary settings and stores a non-empty API key when provided.
    Q_INVOKABLE bool saveAiSummarySettings(bool enabled,
                                           const QString &baseUrl,
                                           const QString &model,
                                           const QString &apiKey);

    // Removes the stored AI API key.
    Q_INVOKABLE bool clearAiApiKey();

signals:
    // Emitted when the current data root changes.
    void dataRootChanged();

    // Emitted when statusMessage changes.
    void statusMessageChanged();

    // Emitted when AI summary settings or key presence changes.
    void aiSummarySettingsChanged();

private:
    // Result of recursively copying files into a target directory.
    struct CopyResult
    {
        bool success = true;
        bool hadExistingFiles = false;
    };

    // Returns the default application-adjacent data directory.
    QString defaultDataRoot() const;

    // Loads the persisted data root or falls back to defaultDataRoot().
    QString loadDataRoot() const;

    // Loads AI summary non-secret settings from QSettings.
    void loadAiSummarySettings();

    // Persists and applies a new data root.
    bool applyDataRoot(const QString &rootPath);

    // Copies all files from sourcePath to targetPath without overwriting existing files.
    CopyResult copyDirectoryContents(const QString &sourcePath, const QString &targetPath) const;

    // Updates statusMessage and notifies QML.
    void setStatusMessage(const QString &message);

    // Resolves a translated status message, falling back to the key when localization is absent.
    QString statusText(QStringView key) const;

    // Returns the per-settings-file credential target name.
    QString aiCredentialTargetName() const;

    // Writes the API key to the platform secret store or fallback settings.
    bool writeAiApiKey(const QString &apiKey);

    // Reads the API key from the platform secret store or fallback settings.
    QString readAiApiKey() const;

    // Deletes the API key from the platform secret store or fallback settings.
    bool deleteAiApiKey();

    // Path to the INI settings file used by QSettings.
    QString m_settingsFilePath;

    // Non-owning pointer to localization for user-facing status text.
    Localization *m_localization = nullptr;

    // Application directory used to compute the default data root.
    QString m_applicationDirPath;

    // Current note data root.
    QString m_dataRoot;

    // User opt-in flag for AI summary calls.
    bool m_aiSummaryEnabled = false;

    // User configured OpenAI-compatible endpoint root.
    QString m_aiBaseUrl;

    // User configured model id.
    QString m_aiModel;

    // Latest settings status text.
    QString m_statusMessage;

    // Non-owning storage pointer supplied by main.cpp or tests.
    NoteStorage *m_storage = nullptr;

    // Non-owning controller pointer supplied by main.cpp.
    NoteController *m_controller = nullptr;
};
