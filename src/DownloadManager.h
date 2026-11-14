#pragma once

#include "DownloadTask.h"
#include <QAbstractListModel>
#include <QList>
#include <QVariantMap>

class DownloadManager : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int activeCount READ activeCount NOTIFY activeCountChanged)
    Q_PROPERTY(int totalCount READ totalCount NOTIFY countChanged)
    Q_PROPERTY(double totalSpeedMBps READ totalSpeedMBps NOTIFY totalSpeedChanged)

public:
    enum Roles {
        ModIdRole = Qt::UserRole + 1,
        TitleRole,
        ThumbnailUrlRole,
        CategorySlugRole,
        StatusTextRole,
        ProgressRole,
        SpeedMBpsRole,
        BytesReceivedRole,
        BytesTotalRole,
        StateRole,
        IsDownloadingRole,
        IsQueuedRole,
        IsFinishedRole,
        IsFailedRole,
        FilePathRole,
        ErrorStringRole
    };

    explicit DownloadManager(QObject* parent = nullptr);
    ~DownloadManager() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int activeCount() const;
    int totalCount() const { return m_tasks.count(); }
    double totalSpeedMBps() const;

    Q_INVOKABLE void startDownload(const QString& modId,
                                   const QString& title,
                                   const QString& thumbnailUrl,
                                   const QString& categorySlug,
                                   const QString& downloadRoute,
                                   const QString& destinationDir,
                                   const QString& fallbackName);

    Q_INVOKABLE void cancelDownload(const QString& modId);
    Q_INVOKABLE void cancelAll();
    Q_INVOKABLE void clearFinished();
    Q_INVOKABLE QVariantMap getModStatus(const QString& modId) const;
    Q_INVOKABLE bool isDownloading(const QString& modId) const;
    Q_INVOKABLE bool isQueued(const QString& modId) const;
    Q_INVOKABLE bool isFinished(const QString& modId) const;

signals:
    void activeCountChanged();
    void countChanged();
    void totalSpeedChanged();
    void downloadCompleted(const QString& modId, const QString& title, const QString& filePath);
    void downloadFailed(const QString& modId, const QString& title, const QString& error);
    void modStatusChanged(const QString& modId);

private:
    void processQueue();
    int runningCount() const;
    DownloadTask* findTask(const QString& modId) const;

    QList<DownloadTask*> m_tasks;
    static constexpr int kMaxConcurrent = 5;
};
