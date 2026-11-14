#include "StmodsClient.h"
#include <QNetworkReply>
#include <QRegularExpression>
#include <QUrlQuery>

namespace {
const char* kUserAgent =
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
    "(KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36";
}

StmodsClient::StmodsClient(QObject* parent)
    : QObject(parent) {
    m_nam.setCookieJar(new QNetworkCookieJar(&m_nam));
}

QNetworkRequest StmodsClient::makeRequest(const QUrl& url) const {
    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", kUserAgent);
    req.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    req.setRawHeader("Accept-Language", "ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    return req;
}

QString StmodsClient::categoryUrl(const QString& slug) const {
    if (slug == "cars")   return "https://stmods.org/beamng_drive/cars/";
    if (slug == "jeeps")  return "https://stmods.org/beamng_drive/jeeps/";
    if (slug == "trucks") return "https://stmods.org/beamng_drive/trucks/";
    if (slug == "maps")   return "https://stmods.org/beamng_drive/maps/";
    if (slug == "racing") return "https://stmods.org/beamng_drive/racing/";
    if (slug == "others") return "https://stmods.org/beamng_drive/others/";
    return "https://stmods.org/beamng_drive/mods/";
}

void StmodsClient::fetchModList(const QString& categorySlug, int page, const QString& sortOrder, bool append) {
    if (m_activeListReply) {
        QNetworkReply* old = m_activeListReply;
        m_activeListReply = nullptr;
        old->disconnect(this);
        old->abort();
        old->deleteLater();
    }

    QUrl url(categoryUrl(categorySlug));
    if (page > 1) {
        QUrlQuery q;
        q.addQueryItem("page", QString::number(page));
        url.setQuery(q);
    }

    QNetworkRequest req = makeRequest(url);
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QByteArray postData = "per_page=20";
    if (sortOrder == "downloads") {
        postData += "&order_by=downloads+DESC";
    } else if (sortOrder == "popularity") {
        postData += "&order_by=views+DESC";
    } else {
        postData += "&order_by=date_public+DESC";
    }

    QNetworkReply* reply = m_nam.post(req, postData);
    m_activeListReply = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply, categorySlug, page, append]() {
        if (m_activeListReply == reply)
            m_activeListReply = nullptr;

        if (reply->error() == QNetworkReply::OperationCanceledError) {
            reply->deleteLater();
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            emit modListError(QStringLiteral("Ошибка загрузки списка: %1").arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        QString html = QString::fromUtf8(reply->readAll());
        reply->deleteLater();

        bool hasMore = false;
        QList<ModSummary> mods = parseModCards(html, categorySlug, hasMore);
        emit modListLoaded(mods, append, hasMore, page);
    });
}

void StmodsClient::searchMods(const QString& query, int page, bool append) {
    if (m_activeListReply) {
        QNetworkReply* old = m_activeListReply;
        m_activeListReply = nullptr;
        old->disconnect(this);
        old->abort();
        old->deleteLater();
    }

    QUrl url("https://stmods.org/search.php");
    QUrlQuery q;
    q.addQueryItem("search_request", query);
    q.addQueryItem("game", "beamng_drive");
    if (page > 1) {
        q.addQueryItem("page", QString::number(page));
    }
    url.setQuery(q);

    QNetworkReply* reply = m_nam.get(makeRequest(url));
    m_activeListReply = reply;

    connect(reply, &QNetworkReply::finished, this, [this, reply, page, append]() {
        if (m_activeListReply == reply)
            m_activeListReply = nullptr;

        if (reply->error() == QNetworkReply::OperationCanceledError) {
            reply->deleteLater();
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            emit modListError(QStringLiteral("Ошибка поиска: %1").arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        QString html = QString::fromUtf8(reply->readAll());
        reply->deleteLater();

        bool hasMore = false;
        QList<ModSummary> mods = parseModCards(html, "all", hasMore);
        emit modListLoaded(mods, append, hasMore, page);
    });
}

void StmodsClient::fetchModDetail(const QString& modUrl, const QString& fallbackCategorySlug) {
    if (m_activeDetailReply) {
        QNetworkReply* old = m_activeDetailReply;
        m_activeDetailReply = nullptr;
        old->disconnect(this);
        old->abort();
        old->deleteLater();
    }

    QNetworkReply* reply = m_nam.get(makeRequest(QUrl(modUrl)));
    m_activeDetailReply = reply;

    connect(reply, &QNetworkReply::finished, this, [this, reply, modUrl, fallbackCategorySlug]() {
        if (m_activeDetailReply == reply)
            m_activeDetailReply = nullptr;

        if (reply->error() == QNetworkReply::OperationCanceledError) {
            reply->deleteLater();
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            emit modDetailError(QStringLiteral("Ошибка загрузки описания мода: %1").arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        QString html = QString::fromUtf8(reply->readAll());
        reply->deleteLater();

        ModDetail detail = parseModDetail(html, modUrl, fallbackCategorySlug);
        emit modDetailLoaded(detail);
    });
}

QList<ModSummary> StmodsClient::parseModCards(const QString& html, const QString& defaultCategorySlug, bool& hasMore) const {
    QList<ModSummary> list;
    hasMore = false;

    const QString marker = "class=\"row index-card\"";
    int pos = html.indexOf(marker);
    while (pos != -1) {
        int nextPos = html.indexOf(marker, pos + marker.size());
        QString chunk = (nextPos == -1) ? html.mid(pos, 12000) : html.mid(pos, nextPos - pos);
        pos = nextPos;

        ModSummary mod;
        mod.categorySlug = defaultCategorySlug;

        // URL
        static QRegularExpression reUrl(R"re(class="[^"]*index-card-img[^"]*"[^>]*href="([^"]+)")re");
        static QRegularExpression reUrlAlt(R"re(href="(/beamng_drive/mods/[^"]+)")re");
        auto mUrl = reUrl.match(chunk);
        if (!mUrl.hasMatch()) mUrl = reUrlAlt.match(chunk);
        if (mUrl.hasMatch()) {
            mod.id = mUrl.captured(1);
            mod.url = toAbsoluteUrl(mod.id);
        } else {
            continue;
        }

        // Thumbnail
        static QRegularExpression reImg(R"(background-image:\s*url\(['"]?([^'")]+)['"]?\))");
        auto mImg = reImg.match(chunk);
        if (mImg.hasMatch()) {
            mod.thumbnailUrl = toAbsoluteUrl(mImg.captured(1));
        }

        // Title and Version/GameVersion inside <h3><b>Title</b> <small>версия X для BeamNG.drive (vY)</small></h3>
        static QRegularExpression reTitle(R"(<h3[^>]*>\s*<b>(.*?)</b>\s*(?:<small>(.*?)</small>)?\s*</h3>)",
                                          QRegularExpression::DotMatchesEverythingOption);
        auto mTitle = reTitle.match(chunk);
        if (mTitle.hasMatch()) {
            mod.title = stripTags(mTitle.captured(1)).trimmed();
            QString sub = stripTags(mTitle.captured(2)).trimmed();

            static QRegularExpression reVer(R"(версия\s+([^\s,]+))", QRegularExpression::CaseInsensitiveOption);
            auto mVer = reVer.match(sub);
            if (mVer.hasMatch()) mod.version = mVer.captured(1);

            static QRegularExpression reGame1(R"(\((v?[\d\.x]+)\))", QRegularExpression::CaseInsensitiveOption);
            static QRegularExpression reGame2(R"(BeamNG(?:\.drive)?\s+\(?(v?[\d\.x]+)\)?)", QRegularExpression::CaseInsensitiveOption);
            static QRegularExpression reGame3(R"(\b(v0\.\d+(?:\.[\dx]+)?)\b)", QRegularExpression::CaseInsensitiveOption);

            auto mG = reGame1.match(sub);
            if (!mG.hasMatch()) mG = reGame2.match(sub);
            if (!mG.hasMatch()) mG = reGame3.match(sub);
            if (!mG.hasMatch()) mG = reGame3.match(chunk);

            if (mG.hasMatch()) {
                QString gv = mG.captured(1).trimmed();
                if (!gv.startsWith('v') && !gv.startsWith('V')) {
                    gv = "v" + gv;
                }
                mod.gameVersion = gv;
            }
        }

        // Fallback title if <h3> didn't match cleanly
        if (mod.title.isEmpty()) {
            static QRegularExpression reH3(R"(<h3[^>]*>(.*?)</h3>)", QRegularExpression::DotMatchesEverythingOption);
            auto mH3 = reH3.match(chunk);
            if (mH3.hasMatch()) mod.title = stripTags(mH3.captured(1)).trimmed();
        }

        // Author
        static QRegularExpression reAuthor(R"(class="index-card-author"[^>]*>.*?<a[^>]*>(.*?)</a>)",
                                           QRegularExpression::DotMatchesEverythingOption);
        auto mAuthor = reAuthor.match(chunk);
        if (mAuthor.hasMatch()) {
            mod.author = stripTags(mAuthor.captured(1)).trimmed();
        } else {
            static QRegularExpression reAuthorPlain(R"(class="index-card-author"[^>]*>(.*?)</div>)",
                                                    QRegularExpression::DotMatchesEverythingOption);
            auto mAP = reAuthorPlain.match(chunk);
            if (mAP.hasMatch()) {
                QString txt = stripTags(mAP.captured(1)).trimmed();
                txt.remove(QRegularExpression("^Автор:\\s*"));
                mod.author = txt.trimmed();
            }
        }
        if (mod.author.isEmpty()) mod.author = QStringLiteral("Неизвестен");

        // Date
        static QRegularExpression reDate(R"(class="[^"]*index-card-date[^"]*"[^>]*>(.*?)</small>)",
                                         QRegularExpression::DotMatchesEverythingOption);
        auto mDate = reDate.match(chunk);
        if (mDate.hasMatch()) {
            mod.date = stripTags(mDate.captured(1)).trimmed();
        }

        // Snippet
        static QRegularExpression reText(R"(class="[^"]*index-card-text[^"]*"[^>]*>(.*?)</div>)",
                                         QRegularExpression::DotMatchesEverythingOption);
        auto mText = reText.match(chunk);
        if (mText.hasMatch()) {
            QString raw = mText.captured(1);
            raw.replace(QRegularExpression("<br\\s*/?>", QRegularExpression::CaseInsensitiveOption), "\n");
            mod.snippet = stripTags(raw).trimmed();
        }

        list.append(mod);
    }

    // Check pagination for next page arrow »
    static QRegularExpression reNext(R"re(<ul[^>]*class="[^"]*pagination[^"]*"[^>]*>(.*?)</ul>)re",
                                     QRegularExpression::DotMatchesEverythingOption);
    auto mPag = reNext.match(html);
    if (mPag.hasMatch()) {
        QString pagHtml = mPag.captured(1);
        if (pagHtml.contains("&raquo;") || pagHtml.contains("»")) {
            // Ensure the next button isn't disabled
            static QRegularExpression reNextLi(R"re(<li[^>]*class="([^"]*)"[^>]*>\s*<a[^>]*>\s*(?:&raquo;|»)\s*</a>)re");
            auto mLi = reNextLi.match(pagHtml);
            if (!mLi.hasMatch() || !mLi.captured(1).contains("disabled")) {
                hasMore = true;
            }
        }
    }

    return list;
}

ModDetail StmodsClient::parseModDetail(const QString& html, const QString& modUrl, const QString& fallbackCategorySlug) const {
    ModDetail detail;
    detail.url = modUrl;
    detail.id = QUrl(modUrl).path();
    detail.categorySlug = fallbackCategorySlug;

    // 1. Parse <h1>
    static QRegularExpression reH1(R"(<h1[^>]*>(.*?)</h1>)", QRegularExpression::DotMatchesEverythingOption);
    auto mH1 = reH1.match(html);
    if (mH1.hasMatch()) {
        QString h1Text = stripTags(mH1.captured(1)).trimmed();

        static QRegularExpression reVer(R"(версия\s+([^\s]+))", QRegularExpression::CaseInsensitiveOption);
        auto mVer = reVer.match(h1Text);
        if (mVer.hasMatch()) detail.version = mVer.captured(1);

        static QRegularExpression reGame(R"(\((v?[\d\.x]+)\))", QRegularExpression::CaseInsensitiveOption);
        auto mGame = reGame.match(h1Text);
        if (mGame.hasMatch()) detail.gameVersion = mGame.captured(1);

        // Clean title: remove leading "Мод " and trailing " версия ..."
        QString cleanTitle = h1Text;
        cleanTitle.remove(QRegularExpression("^Мод\\s+", QRegularExpression::CaseInsensitiveOption));
        cleanTitle.remove(QRegularExpression("\\s+версия\\s+.*$", QRegularExpression::CaseInsensitiveOption));
        cleanTitle.remove(QRegularExpression("\\s+для\\s+BeamNG\\.drive.*$", QRegularExpression::CaseInsensitiveOption));
        detail.title = cleanTitle.trimmed();
    }

    // 2. Parse gallery images (full-res from fancybox links)
    static QRegularExpression reMainImg(R"re(class="material_(?:main_)?img"[^>]*href="([^"]+)")re");
    auto itImg = reMainImg.globalMatch(html);
    while (itImg.hasNext()) {
        auto m = itImg.next();
        QString abs = toAbsoluteUrl(m.captured(1));
        if (!detail.galleryUrls.contains(abs))
            detail.galleryUrls.append(abs);
    }

    // Fallback if href order differed
    if (detail.galleryUrls.isEmpty()) {
        static QRegularExpression reImgAlt(R"re(href="([^"]+)"[^>]*class="material_(?:main_)?img")re");
        auto itAlt = reImgAlt.globalMatch(html);
        while (itAlt.hasNext()) {
            QString abs = toAbsoluteUrl(itAlt.next().captured(1));
            if (!detail.galleryUrls.contains(abs))
                detail.galleryUrls.append(abs);
        }
    }

    // 3. Parse description, features & changelog inside gray_border_block
    int blockStart = html.indexOf("class=\"gray_border_block\"");
    if (blockStart != -1) {
        int blockEnd = html.indexOf("class=\"author-downloader\"", blockStart);
        if (blockEnd == -1) blockEnd = blockStart + 15000;
        QString block = html.mid(blockStart, blockEnd - blockStart);

        // Find main description <div> after main image
        static QRegularExpression reDescDiv(R"(</a>\s*<div>(.*?)</div>)", QRegularExpression::DotMatchesEverythingOption);
        auto mDesc = reDescDiv.match(block);
        if (mDesc.hasMatch()) {
            QString descHtml = mDesc.captured(1);
            descHtml.replace(QRegularExpression("<br\\s*/?>", QRegularExpression::CaseInsensitiveOption), "\n");
            descHtml.replace(QRegularExpression("</p>", QRegularExpression::CaseInsensitiveOption), "\n");
            QString cleanDesc = stripTags(descHtml);

            QStringList proseLines;
            const QStringList lines = cleanDesc.split('\n');
            for (const QString& rawLine : lines) {
                QString line = rawLine.trimmed();
                if (line.isEmpty()) continue;
                if (line.startsWith("-") || line.startsWith("•") || line.startsWith("—")) {
                    QString feat = line;
                    feat.remove(QRegularExpression("^[-•—]\\s*"));
                    if (feat.endsWith(';')) feat.chop(1);
                    if (!feat.isEmpty())
                        detail.features.append(feat.trimmed());
                } else {
                    proseLines.append(line);
                }
            }
            detail.description = proseLines.join("\n\n");
        }

        // Changelog
        static QRegularExpression reChangelog(
            R"(<p\s+class="color-green"[^>]*>\s*<b>(.*?)</b>\s*</p>\s*<p>(.*?)</p>)",
            QRegularExpression::DotMatchesEverythingOption);
        auto mLog = reChangelog.match(block);
        if (mLog.hasMatch()) {
            detail.changelogTitle = stripTags(mLog.captured(1)).trimmed();
            QString logBody = mLog.captured(2);
            logBody.replace(QRegularExpression("<br\\s*/?>", QRegularExpression::CaseInsensitiveOption), "\n");
            detail.changelogText = stripTags(logBody).trimmed();
        }
    }

    // 4. Author
    static QRegularExpression reAuthor(R"(class="author-downloader"[^>]*>.*?Автор:\s*<a[^>]*>(.*?)</a>)",
                                       QRegularExpression::DotMatchesEverythingOption);
    auto mAuthor = reAuthor.match(html);
    if (mAuthor.hasMatch()) {
        detail.author = stripTags(mAuthor.captured(1)).trimmed();
    }

    // 5. Category
    static QRegularExpression reCat(R"re(Категория:\s*<a[^>]+href="([^"]+)"[^>]*>(.*?)</a>)re",
                                    QRegularExpression::DotMatchesEverythingOption);
    auto mCat = reCat.match(html);
    if (mCat.hasMatch()) {
        QString href = mCat.captured(1);
        detail.categoryName = stripTags(mCat.captured(2)).trimmed();
        static QRegularExpression reSlug(R"re(/([a-z_]+)/?$)re");
        auto mSlug = reSlug.match(href);
        if (mSlug.hasMatch()) {
            detail.categorySlug = mSlug.captured(1);
        }
    }

    // 6. Download Link & Size
    static QRegularExpression reDl(
        R"re(<a[^>]+href="(/download_file\.php\?[^"]+)"[^>]*>(.*?)</a>)re",
        QRegularExpression::DotMatchesEverythingOption);
    auto itDl = reDl.globalMatch(html);
    QString firstRoute, firstText;
    while (itDl.hasNext()) {
        auto m = itDl.next();
        QString route = decodeEntities(m.captured(1));
        QString btnHtml = m.captured(2);
        QString btnText = stripTags(btnHtml).trimmed();

        if (firstRoute.isEmpty()) {
            firstRoute = route;
            firstText = btnText;
        }
        if (btnText.contains("ModsFire", Qt::CaseInsensitive)) {
            firstRoute = route;
            firstText = btnText;
            break;
        }
    }

    if (!firstRoute.isEmpty()) {
        detail.downloadRoute = toAbsoluteUrl(firstRoute);

        static QRegularExpression reSize(R"(\(([\d,\.]+\s*(?:МБ|ГБ|КБ|MB|GB|KB))\))",
                                         QRegularExpression::CaseInsensitiveOption);
        auto mSize = reSize.match(firstText);
        if (mSize.hasMatch()) {
            detail.sizeText = mSize.captured(1);
        }

        if (firstText.contains("ModsFire", Qt::CaseInsensitive)) {
            detail.hosterName = "ModsFire";
        } else {
            static QRegularExpression reHoster(R"(с\s+([A-Za-z0-9\.\-]+))");
            auto mHost = reHoster.match(firstText);
            if (mHost.hasMatch()) detail.hosterName = mHost.captured(1);
        }
    }

    return detail;
}

QString StmodsClient::decodeEntities(QString text) {
    text.replace("&nbsp;", " ");
    text.replace("&amp;", "&");
    text.replace("&quot;", "\"");
    text.replace("&#039;", "'");
    text.replace("&apos;", "'");
    text.replace("&lt;", "<");
    text.replace("&gt;", ">");
    text.replace("&laquo;", "«");
    text.replace("&raquo;", "»");
    text.replace("&ndash;", "–");
    text.replace("&mdash;", "—");

    static QRegularExpression reNum(R"(&#(\d+);)");
    auto it = reNum.globalMatch(text);
    while (it.hasNext()) {
        auto m = it.next();
        bool ok = false;
        int code = m.captured(1).toInt(&ok);
        if (ok) {
            text.replace(m.captured(0), QChar(code));
        }
    }
    return text;
}

QString StmodsClient::stripTags(QString html) {
    html.remove(QRegularExpression("<[^>]*>"));
    html = decodeEntities(html);
    html.replace(QRegularExpression("[ \t]+"), " ");
    html.replace(QRegularExpression("\n{3,}"), "\n\n");
    return html;
}

QString StmodsClient::toAbsoluteUrl(const QString& path) {
    if (path.startsWith("http://") || path.startsWith("https://"))
        return path;
    if (path.startsWith("//"))
        return "https:" + path;
    if (path.startsWith("/"))
        return "https://stmods.org" + path;
    return "https://stmods.org/" + path;
}
