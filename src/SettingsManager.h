#pragma once

#include <QObject>
#include <QSettings>
#include <QString>

class SettingsManager : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString gamePath READ gamePath WRITE setGamePath NOTIFY gamePathChanged)
    Q_PROPERTY(bool hasGamePath READ hasGamePath NOTIFY gamePathChanged)
    Q_PROPERTY(bool canLaunchGame READ canLaunchGame NOTIFY gamePathChanged)

public:
    explicit SettingsManager(QObject* parent = nullptr);

    QString gamePath() const;
    Q_INVOKABLE bool setGamePath(const QString& path);
    bool hasGamePath() const;
    bool canLaunchGame() const;

    Q_INVOKABLE QString browseGameFolder();
    Q_INVOKABLE QString resolveTargetFolder(const QString& categorySlug) const;
    QString targetFolderForCategory(const QString& categorySlug) const { return resolveTargetFolder(categorySlug); }
    Q_INVOKABLE bool isValidBeamNgFolder(const QString& path) const;
    Q_INVOKABLE QString findGameExecutable() const;
    Q_INVOKABLE bool launchGame();

signals:
    void gamePathChanged();

private:
    QSettings m_settings;
    QString m_gamePath;
};
