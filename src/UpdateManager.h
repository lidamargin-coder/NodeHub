#pragma once

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QUrl>

class UpdateManager : public QObject {
    Q_OBJECT

    Q_PROPERTY(QString currentVersion READ currentVersion CONSTANT)
    Q_PROPERTY(QString newVersion READ newVersion NOTIFY newVersionChanged)
    Q_PROPERTY(QString releaseNotes READ releaseNotes NOTIFY releaseNotesChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusTextChanged)
    Q_PROPERTY(int progressPercent READ progressPercent NOTIFY progressChanged)
    Q_PROPERTY(qint64 bytesReceived READ bytesReceived NOTIFY progressChanged)
    Q_PROPERTY(qint64 bytesTotal READ bytesTotal NOTIFY progressChanged)
    Q_PROPERTY(QString state READ state NOTIFY stateChanged)
    Q_PROPERTY(bool isUpdating READ isUpdating NOTIFY stateChanged)

public:
    explicit UpdateManager(QObject* parent = nullptr);
    ~UpdateManager() override;

    QString currentVersion() const;
    QString newVersion() const { return m_newVersion; }
    QString releaseNotes() const { return m_releaseNotes; }
    QString statusText() const { return m_statusText; }
    int progressPercent() const { return m_progressPercent; }
    qint64 bytesReceived() const { return m_bytesReceived; }
    qint64 bytesTotal() const { return m_bytesTotal; }
    QString state() const { return m_state; } // "idle", "checking", "downloading", "ready", "up_to_date", "error"
    bool isUpdating() const {
        return m_state == "checking" || m_state == "downloading" || m_state == "ready";
    }

    Q_INVOKABLE void checkUpdates(bool silent = true);
    Q_INVOKABLE void cancelUpdate();
    Q_INVOKABLE void installAndRestart();

signals:
    void newVersionChanged();
    void releaseNotesChanged();
    void statusTextChanged();
    void progressChanged();
    void stateChanged();
    void updateReady(const QString& version, const QString& installerPath);
    void messageRequested(const QString& text, bool isError);

private slots:
    void onCheckFinished();
    void onDownloadReadyRead();
    void onDownloadProgress(qint64 received, qint64 total);
    void onDownloadFinished();

private:
    void setState(const QString& newState);
    void setStatus(const QString& text);
    static bool isRemoteNewer(const QString& currentVer, const QString& remoteVer);
    void startDownload(const QUrl& url, qint64 expectedSize);

    QNetworkAccessManager m_nam;
    QNetworkReply* m_checkReply = nullptr;
    QNetworkReply* m_downloadReply = nullptr;
    QFile* m_downloadFile = nullptr;

    QString m_newVersion;
    QString m_releaseNotes;
    QString m_downloadUrl;
    QString m_downloadedFilePath;
    QString m_statusText;
    QString m_state = "idle";
    int m_progressPercent = 0;
    qint64 m_bytesReceived = 0;
    qint64 m_bytesTotal = 0;
    bool m_silentCheck = true;
};
