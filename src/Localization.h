#pragma once

#include <QDate>
#include <QLocale>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantList>

class Localization : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString language READ language NOTIFY languageChanged)
    Q_PROPERTY(QVariantList languages READ languages CONSTANT)
    Q_PROPERTY(bool rightToLeft READ isRightToLeft NOTIFY languageChanged)

public:
    explicit Localization(QString settingsFilePath, QObject *parent = nullptr);

    QString language() const;
    QVariantList languages() const;
    bool isRightToLeft() const;
    QStringList translationKeys() const;

    Q_INVOKABLE QString t(const QString &key) const;
    Q_INVOKABLE bool setLanguage(const QString &languageCode);
    Q_INVOKABLE int languageIndex() const;

    static QString englishText(QStringView key);

    QString text(QStringView key) const;
    QString formatDateTitle(const QDate &date) const;
    QString formatYearLabel(int year) const;
    QString formatMonthLabel(int month) const;

signals:
    void languageChanged();

private:
    QString loadLanguage() const;
    bool isSupportedLanguage(const QString &languageCode) const;
    QLocale localeForLanguage(const QString &languageCode) const;
    QLocale localeForCurrentLanguage() const;

    QString m_settingsFilePath;
    QString m_language;
};
