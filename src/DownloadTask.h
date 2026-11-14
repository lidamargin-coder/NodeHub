#pragma once

#include <QElapsedTimer>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QObject>
#include <QString>
#include <QUrl>

class DownloadTask : public QObject {
    Q_OBJECT

public:
    enum State {
        Queued,
        Resolving,
        Downloading,
        Finished,
        Failed,
        Canceled
    };
    Q_ENUM(State)

    DownloadTask(const QString& modId,
                 const QString& title,
                 const QString& thumbnailUrl,
                 const QString& categorySlug,
                 const QString& downloadRoute,
                 const QString& destinationDir,
                 const QString& fallbackName,
                 QObject* parent = nullptr);
    ~DownloadTask() override;

    QString modId() const { return m_modId; }
    QString title() const { return m_title; }
    QString thumbnailUrl() const { return m_thumbnailUrl; }
    QString categorySlug() const { return m_categorySlug; }
    QString downloadRoute() const { return m_downloadRoute; }
    QString destinationDir() const { return m_destDir; }
    QString statusText() const { return m_statusText; }
    double progress() const { return m_progress; }
    double speedMBps() const { return m_speedMBps; }
    qint64 bytesReceived() const { return m_bytesReceived; }
    qint64 bytesTotal() const { return m_bytesTotal; }
    State state() const { return m_state; }
    QString filePath() const { return m_finalFilePath; }
    QString errorString() const { return m_errorString; }

    bool isDownloading() const { return m_state == Downloading || m_state == Resolving; }
    bool isQueued() const { return m_state == Queued; }
    bool isFinished() const { return m_state == Finished; }
    bool isFailed() const { return m_state == Failed; }

    void start();
    void cancel();

signals:
    void progressChanged();
    void stateChanged();
    void finished(const QString& filePath);
    void failed(const QString& error);

private:
    QNetworkRequest makeRequest(const QUrl& url, const QString& referer = {}) const;

    void step1ResolveStmodsRedirect(const QUrl& url);
    void step2FetchModsfirePage(const QUrl& modsfireUrl);
    void step3FetchGenerateLink(const QUrl& genUrl, const QUrl& referer);
    void step4ResolveFinalUrl(const QUrl& dlButtonUrl, const QUrl& referer);
    void step5StreamFile(const QUrl& directUrl, const QUrl& referer);

    void setStatus(const QString& text);
    void fail(const QString& error);
    void cleanupFile(bool deletePart);
    static QString sanitizeFilename(QString name);
    static QString extractFilenameFromUrl(const QUrl& url);

    QString m_modId;
    QString m_title;
    QString m_thumbnailUrl;
    QString m_categorySlug;
    QString m_downloadRoute;
    QString m_destDir;
    QString m_fallbackName;

    QNetworkAccessManager m_nam;
    QNetworkReply* m_reply = nullptr;
    QFile m_file;
    QElapsedTimer m_timer;

    QString m_detectedFilename;
    QString m_partFilePath;
    QString m_finalFilePath;
    QString m_errorString;

    State m_state = Queued;
    double m_progress = 0.0;
    QString m_statusText = QStringLiteral("В очереди...");
    qint64 m_bytesReceived = 0;
    qint64 m_bytesTotal = 0;
    double m_speedMBps = 0.0;
};
