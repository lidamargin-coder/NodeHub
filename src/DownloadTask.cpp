#include "DownloadTask.h"
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QUrlQuery>

namespace {
const char* kUserAgent =
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 "
    "(KHTML, like Gecko) Chrome/128.0.0.0 Safari/537.36";
}

DownloadTask::DownloadTask(const QString& modId,
                           const QString& title,
                           const QString& thumbnailUrl,
                           const QString& categorySlug,
                           const QString& downloadRoute,
                           const QString& destinationDir,
                           const QString& fallbackName,
                           QObject* parent)
    : QObject(parent),
      m_modId(modId),
      m_title(title),
      m_thumbnailUrl(thumbnailUrl),
      m_categorySlug(categorySlug),
      m_downloadRoute(downloadRoute),
      m_destDir(destinationDir),
      m_fallbackName(sanitizeFilename(fallbackName)) {
    m_nam.setCookieJar(new QNetworkCookieJar(&m_nam));
}

DownloadTask::~DownloadTask() {
    cancel();
}

QNetworkRequest DownloadTask::makeRequest(const QUrl& url, const QString& referer) const {
    QNetworkRequest req(url);
    req.setRawHeader("User-Agent", kUserAgent);
    req.setRawHeader("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8");
    req.setRawHeader("Accept-Language", "ru-RU,ru;q=0.9,en-US;q=0.8,en;q=0.7");
    if (!referer.isEmpty()) {
        req.setRawHeader("Referer", referer.toUtf8());
    }
    return req;
}

void DownloadTask::start() {
    if (m_state == Downloading || m_state == Resolving)
        return;

    m_state = Resolving;
    m_progress = 0.0;
    m_bytesReceived = 0;
    m_bytesTotal = 0;
    m_speedMBps = 0.0;
    m_errorString.clear();
    emit stateChanged();
    emit progressChanged();

    // Fresh cookie jar for this flow
    m_nam.setCookieJar(new QNetworkCookieJar(&m_nam));

    setStatus(QStringLiteral("Получение ссылки..."));
    step1ResolveStmodsRedirect(QUrl(m_downloadRoute));
}

void DownloadTask::cancel() {
    if (m_reply) {
        QNetworkReply* reply = m_reply;
        m_reply = nullptr;
        reply->disconnect(this);
        reply->abort();
        reply->deleteLater();
    }
    cleanupFile(true);
    if (m_state != Finished && m_state != Canceled) {
        m_state = Canceled;
        m_speedMBps = 0.0;
        setStatus(QStringLiteral("Отменено"));
        emit stateChanged();
        emit progressChanged();
    }
}

void DownloadTask::step1ResolveStmodsRedirect(const QUrl& url) {
    QNetworkRequest req = makeRequest(url);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);

    m_reply = m_nam.get(req);
    connect(m_reply, &QNetworkReply::finished, this, [this]() {
        QNetworkReply* reply = m_reply;
        m_reply = nullptr;
        if (!reply) return;

        if (reply->error() == QNetworkReply::OperationCanceledError) {
            reply->deleteLater();
            return;
        }

        QUrl redirectTarget = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (redirectTarget.isValid()) {
            redirectTarget = reply->url().resolved(redirectTarget);
            reply->deleteLater();

            QString host = redirectTarget.host().toLower();
            if (host.contains("modsfire.com")) {
                step2FetchModsfirePage(redirectTarget);
            } else if (host.contains("stmods.org")) {
                step1ResolveStmodsRedirect(redirectTarget);
            } else if (redirectTarget.path().endsWith(".zip", Qt::CaseInsensitive) ||
                       redirectTarget.path().endsWith(".7z", Qt::CaseInsensitive) ||
                       redirectTarget.path().endsWith(".rar", Qt::CaseInsensitive)) {
                step5StreamFile(redirectTarget, {});
            } else {
                fail(QStringLiteral("Неподдерживаемый файлообменник: %1").arg(host));
            }
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            fail(QStringLiteral("Ошибка перехода STMods: %1").arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        QString html = QString::fromUtf8(reply->readAll());
        reply->deleteLater();

        static QRegularExpression reMf(R"(https?://(?:www\.)?modsfire\.com/[A-Za-z0-9_\-/]+)");
        auto m = reMf.match(html);
        if (m.hasMatch()) {
            step2FetchModsfirePage(QUrl(m.captured(0)));
        } else {
            fail(QStringLiteral("Не удалось получить ссылку на ModsFire"));
        }
    });
}

void DownloadTask::step2FetchModsfirePage(const QUrl& modsfireUrl) {
    setStatus(QStringLiteral("Подготовка ModsFire..."));

    QNetworkRequest req = makeRequest(modsfireUrl);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    m_reply = m_nam.get(req);
    connect(m_reply, &QNetworkReply::finished, this, [this, modsfireUrl]() {
        QNetworkReply* reply = m_reply;
        m_reply = nullptr;
        if (!reply) return;

        if (reply->error() == QNetworkReply::OperationCanceledError) {
            reply->deleteLater();
            return;
        }
        if (reply->error() != QNetworkReply::NoError) {
            fail(QStringLiteral("Ошибка ModsFire (шаг 1): %1").arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        QUrl finalPageUrl = reply->url();
        QString html = QString::fromUtf8(reply->readAll());
        reply->deleteLater();

        // 1. Extract filename from <title> if present (e.g. "Download file mod_name.zip - First Step...")
        static QRegularExpression reZipName(R"re([\w\.\-]+\.(?:zip|7z|rar))re", QRegularExpression::CaseInsensitiveOption);
        static QRegularExpression reTitle(R"(<title>(.*?)</title>)", QRegularExpression::CaseInsensitiveOption);
        auto mTitle = reTitle.match(html);
        if (mTitle.hasMatch()) {
            auto mZip = reZipName.match(mTitle.captured(1));
            if (mZip.hasMatch()) {
                m_detectedFilename = mZip.captured(0);
            }
        }

        // 2. Check if direct download button is already present (e.g. href="/d/..." or id="downloadnow")
        static QRegularExpression reDirectBtn(
            R"re(<a\b[^>]*?\bhref="([^"]*?/d/[A-Za-z0-9_\-/]+)"[^>]*?>)re",
            QRegularExpression::CaseInsensitiveOption);
        auto mDirect = reDirectBtn.match(html);
        if (mDirect.hasMatch()) {
            QUrl dlUrl = finalPageUrl.resolved(QUrl(mDirect.captured(1)));
            step4ResolveFinalUrl(dlUrl, finalPageUrl);
            return;
        }

        // 3. Look for /download/... "Generate Link" href
        static QRegularExpression reGen(
            R"re(<a\b[^>]*?\bhref="([^"]*?/download/[A-Za-z0-9_\-/]+)"[^>]*?>)re",
            QRegularExpression::CaseInsensitiveOption);
        auto mGen = reGen.match(html);
        if (mGen.hasMatch()) {
            QUrl genUrl = finalPageUrl.resolved(QUrl(mGen.captured(1)));
            step3FetchGenerateLink(genUrl, finalPageUrl);
            return;
        }

        // Fallback: any href containing /download/
        static QRegularExpression reGenAny(R"re(href="(/download/[A-Za-z0-9_\-/]+)")re");
        auto mGenAny = reGenAny.match(html);
        if (mGenAny.hasMatch()) {
            QUrl genUrl = finalPageUrl.resolved(QUrl(mGenAny.captured(1)));
            step3FetchGenerateLink(genUrl, finalPageUrl);
            return;
        }

        fail(QStringLiteral("Не найдена ссылка на скачивание на ModsFire"));
    });
}

void DownloadTask::step3FetchGenerateLink(const QUrl& genUrl, const QUrl& referer) {
    setStatus(QStringLiteral("Генерация ссылки..."));

    QNetworkRequest req = makeRequest(genUrl, referer.toString());
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    m_reply = m_nam.get(req);
    connect(m_reply, &QNetworkReply::finished, this, [this, genUrl]() {
        QNetworkReply* reply = m_reply;
        m_reply = nullptr;
        if (!reply) return;

        if (reply->error() == QNetworkReply::OperationCanceledError) {
            reply->deleteLater();
            return;
        }
        if (reply->error() != QNetworkReply::NoError) {
            fail(QStringLiteral("Ошибка генерации ссылки: %1").arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        QUrl finalGenPageUrl = reply->url();
        QString html = QString::fromUtf8(reply->readAll());
        reply->deleteLater();

        // Update detected filename if not already set
        if (m_detectedFilename.isEmpty()) {
            static QRegularExpression reZipName(R"re([\w\.\-]+\.(?:zip|7z|rar))re", QRegularExpression::CaseInsensitiveOption);
            static QRegularExpression reTitle(R"(<title>(.*?)</title>)", QRegularExpression::CaseInsensitiveOption);
            auto mTitle = reTitle.match(html);
            if (mTitle.hasMatch()) {
                auto mZip = reZipName.match(mTitle.captured(1));
                if (mZip.hasMatch()) {
                    m_detectedFilename = mZip.captured(0);
                }
            }
        }

        // Download button on step 2 matches:
        // A) <a ... href="/d/..." ...>
        static QRegularExpression reDPath(
            R"re(<a\b[^>]*?\bhref="([^"]*?/d/[A-Za-z0-9_\-/]+)"[^>]*?>)re",
            QRegularExpression::CaseInsensitiveOption);
        auto mD = reDPath.match(html);
        if (mD.hasMatch()) {
            QUrl dlUrl = finalGenPageUrl.resolved(QUrl(mD.captured(1)));
            step4ResolveFinalUrl(dlUrl, finalGenPageUrl);
            return;
        }

        // B) <a ... id="downloadnow" ... href="..."> or reverse
        static QRegularExpression reDlNow1(
            R"re(<a\b[^>]*?\bid="downloadnow"[^>]*?\bhref="([^"]+)")re",
            QRegularExpression::CaseInsensitiveOption);
        auto mNow1 = reDlNow1.match(html);
        if (mNow1.hasMatch()) {
            QUrl dlUrl = finalGenPageUrl.resolved(QUrl(mNow1.captured(1)));
            step4ResolveFinalUrl(dlUrl, finalGenPageUrl);
            return;
        }

        static QRegularExpression reDlNow2(
            R"re(<a\b[^>]*?\bhref="([^"]+)"[^>]*?\bid="downloadnow")re",
            QRegularExpression::CaseInsensitiveOption);
        auto mNow2 = reDlNow2.match(html);
        if (mNow2.hasMatch()) {
            QUrl dlUrl = finalGenPageUrl.resolved(QUrl(mNow2.captured(1)));
            step4ResolveFinalUrl(dlUrl, finalGenPageUrl);
            return;
        }

        // C) <a ... class="...download-button..." ... href="..."> or reverse
        static QRegularExpression reBtn1(
            R"re(<a\b[^>]*?\bclass="[^"]*download-button[^"]*"[^>]*?\bhref="([^"]+)")re",
            QRegularExpression::CaseInsensitiveOption);
        auto mBtn1 = reBtn1.match(html);
        if (mBtn1.hasMatch()) {
            QUrl dlUrl = finalGenPageUrl.resolved(QUrl(mBtn1.captured(1)));
            step4ResolveFinalUrl(dlUrl, finalGenPageUrl);
            return;
        }

        static QRegularExpression reBtn2(
            R"re(<a\b[^>]*?\bhref="([^"]+)"[^>]*?\bclass="[^"]*download-button)re",
            QRegularExpression::CaseInsensitiveOption);
        auto mBtn2 = reBtn2.match(html);
        if (mBtn2.hasMatch()) {
            QUrl dlUrl = finalGenPageUrl.resolved(QUrl(mBtn2.captured(1)));
            step4ResolveFinalUrl(dlUrl, finalGenPageUrl);
            return;
        }

        // D) Fallback raw href="/d/..."
        static QRegularExpression reRawD(R"re(href="(/d/[A-Za-z0-9_\-/]+)")re");
        auto mRawD = reRawD.match(html);
        if (mRawD.hasMatch()) {
            QUrl dlUrl = finalGenPageUrl.resolved(QUrl(mRawD.captured(1)));
            step4ResolveFinalUrl(dlUrl, finalGenPageUrl);
            return;
        }

        fail(QStringLiteral("Кнопка загрузки не найдена"));
    });
}

void DownloadTask::step4ResolveFinalUrl(const QUrl& dlButtonUrl, const QUrl& referer) {
    setStatus(QStringLiteral("Подключение к серверу..."));

    QNetworkRequest req = makeRequest(dlButtonUrl, referer.toString());
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::ManualRedirectPolicy);

    m_reply = m_nam.get(req);
    connect(m_reply, &QNetworkReply::finished, this, [this, dlButtonUrl]() {
        QNetworkReply* reply = m_reply;
        m_reply = nullptr;
        if (!reply) return;

        if (reply->error() == QNetworkReply::OperationCanceledError) {
            reply->deleteLater();
            return;
        }

        QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
        if (redirect.isValid()) {
            redirect = reply->url().resolved(redirect);
            reply->deleteLater();
            step5StreamFile(redirect, dlButtonUrl);
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            fail(QStringLiteral("Ошибка получения файла: %1").arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        // Might have served directly without 302
        step5StreamFile(reply->url(), dlButtonUrl);
        reply->deleteLater();
    });
}

void DownloadTask::step5StreamFile(const QUrl& directUrl, const QUrl& referer) {
    m_state = Downloading;
    setStatus(QStringLiteral("Загрузка архива..."));
    emit stateChanged();

    // Determine clean filename
    QString filename = m_detectedFilename;
    if (filename.isEmpty()) {
        filename = extractFilenameFromUrl(directUrl);
    }
    if (filename.isEmpty()) {
        filename = m_fallbackName;
        if (!filename.endsWith(".zip", Qt::CaseInsensitive)) {
            filename += ".zip";
        }
    }
    filename = sanitizeFilename(filename);

    QDir dir(m_destDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    m_finalFilePath = dir.absoluteFilePath(filename);
    m_partFilePath = m_finalFilePath + ".part";

    m_file.setFileName(m_partFilePath);
    if (!m_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        fail(QStringLiteral("Не удалось создать файл для сохранения:\n%1").arg(m_partFilePath));
        return;
    }

    QNetworkRequest req = makeRequest(directUrl, referer.toString());
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    m_timer.start();
    m_bytesReceived = 0;
    m_bytesTotal = 0;
    m_progress = 0.0;
    m_speedMBps = 0.0;

    m_reply = m_nam.get(req);

    connect(m_reply, &QNetworkReply::downloadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal) {
        m_bytesReceived = bytesReceived;
        m_bytesTotal = bytesTotal;
        if (bytesTotal > 0) {
            m_progress = static_cast<double>(bytesReceived) / static_cast<double>(bytesTotal);
        }

        qint64 elapsed = m_timer.elapsed();
        if (elapsed > 300) {
            m_speedMBps = (static_cast<double>(bytesReceived) / (1024.0 * 1024.0)) / (static_cast<double>(elapsed) / 1000.0);
        }

        emit progressChanged();
    });

    connect(m_reply, &QIODevice::readyRead, this, [this]() {
        if (!m_reply) return;
        QByteArray chunk = m_reply->readAll();
        if (!chunk.isEmpty()) {
            m_file.write(chunk);
        }
    });

    connect(m_reply, &QNetworkReply::finished, this, [this]() {
        QNetworkReply* reply = m_reply;
        m_reply = nullptr;
        if (!reply) return;

        if (reply->error() == QNetworkReply::OperationCanceledError) {
            cleanupFile(true);
            reply->deleteLater();
            return;
        }

        if (reply->error() != QNetworkReply::NoError) {
            cleanupFile(true);
            fail(QStringLiteral("Ошибка при скачивании файла: %1").arg(reply->errorString()));
            reply->deleteLater();
            return;
        }

        // Flush & finalize file
        m_file.flush();
        m_file.close();

        // Atomically rename .part -> final .zip
        QFile::remove(m_finalFilePath);
        if (!QFile::rename(m_partFilePath, m_finalFilePath)) {
            // Fallback copy
            QFile::copy(m_partFilePath, m_finalFilePath);
            QFile::remove(m_partFilePath);
        }

        m_state = Finished;
        m_progress = 1.0;
        m_speedMBps = 0.0;
        setStatus(QStringLiteral("Установлен ✓"));
        emit stateChanged();
        emit progressChanged();
        emit finished(m_finalFilePath);

        reply->deleteLater();
    });
}

void DownloadTask::setStatus(const QString& text) {
    if (m_statusText != text) {
        m_statusText = text;
    }
}

void DownloadTask::fail(const QString& error) {
    m_state = Failed;
    m_errorString = error;
    m_speedMBps = 0.0;
    setStatus(QStringLiteral("Ошибка"));
    cleanupFile(true);
    emit stateChanged();
    emit progressChanged();
    emit failed(error);
}

void DownloadTask::cleanupFile(bool deletePart) {
    if (m_file.isOpen()) {
        m_file.close();
    }
    if (deletePart && !m_partFilePath.isEmpty() && QFile::exists(m_partFilePath)) {
        QFile::remove(m_partFilePath);
    }
}

QString DownloadTask::sanitizeFilename(QString name) {
    name.replace(QRegularExpression(R"([<>:"/\\|?*])"), "_");
    name.replace(QRegularExpression(R"(\s+)"), "_");
    name = name.trimmed();
    if (name.isEmpty()) name = "mod";
    return name;
}

QString DownloadTask::extractFilenameFromUrl(const QUrl& url) {
    // 1. Check for base64 encoded path in query parameter "p" (ModsFire direct download link)
    QUrlQuery query(url);
    if (query.hasQueryItem("p")) {
        QString p = query.queryItemValue("p");
        QByteArray decoded = QByteArray::fromBase64(p.toUtf8());
        QString decodedStr = QString::fromUtf8(decoded);
        int slash = decodedStr.lastIndexOf('/');
        QString cand = (slash != -1) ? decodedStr.mid(slash + 1) : decodedStr;
        if (cand.endsWith(".zip", Qt::CaseInsensitive) ||
            cand.endsWith(".7z", Qt::CaseInsensitive) ||
            cand.endsWith(".rar", Qt::CaseInsensitive)) {
            return cand;
        }
    }

    // 2. Standard path extraction
    QString path = url.path();
    int idx = path.lastIndexOf('/');
    if (idx != -1) {
        QString cand = path.mid(idx + 1);
        if (cand.endsWith(".zip", Qt::CaseInsensitive) ||
            cand.endsWith(".7z", Qt::CaseInsensitive) ||
            cand.endsWith(".rar", Qt::CaseInsensitive)) {
            return cand;
        }
    }
    return {};
}
