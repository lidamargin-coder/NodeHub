#pragma once

#include "DownloadManager.h"
#include "InstalledModsManager.h"
#include "ModListModel.h"
#include "SettingsManager.h"
#include "StmodsClient.h"
#include <QObject>
#include <QVariantMap>

class AppController : public QObject {
    Q_OBJECT
    Q_PROPERTY(ModListModel* modModel READ modModel CONSTANT)
    Q_PROPERTY(SettingsManager* settings READ settings CONSTANT)
    Q_PROPERTY(DownloadManager* downloadManager READ downloadManager CONSTANT)
    Q_PROPERTY(InstalledModsManager* installedMods READ installedMods CONSTANT)

    Q_PROPERTY(QString currentCategory READ currentCategory NOTIFY currentCategoryChanged)
    Q_PROPERTY(QString currentSort READ currentSort WRITE setSort NOTIFY currentSortChanged)
    Q_PROPERTY(QString searchQuery READ searchQuery NOTIFY searchQueryChanged)
    Q_PROPERTY(bool loadingList READ loadingList NOTIFY loadingListChanged)
    Q_PROPERTY(bool loadingDetail READ loadingDetail NOTIFY loadingDetailChanged)
    Q_PROPERTY(bool hasMore READ hasMore NOTIFY hasMoreChanged)

    Q_PROPERTY(QString selectedModId READ selectedModId NOTIFY selectedModChanged)
    Q_PROPERTY(QVariantMap selectedModDetail READ selectedModDetail NOTIFY selectedModDetailChanged)
    Q_PROPERTY(QString notificationText READ notificationText NOTIFY notificationChanged)
    Q_PROPERTY(bool notificationError READ notificationError NOTIFY notificationChanged)

public:
    explicit AppController(QObject* parent = nullptr);

    ModListModel* modModel() { return &m_modModel; }
    SettingsManager* settings() { return &m_settings; }
    DownloadManager* downloadManager() { return &m_downloadManager; }
    InstalledModsManager* installedMods() { return &m_installedMods; }

    QString currentCategory() const { return m_currentCategory; }
    QString currentSort() const { return m_currentSort; }
    QString searchQuery() const { return m_searchQuery; }
    bool loadingList() const { return m_loadingList; }
    bool loadingDetail() const { return m_loadingDetail; }
    bool hasMore() const { return m_hasMore; }
    QString selectedModId() const { return m_selectedModId; }
    QVariantMap selectedModDetail() const { return m_selectedModDetail; }
    QString notificationText() const { return m_notificationText; }
    bool notificationError() const { return m_notificationError; }

    Q_INVOKABLE void initialize();
    Q_INVOKABLE void selectCategory(const QString& slug);
    Q_INVOKABLE void setSort(const QString& sort);
    Q_INVOKABLE void search(const QString& query);
    Q_INVOKABLE void loadMore();
    Q_INVOKABLE void selectMod(const QString& modId, const QString& modUrl, const QString& categorySlug);
    Q_INVOKABLE void installSelectedMod();
    Q_INVOKABLE void cancelModDownload(const QString& modId);
    Q_INVOKABLE void clearNotification();
    Q_INVOKABLE void openTargetFolderForSelected();
    Q_INVOKABLE void openTargetFolderForCategory(const QString& categorySlug);

    // Installed mods & Game launcher controls
    Q_INVOKABLE void launchGame();
    Q_INVOKABLE void rescanInstalledMods();
    Q_INVOKABLE bool isModInstalled(const QString& modId,
                                    const QString& title = QString(),
                                    const QString& filename = QString()) const;
    Q_INVOKABLE bool isModEnabled(const QString& modIdOrFilename) const;
    Q_INVOKABLE bool toggleInstalledMod(const QString& filePath);
    Q_INVOKABLE bool deleteInstalledMod(const QString& filePath);
    Q_INVOKABLE void openInstalledModFolder(const QString& filePath);

signals:
    void currentCategoryChanged();
    void currentSortChanged();
    void searchQueryChanged();
    void loadingListChanged();
    void loadingDetailChanged();
    void hasMoreChanged();
    void selectedModChanged();
    void selectedModDetailChanged();
    void notificationChanged();
    void requestFolderSetup();

private:
    void setLoadingList(bool loading);
    void setLoadingDetail(bool loading);
    void showNotification(const QString& text, bool isError = false);

    ModListModel m_modModel;
    SettingsManager m_settings;
    StmodsClient m_client;
    DownloadManager m_downloadManager;
    InstalledModsManager m_installedMods;

    QString m_currentCategory = "cars";
    QString m_currentSort = "newest"; // "newest", "popularity", "downloads"
    QString m_searchQuery;
    int m_currentPage = 1;
    bool m_loadingList = false;
    bool m_loadingDetail = false;
    bool m_hasMore = false;

    QString m_selectedModId;
    QVariantMap m_selectedModDetail;
    QString m_notificationText;
    bool m_notificationError = false;
};
