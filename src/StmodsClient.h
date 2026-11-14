#pragma once

#include "ModItem.h"
#include <QNetworkAccessManager>
#include <QNetworkCookieJar>
#include <QObject>
#include <QList>

class StmodsClient : public QObject {
    Q_OBJECT

public:
    explicit StmodsClient(QObject* parent = nullptr);

    void fetchModList(const QString& categorySlug, int page, const QString& sortOrder, bool append);
    void searchMods(const QString& query, int page, bool append);
    void fetchModDetail(const QString& modUrl, const QString& fallbackCategorySlug = {});

signals:
    void modListLoaded(const QList<ModSummary>& mods, bool append, bool hasMore, int currentPage);
    void modListError(const QString& error);
    void modDetailLoaded(const ModDetail& detail);
    void modDetailError(const QString& error);

private:
    QNetworkRequest makeRequest(const QUrl& url) const;
    QString categoryUrl(const QString& slug) const;
    QList<ModSummary> parseModCards(const QString& html, const QString& defaultCategorySlug, bool& hasMore) const;
    ModDetail parseModDetail(const QString& html, const QString& modUrl, const QString& fallbackCategorySlug) const;

    static QString decodeEntities(QString text);
    static QString stripTags(QString html);
    static QString toAbsoluteUrl(const QString& path);

    QNetworkAccessManager m_nam;
    QNetworkReply* m_activeListReply = nullptr;
    QNetworkReply* m_activeDetailReply = nullptr;
};
