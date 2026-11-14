#pragma once

#include <QAbstractListModel>
#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QSet>
#include <QString>
#include <QVariantMap>

struct InstalledModItem {
    QString id;             // Unique key, e.g. clean filename without extension
    QString modId;          // STMods mod id/slug if known, e.g. "/beamng_drive/mods/..."
    QString title;          // Human-readable title
    QString filename;       // e.g. "volvo_xc70_us_2002_v1.0.zip"
    QString filePath;       // Absolute path on disk
    QString categorySlug;   // "cars", "maps", "trucks", etc.
    QString categoryName;   // "Легковые авто", "Карты", etc.
    qint64 fileSizeBytes = 0;
    QString fileSizeText;   // e.g. "61.8 МБ"
    bool enabled = true;    // true if .zip, false if .zip.disabled
    QString thumbnailUrl;   // Preview image URL if known
    QString version;
    QDateTime installedAt;
};

class InstalledModsManager : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int totalInstalledCount READ totalInstalledCount NOTIFY countChanged)
    Q_PROPERTY(int enabledCount READ enabledCount NOTIFY countChanged)
    Q_PROPERTY(QString totalSizeText READ totalSizeText NOTIFY countChanged)
    Q_PROPERTY(bool isScanning READ isScanning NOTIFY isScanningChanged)
    Q_PROPERTY(QString categoryFilter READ categoryFilter WRITE setCategoryFilter NOTIFY filterChanged)
    Q_PROPERTY(QString searchQuery READ searchQuery WRITE setSearchQuery NOTIFY filterChanged)
    Q_PROPERTY(int filteredCount READ filteredCount NOTIFY filterChanged)

public:
    enum InstalledRoles {
        IdRole = Qt::UserRole + 1,
        ModIdRole,
        TitleRole,
        FilenameRole,
        FilePathRole,
        CategorySlugRole,
        CategoryNameRole,
        FileSizeBytesRole,
        FileSizeTextRole,
        IsEnabledRole,
        ThumbnailUrlRole,
        VersionRole,
        InstalledDateRole
    };

    explicit InstalledModsManager(QObject* parent = nullptr);
    ~InstalledModsManager() override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    int totalInstalledCount() const { return m_items.count(); }
    int enabledCount() const;
    QString totalSizeText() const;
    bool isScanning() const { return m_isScanning; }

    QString categoryFilter() const { return m_categoryFilter; }
    void setCategoryFilter(const QString& slug);

    QString searchQuery() const { return m_searchQuery; }
    void setSearchQuery(const QString& query);

    int filteredCount() const { return m_filteredIndices.count(); }

    Q_INVOKABLE int countForCategory(const QString& catSlug) const;
    Q_INVOKABLE void rescan(const QString& gamePath);
    Q_INVOKABLE bool isModInstalled(const QString& modId,
                                    const QString& title = QString(),
                                    const QString& filename = QString()) const;
    Q_INVOKABLE bool isModEnabled(const QString& modIdOrFilename) const;
    Q_INVOKABLE QVariantMap getModInfo(const QString& modIdOrFilename) const;

    Q_INVOKABLE bool toggleMod(const QString& filePath);
    Q_INVOKABLE bool deleteMod(const QString& filePath);
    Q_INVOKABLE void openModFolder(const QString& filePath);

    Q_INVOKABLE void recordInstalledMod(const QString& modId,
                                        const QString& title,
                                        const QString& categorySlug,
                                        const QString& archivePath,
                                        const QString& thumbnailUrl,
                                        const QString& version = QString());

signals:
    void countChanged();
    void filterChanged();
    void isScanningChanged();
    void modInstalled(const QString& modId, const QString& title);
    void modUninstalled(const QString& modId, const QString& title);
    void modToggled(const QString& filePath, bool enabled);

private:
    void loadRegistry();
    void saveRegistry();
    void updateFilter();
    static QString formatSize(qint64 bytes);
    static QString cleanModTitle(const QString& filename);
    static QString normalizeString(const QString& str);

    QList<InstalledModItem> m_items;
    QList<int> m_filteredIndices;
    QString m_categoryFilter = "all";
    QString m_searchQuery;
    bool m_isScanning = false;
    QString m_lastGamePath;
};
