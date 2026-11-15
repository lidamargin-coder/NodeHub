#include "UpdateManager.h"
#include "AppVersion.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QProcess>
#include <QStandardPaths>
#include <QTimer>

#ifdef Q_OS_WIN
#include <windows.h>
#include <shellapi.h>
#endif

namespace {
struct SemVer {
    int major = 0;
    int minor = 0;
    int patch = 0;

    static SemVer parse(QString str) {
        str = str.trimmed().toLower();
        while (str.startsWith('v')) {
            str.remove(0, 1);
        }
        int dashIdx = str.indexOf('-');
        if (dashIdx != -1) {
            str = str.left(dashIdx);
        }
        int plusIdx = str.indexOf('+');
        if (plusIdx != -1) {
            str = str.left(plusIdx);
        }

        SemVer v;
        const QStringList parts = str.split('.');
        if (parts.size() > 0) v.major = parts[0].toInt();
        if (parts.size() > 1) v.minor = parts[1].toInt();
        if (parts.size() > 2) v.patch = parts[2].toInt();
        return v;
    }

    bool isGreaterThan(const SemVer& other) const {
        if (major != other.major) return major > other.major;
        if (minor != other.minor) return minor > other.minor;
        return patch > other.patch;
    }
};

QString formatMb(qint64 bytes) {
    double mb = static_cast<double>(bytes) / (1024.0 * 1024.0);
    return QString::number(mb, 'f', 1);
}
} // namespace

UpdateManager::UpdateManager(QObject* parent)
    : QObject(parent) {
}

UpdateManager::~UpdateManager() {
    if (m_checkReply) {
        m_checkReply->abort();
        m_checkReply->deleteLater();
        m_checkReply = nullptr;
    }
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }
    if (m_downloadFile) {
        m_downloadFile->close();
        delete m_downloadFile;
        m_downloadFile = nullptr;
    }
}

QString UpdateManager::currentVersion() const {
    return QStringLiteral(NODEHUB_VERSION);
}

void UpdateManager::setState(const QString& newState) {
    if (m_state != newState) {
        m_state = newState;
        emit stateChanged();
    }
}

void UpdateManager::setStatus(const QString& text) {
    if (m_statusText != text) {
        m_statusText = text;
        emit statusTextChanged();
    }
}

bool UpdateManager::isRemoteNewer(const QString& currentVer, const QString& remoteVer) {
    SemVer c = SemVer::parse(currentVer);
    SemVer r = SemVer::parse(remoteVer);
    return r.isGreaterThan(c);
}

void UpdateManager::checkUpdates(bool silent) {
    if (m_checkReply || (m_downloadReply && m_state == "downloading")) {
        qDebug() << "UpdateManager::checkUpdates already in progress";
        return;
    }

    m_silentCheck = silent;
    setState("checking");
    setStatus(QStringLiteral("Проверка обновлений..."));

    const QString urlStr = QStringLiteral("https://api.github.com/repos/%1/releases/latest").arg(QStringLiteral(NODEHUB_GITHUB_REPO));
    QUrl url(urlStr);
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral(NODEHUB_USER_AGENT));
    request.setRawHeader("Accept", "application/vnd.github.v3+json");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    qDebug() << "UpdateManager::checkUpdates checking:" << urlStr << "silent=" << silent;
    m_checkReply = m_nam.get(request);
    connect(m_checkReply, &QNetworkReply::finished, this, &UpdateManager::onCheckFinished);
}

void UpdateManager::onCheckFinished() {
    if (!m_checkReply) return;

    QNetworkReply* reply = m_checkReply;
    m_checkReply = nullptr;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "UpdateManager check error:" << reply->errorString();
        setState("error");
        setStatus(QStringLiteral("Не удалось проверить обновления"));
        if (!m_silentCheck) {
            emit messageRequested(QStringLiteral("Ошибка проверки обновлений: %1").arg(reply->errorString()), true);
        } else {
            setState("idle");
        }
        return;
    }

    const QByteArray data = reply->readAll();
    QJsonParseError parseErr;
    const QJsonDocument doc = QJsonDocument::fromJson(data, &parseErr);
    if (parseErr.error != QJsonParseError::NoError || !doc.isObject()) {
        qWarning() << "UpdateManager JSON parse error:" << parseErr.errorString();
        setState("idle");
        return;
    }

    const QJsonObject root = doc.object();
    QString tagName = root.value(QStringLiteral("tag_name")).toString().trimmed();
    if (tagName.isEmpty()) {
        tagName = root.value(QStringLiteral("name")).toString().trimmed();
    }
    const QString body = root.value(QStringLiteral("body")).toString();

    // Clean remote version string
    QString cleanRemoteVer = tagName;
    if (cleanRemoteVer.startsWith('v') || cleanRemoteVer.startsWith('V')) {
        cleanRemoteVer.remove(0, 1);
    }

    qDebug() << "UpdateManager: local=" << currentVersion() << "remote=" << cleanRemoteVer << "tag=" << tagName;

    if (!isRemoteNewer(currentVersion(), cleanRemoteVer)) {
        qDebug() << "UpdateManager: NodeHub is up to date (" << currentVersion() << ")";
        setState("up_to_date");
        setStatus(QStringLiteral("У вас актуальная версия"));
        if (!m_silentCheck) {
            emit messageRequested(QStringLiteral("У вас установлена последняя версия NodeHub (v%1)").arg(currentVersion()), false);
        }
        QTimer::singleShot(4000, this, [this]() {
            if (m_state == "up_to_date") setState("idle");
        });
        return;
    }

    // Found newer version!
    m_newVersion = cleanRemoteVer;
    emit newVersionChanged();

    m_releaseNotes = body;
    emit releaseNotesChanged();

    // Look for installer asset in assets array
    QString exeUrl;
    qint64 exeSize = 0;
    const QJsonArray assets = root.value(QStringLiteral("assets")).toArray();
    for (const auto& aVal : assets) {
        const QJsonObject aObj = aVal.toObject();
        const QString aName = aObj.value(QStringLiteral("name")).toString();
        if (aName.endsWith(QStringLiteral(".exe"), Qt::CaseInsensitive)) {
            exeUrl = aObj.value(QStringLiteral("browser_download_url")).toString();
            exeSize = static_cast<qint64>(aObj.value(QStringLiteral("size")).toDouble());
            break;
        }
    }

    if (exeUrl.isEmpty()) {
        qWarning() << "UpdateManager: Newer release found but no .exe asset available";
        setState("idle");
        return;
    }

    qDebug() << "UpdateManager: Starting automatic update to v" << m_newVersion << "from" << exeUrl;
    startDownload(QUrl(exeUrl), exeSize);
}

void UpdateManager::startDownload(const QUrl& url, qint64 expectedSize) {
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }
    if (m_downloadFile) {
        m_downloadFile->close();
        delete m_downloadFile;
        m_downloadFile = nullptr;
    }

    m_downloadUrl = url.toString();
    m_bytesTotal = expectedSize;
    m_bytesReceived = 0;
    m_progressPercent = 0;
    emit progressChanged();

    const QString tempDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    m_downloadedFilePath = QDir(tempDir).filePath(QStringLiteral("NodeHub_Update_v%1.exe").arg(m_newVersion));

    // Remove any leftover previous attempt
    QFile::remove(m_downloadedFilePath);

    m_downloadFile = new QFile(m_downloadedFilePath, this);
    if (!m_downloadFile->open(QIODevice::WriteOnly)) {
        qWarning() << "UpdateManager: Failed to open temp update file for writing:" << m_downloadedFilePath;
        delete m_downloadFile;
        m_downloadFile = nullptr;
        setState("error");
        setStatus(QStringLiteral("Ошибка создания временного файла обновления"));
        return;
    }

    setState("downloading");
    setStatus(QStringLiteral("Загрузка обновления NodeHub v%1...").arg(m_newVersion));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral(NODEHUB_USER_AGENT));
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);

    m_downloadReply = m_nam.get(request);
    connect(m_downloadReply, &QNetworkReply::readyRead, this, &UpdateManager::onDownloadReadyRead);
    connect(m_downloadReply, &QNetworkReply::downloadProgress, this, &UpdateManager::onDownloadProgress);
    connect(m_downloadReply, &QNetworkReply::finished, this, &UpdateManager::onDownloadFinished);
}

void UpdateManager::onDownloadReadyRead() {
    if (m_downloadFile && m_downloadReply) {
        m_downloadFile->write(m_downloadReply->readAll());
    }
}

void UpdateManager::onDownloadProgress(qint64 received, qint64 total) {
    m_bytesReceived = received;
    if (total > 0) {
        m_bytesTotal = total;
    }

    if (m_bytesTotal > 0) {
        m_progressPercent = static_cast<int>((m_bytesReceived * 100) / m_bytesTotal);
    } else {
        m_progressPercent = 0;
    }
    emit progressChanged();

    const QString status = QStringLiteral("Обновление NodeHub v%1: %2% (%3 / %4 МБ)")
        .arg(m_newVersion)
        .arg(m_progressPercent)
        .arg(formatMb(m_bytesReceived))
        .arg(m_bytesTotal > 0 ? formatMb(m_bytesTotal) : QStringLiteral("?"));
    setStatus(status);
}

void UpdateManager::onDownloadFinished() {
    if (!m_downloadReply) return;

    QNetworkReply* reply = m_downloadReply;
    m_downloadReply = nullptr;
    reply->deleteLater();

    if (m_downloadFile) {
        m_downloadFile->flush();
        m_downloadFile->close();
    }

    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "UpdateManager: download failed:" << reply->errorString();
        if (m_downloadFile) {
            m_downloadFile->remove();
            delete m_downloadFile;
            m_downloadFile = nullptr;
        }
        setState("error");
        setStatus(QStringLiteral("Ошибка при скачивании обновления"));
        emit messageRequested(QStringLiteral("Не удалось скачать обновление: %1").arg(reply->errorString()), true);
        return;
    }

    // Verify downloaded file size (installer must be at least ~5 MB)
    QFileInfo fi(m_downloadedFilePath);
    if (!fi.exists() || fi.size() < 5 * 1024 * 1024) {
        qWarning() << "UpdateManager: downloaded installer file is too small or corrupt:" << fi.size();
        QFile::remove(m_downloadedFilePath);
        setState("error");
        setStatus(QStringLiteral("Загруженный файл обновления поврежден"));
        return;
    }

    qDebug() << "UpdateManager: update installer downloaded successfully:" << m_downloadedFilePath << "size=" << fi.size();

    setState("ready");
    m_progressPercent = 100;
    emit progressChanged();
    setStatus(QStringLiteral("Обновление v%1 загружено. Перезапуск лаунчера...").arg(m_newVersion));
    emit updateReady(m_newVersion, m_downloadedFilePath);

    // Give user 1.2s to see that the update is ready, then launch installer and restart
    QTimer::singleShot(1200, this, &UpdateManager::installAndRestart);
}

void UpdateManager::cancelUpdate() {
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }
    if (m_downloadFile) {
        m_downloadFile->close();
        m_downloadFile->remove();
        delete m_downloadFile;
        m_downloadFile = nullptr;
    }
    setState("idle");
    setStatus(QString());
}

void UpdateManager::installAndRestart() {
    if (m_downloadedFilePath.isEmpty() || !QFile::exists(m_downloadedFilePath)) {
        qWarning() << "UpdateManager::installAndRestart file not found:" << m_downloadedFilePath;
        setState("idle");
        return;
    }

    const QString exePath = QDir::toNativeSeparators(m_downloadedFilePath);
    const QString appDir = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());

    QStringList args;
    args << QStringLiteral("/VERYSILENT")
         << QStringLiteral("/SUPPRESSMSGBOXES")
         << QStringLiteral("/NORESTART")
         << QStringLiteral("/CLOSEAPPLICATIONS");

    // If running from an installed location (not dev/source tree), instruct installer to install directly here
    if (!QFile::exists(appDir + QStringLiteral("/../CMakeLists.txt")) &&
        !QFile::exists(appDir + QStringLiteral("/CMakeLists.txt"))) {
        args << QStringLiteral("/DIR=\"%1\"").arg(appDir);
    }

    qDebug() << "UpdateManager::installAndRestart launching:" << exePath << args;

#ifdef Q_OS_WIN
    const QString argsStr = args.join(QStringLiteral(" "));
    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.lpVerb = L"runas"; // triggers UAC elevation if required for Program Files
    sei.lpFile = reinterpret_cast<LPCWSTR>(exePath.utf16());
    sei.lpParameters = reinterpret_cast<LPCWSTR>(argsStr.utf16());
    sei.nShow = SW_SHOWNORMAL;
    sei.fMask = SEE_MASK_NOASYNC;

    BOOL success = ShellExecuteExW(&sei);
    if (!success) {
        DWORD err = GetLastError();
        qWarning() << "UpdateManager: ShellExecuteExW failed with code:" << err;
        if (err == ERROR_CANCELLED) {
            // User cancelled UAC prompt
            setState("idle");
            setStatus(QStringLiteral("Обновление отменено пользователем."));
            emit messageRequested(QStringLiteral("Обновление отменено (отказано в правах администратора)"), true);
            return;
        }
    }
#else
    QProcess::startDetached(exePath, args);
#endif

    // Give installer process 500ms to spin up, then exit NodeHub cleanly
    QTimer::singleShot(500, qApp, &QCoreApplication::quit);
}
