#include "DownloadManager.h"
#include <QDir>

DownloadManager::DownloadManager(QObject* parent)
    : QAbstractListModel(parent) {
}

DownloadManager::~DownloadManager() {
    cancelAll();
}

int DownloadManager::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_tasks.count();
}

QVariant DownloadManager::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_tasks.count())
        return {};

    const DownloadTask* t = m_tasks.at(index.row());
    switch (role) {
    case ModIdRole:         return t->modId();
    case TitleRole:         return t->title();
    case ThumbnailUrlRole:  return t->thumbnailUrl();
    case CategorySlugRole:  return t->categorySlug();
    case StatusTextRole:    return t->statusText();
    case ProgressRole:      return t->progress();
    case SpeedMBpsRole:     return t->speedMBps();
    case BytesReceivedRole: return t->bytesReceived();
    case BytesTotalRole:    return t->bytesTotal();
    case StateRole:         return static_cast<int>(t->state());
    case IsDownloadingRole: return t->isDownloading();
    case IsQueuedRole:      return t->isQueued();
    case IsFinishedRole:    return t->isFinished();
    case IsFailedRole:      return t->isFailed();
    case FilePathRole:      return t->filePath();
    case ErrorStringRole:   return t->errorString();
    default:                return {};
    }
}

QHash<int, QByteArray> DownloadManager::roleNames() const {
    return {
        { ModIdRole,         "modId" },
        { TitleRole,         "title" },
        { ThumbnailUrlRole,  "thumbnailUrl" },
        { CategorySlugRole,  "categorySlug" },
        { StatusTextRole,    "statusText" },
        { ProgressRole,      "progress" },
        { SpeedMBpsRole,     "speedMBps" },
        { BytesReceivedRole, "bytesReceived" },
        { BytesTotalRole,    "bytesTotal" },
        { StateRole,         "state" },
        { IsDownloadingRole, "isDownloading" },
        { IsQueuedRole,      "isQueued" },
        { IsFinishedRole,    "isFinished" },
        { IsFailedRole,      "isFailed" },
        { FilePathRole,      "filePath" },
        { ErrorStringRole,   "errorString" }
    };
}

int DownloadManager::activeCount() const {
    int c = 0;
    for (auto* t : m_tasks) {
        if (t->isDownloading() || t->isQueued())
            ++c;
    }
    return c;
}

double DownloadManager::totalSpeedMBps() const {
    double s = 0.0;
    for (auto* t : m_tasks) {
        if (t->isDownloading())
            s += t->speedMBps();
    }
    return s;
}

int DownloadManager::runningCount() const {
    int c = 0;
    for (auto* t : m_tasks) {
        if (t->isDownloading())
            ++c;
    }
    return c;
}

DownloadTask* DownloadManager::findTask(const QString& modId) const {
    for (auto* t : m_tasks) {
        if (t->modId() == modId)
            return t;
    }
    return nullptr;
}

void DownloadManager::startDownload(const QString& modId,
                                    const QString& title,
                                    const QString& thumbnailUrl,
                                    const QString& categorySlug,
                                    const QString& downloadRoute,
                                    const QString& destinationDir,
                                    const QString& fallbackName) {
    DownloadTask* existing = findTask(modId);
    if (existing) {
        if (existing->isDownloading() || existing->isQueued()) {
            return; // Already in progress
        }
        // If it was finished or failed, remove old and start fresh
        int row = m_tasks.indexOf(existing);
        beginRemoveRows(QModelIndex(), row, row);
        m_tasks.removeAt(row);
        endRemoveRows();
        delete existing;
    }

    auto* task = new DownloadTask(modId, title, thumbnailUrl, categorySlug,
                                  downloadRoute, destinationDir, fallbackName, this);

    int newRow = m_tasks.count();
    beginInsertRows(QModelIndex(), newRow, newRow);
    m_tasks.append(task);
    endInsertRows();

    emit countChanged();
    emit activeCountChanged();
    emit modStatusChanged(modId);

    connect(task, &DownloadTask::progressChanged, this, [this, task]() {
        int r = m_tasks.indexOf(task);
        if (r != -1) {
            QModelIndex idx = index(r);
            emit dataChanged(idx, idx);
        }
        emit totalSpeedChanged();
        emit modStatusChanged(task->modId());
    });

    connect(task, &DownloadTask::stateChanged, this, [this, task]() {
        int r = m_tasks.indexOf(task);
        if (r != -1) {
            QModelIndex idx = index(r);
            emit dataChanged(idx, idx);
        }
        emit activeCountChanged();
        emit modStatusChanged(task->modId());
        processQueue();
    });

    connect(task, &DownloadTask::finished, this, [this, task](const QString& filePath) {
        emit downloadCompleted(task->modId(), task->title(), filePath);
        processQueue();
    });

    connect(task, &DownloadTask::failed, this, [this, task](const QString& error) {
        emit downloadFailed(task->modId(), task->title(), error);
        processQueue();
    });

    processQueue();
}

void DownloadManager::processQueue() {
    int running = runningCount();
    if (running >= kMaxConcurrent)
        return;

    for (auto* t : m_tasks) {
        if (t->isQueued()) {
            t->start();
            running++;
            if (running >= kMaxConcurrent)
                break;
        }
    }
}

void DownloadManager::cancelDownload(const QString& modId) {
    DownloadTask* t = findTask(modId);
    if (t) {
        t->cancel();
        emit modStatusChanged(modId);
        emit activeCountChanged();
        processQueue();
    }
}

void DownloadManager::cancelAll() {
    for (auto* t : m_tasks) {
        t->cancel();
    }
    emit activeCountChanged();
}

void DownloadManager::clearFinished() {
    for (int i = m_tasks.count() - 1; i >= 0; --i) {
        DownloadTask* t = m_tasks.at(i);
        if (t->isFinished() || t->state() == DownloadTask::Canceled) {
            beginRemoveRows(QModelIndex(), i, i);
            m_tasks.removeAt(i);
            endRemoveRows();
            delete t;
        }
    }
    emit countChanged();
    emit activeCountChanged();
}

QVariantMap DownloadManager::getModStatus(const QString& modId) const {
    DownloadTask* t = findTask(modId);
    QVariantMap map;
    if (!t) {
        map["isDownloading"] = false;
        map["isQueued"] = false;
        map["isFinished"] = false;
        map["isFailed"] = false;
        map["progress"] = 0.0;
        map["speedMBps"] = 0.0;
        map["statusText"] = QString();
        map["filePath"] = QString();
        return map;
    }

    map["isDownloading"] = t->isDownloading();
    map["isQueued"] = t->isQueued();
    map["isFinished"] = t->isFinished();
    map["isFailed"] = t->isFailed();
    map["progress"] = t->progress();
    map["speedMBps"] = t->speedMBps();
    map["bytesReceived"] = t->bytesReceived();
    map["bytesTotal"] = t->bytesTotal();
    map["statusText"] = t->statusText();
    map["filePath"] = t->filePath();
    map["errorString"] = t->errorString();
    return map;
}

bool DownloadManager::isDownloading(const QString& modId) const {
    DownloadTask* t = findTask(modId);
    return t && t->isDownloading();
}

bool DownloadManager::isQueued(const QString& modId) const {
    DownloadTask* t = findTask(modId);
    return t && t->isQueued();
}

bool DownloadManager::isFinished(const QString& modId) const {
    DownloadTask* t = findTask(modId);
    return t && t->isFinished();
}
