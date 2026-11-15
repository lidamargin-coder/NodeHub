#include "AppController.h"
#include <QDesktopServices>
#include <QDir>
#include <QUrl>

AppController::AppController(QObject* parent)
    : QObject(parent) {
    connect(&m_client, &StmodsClient::modListLoaded, this,
            [this](const QList<ModSummary>& mods, bool append, bool hasMore, int currentPage) {
                setLoadingList(false);
                m_currentPage = currentPage;
                if (m_hasMore != hasMore) {
                    m_hasMore = hasMore;
                    emit hasMoreChanged();
                }

                if (append) {
                    m_modModel.appendMods(mods);
                } else {
                    m_modModel.setMods(mods);
                    if (!mods.isEmpty() && m_selectedModId.isEmpty()) {
                        const auto& first = mods.first();
                        selectMod(first.id, first.url, first.categorySlug);
                    }
                }
            });

    connect(&m_client, &StmodsClient::modListError, this, [this](const QString& error) {
        setLoadingList(false);
        showNotification(error, true);
    });

    connect(&m_client, &StmodsClient::modDetailLoaded, this, [this](const ModDetail& detail) {
        setLoadingDetail(false);
        m_selectedModDetail = detail.toVariantMap();
        emit selectedModDetailChanged();
    });

    connect(&m_client, &StmodsClient::modDetailError, this, [this](const QString& error) {
        setLoadingDetail(false);
        showNotification(error, true);
    });

    connect(&m_settings, &SettingsManager::gamePathChanged, this, [this]() {
        m_installedMods.rescan(m_settings.gamePath());
    });

    connect(&m_downloadManager, &DownloadManager::downloadCompleted, this,
            [this](const QString& modId, const QString& title, const QString& filePath) {
                QString catSlug = m_selectedModDetail.value("categorySlug", "cars").toString();
                QString thumb = m_selectedModDetail.value("thumbnailUrl", "").toString();
                QString ver = m_selectedModDetail.value("version", "").toString();
                m_installedMods.recordInstalledMod(modId, title, catSlug, filePath, thumb, ver);
                showNotification(QStringLiteral("Мод «%1» успешно установлен!").arg(title), false);
            });

    connect(&m_downloadManager, &DownloadManager::downloadFailed, this,
            [this](const QString& modId, const QString& title, const QString& error) {
                Q_UNUSED(modId);
                showNotification(QStringLiteral("Ошибка установки «%1»:\n%2").arg(title, error), true);
            });

    connect(&m_updateManager, &UpdateManager::messageRequested, this,
            [this](const QString& text, bool isError) {
                showNotification(text, isError);
            });
}

void AppController::initialize() {
    qDebug() << "AppController::initialize gamePath=" << m_settings.gamePath() << "hasGamePath=" << m_settings.hasGamePath();
    if (m_settings.hasGamePath()) {
        m_installedMods.rescan(m_settings.gamePath());
    }
    selectCategory("cars");
    if (!m_settings.hasGamePath()) {
        emit requestFolderSetup();
    }

    // Auto-check for launcher updates on every startup silently
    m_updateManager.checkUpdates(true);
}

void AppController::selectCategory(const QString& slug) {
    if (m_currentCategory != slug || !m_searchQuery.isEmpty()) {
        m_currentCategory = slug;
        emit currentCategoryChanged();
    }
    if (!m_searchQuery.isEmpty()) {
        m_searchQuery.clear();
        emit searchQueryChanged();
    }

    m_currentPage = 1;
    m_selectedModId.clear();

    if (slug == "installed") {
        m_installedMods.rescan(m_settings.gamePath());
        setLoadingList(false);
        return;
    }

    setLoadingList(true);
    m_client.fetchModList(m_currentCategory, 1, m_currentSort, false);
}

void AppController::setSort(const QString& sort) {
    if (m_currentSort == sort)
        return;

    m_currentSort = sort;
    emit currentSortChanged();

    if (!m_searchQuery.isEmpty()) {
        return; // STMods search doesn't support order_by
    }

    m_currentPage = 1;
    m_selectedModId.clear();
    setLoadingList(true);
    m_client.fetchModList(m_currentCategory, 1, m_currentSort, false);
}

void AppController::search(const QString& query) {
    QString trimmed = query.trimmed();
    if (trimmed.isEmpty()) {
        selectCategory(m_currentCategory);
        return;
    }

    m_searchQuery = trimmed;
    emit searchQueryChanged();

    m_currentPage = 1;
    m_selectedModId.clear();
    setLoadingList(true);
    m_client.searchMods(m_searchQuery, 1, false);
}

void AppController::loadMore() {
    if (m_loadingList || !m_hasMore)
        return;

    setLoadingList(true);
    int nextPage = m_currentPage + 1;
    if (!m_searchQuery.isEmpty()) {
        m_client.searchMods(m_searchQuery, nextPage, true);
    } else {
        m_client.fetchModList(m_currentCategory, nextPage, m_currentSort, true);
    }
}

void AppController::selectMod(const QString& modId, const QString& modUrl, const QString& categorySlug) {
    if (m_selectedModId == modId && !m_selectedModDetail.isEmpty())
        return;

    m_selectedModId = modId;
    emit selectedModChanged();

    setLoadingDetail(true);
    m_client.fetchModDetail(modUrl, categorySlug);
}

void AppController::installSelectedMod() {
    if (!m_settings.hasGamePath()) {
        emit requestFolderSetup();
        return;
    }

    if (m_selectedModDetail.isEmpty())
        return;

    QString downloadRoute = m_selectedModDetail.value("downloadRoute").toString();
    if (downloadRoute.isEmpty()) {
        showNotification(QStringLiteral("Ссылка на загрузку не найдена"), true);
        return;
    }

    QString modId = m_selectedModDetail.value("id").toString();
    QString title = m_selectedModDetail.value("title").toString();
    QString category = m_selectedModDetail.value("categorySlug").toString();
    if (category.isEmpty()) {
        category = m_currentCategory;
    }

    QString targetDir = m_settings.targetFolderForCategory(category);
    QString fallbackName = title;

    QString thumb = "";
    auto gallery = m_selectedModDetail.value("galleryUrls").toStringList();
    if (!gallery.isEmpty()) {
        thumb = gallery.first();
    }

    m_downloadManager.startDownload(modId, title, thumb, category, downloadRoute, targetDir, fallbackName);
}

void AppController::cancelModDownload(const QString& modId) {
    m_downloadManager.cancelDownload(modId);
}

void AppController::clearNotification() {
    if (!m_notificationText.isEmpty()) {
        m_notificationText.clear();
        m_notificationError = false;
        emit notificationChanged();
    }
}

void AppController::openTargetFolderForSelected() {
    QString category = m_selectedModDetail.value("categorySlug").toString();
    if (category.isEmpty()) {
        category = m_currentCategory;
    }
    openTargetFolderForCategory(category);
}

void AppController::openTargetFolderForCategory(const QString& categorySlug) {
    QString folder = m_settings.targetFolderForCategory(categorySlug);
    QDir dir(folder);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(dir.absolutePath()));
}

void AppController::setLoadingList(bool loading) {
    if (m_loadingList != loading) {
        m_loadingList = loading;
        emit loadingListChanged();
    }
}

void AppController::setLoadingDetail(bool loading) {
    if (m_loadingDetail != loading) {
        m_loadingDetail = loading;
        emit loadingDetailChanged();
    }
}

void AppController::showNotification(const QString& text, bool isError) {
    m_notificationText = text;
    m_notificationError = isError;
    emit notificationChanged();
}

void AppController::launchGame() {
    if (!m_settings.hasGamePath()) {
        emit requestFolderSetup();
        return;
    }
    if (m_settings.launchGame()) {
        showNotification(QStringLiteral("Запуск BeamNG.drive..."), false);
    } else {
        showNotification(QStringLiteral("Не удалось запустить игру. Проверьте правильность папки BeamNG.drive."), true);
    }
}

void AppController::rescanInstalledMods() {
    m_installedMods.rescan(m_settings.gamePath());
}

bool AppController::isModInstalled(const QString& modId, const QString& title, const QString& filename) const {
    return m_installedMods.isModInstalled(modId, title, filename);
}

bool AppController::isModEnabled(const QString& modIdOrFilename) const {
    return m_installedMods.isModEnabled(modIdOrFilename);
}

bool AppController::toggleInstalledMod(const QString& filePath) {
    bool ok = m_installedMods.toggleMod(filePath);
    if (ok) {
        bool enabled = m_installedMods.isModEnabled(filePath);
        showNotification(enabled ? QStringLiteral("Мод включен") : QStringLiteral("Мод отключен"), false);
    }
    return ok;
}

bool AppController::deleteInstalledMod(const QString& filePath) {
    bool ok = m_installedMods.deleteMod(filePath);
    if (ok) {
        showNotification(QStringLiteral("Мод удален из игры"), false);
    }
    return ok;
}

void AppController::openInstalledModFolder(const QString& filePath) {
    m_installedMods.openModFolder(filePath);
}
