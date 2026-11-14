#include "SettingsManager.h"
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QProcess>

SettingsManager::SettingsManager(QObject* parent)
    : QObject(parent)
    , m_settings("NodeHub", "NodeHub") {
    m_gamePath = m_settings.value("beamng/gamePath", "").toString();
}

QString SettingsManager::gamePath() const {
    return m_gamePath;
}

bool SettingsManager::setGamePath(const QString& path) {
    QString cleanPath = path.trimmed();
    if (cleanPath.startsWith('"') && cleanPath.endsWith('"') && cleanPath.size() >= 2) {
        cleanPath = cleanPath.mid(1, cleanPath.size() - 2).trimmed();
    }
    if (cleanPath.startsWith('\'') && cleanPath.endsWith('\'') && cleanPath.size() >= 2) {
        cleanPath = cleanPath.mid(1, cleanPath.size() - 2).trimmed();
    }
    cleanPath = QDir::cleanPath(cleanPath);

    if (cleanPath.isEmpty())
        return false;

    QDir dir(cleanPath);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            return false;
        }
    }

    if (m_gamePath != cleanPath) {
        m_gamePath = cleanPath;
        m_settings.setValue("beamng/gamePath", m_gamePath);
        m_settings.sync();
        emit gamePathChanged();
    }
    return true;
}

bool SettingsManager::hasGamePath() const {
    return !m_gamePath.isEmpty() && QDir(m_gamePath).exists();
}

bool SettingsManager::canLaunchGame() const {
    return !findGameExecutable().isEmpty();
}

QString SettingsManager::findGameExecutable() const {
    if (m_gamePath.isEmpty()) return {};

    QDir base(m_gamePath);
    QString p1 = base.filePath("BeamNG.drive.exe");
    if (QFile::exists(p1)) return p1;

    QString p2 = base.filePath("Bin64/BeamNG.drive.x64.exe");
    if (QFile::exists(p2)) return p2;

    return {};
}

bool SettingsManager::launchGame() {
    QString exePath = findGameExecutable();
    if (exePath.isEmpty()) return false;

    return QProcess::startDetached(exePath, QStringList(), m_gamePath);
}

QString SettingsManager::browseGameFolder() {
    QString startDir = m_gamePath.isEmpty() ? QDir::homePath() : m_gamePath;
    QString selected = QFileDialog::getExistingDirectory(
        nullptr,
        QStringLiteral("Выберите корневую папку игры BeamNG.drive"),
        startDir,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
    );
    if (!selected.isEmpty()) {
        selected = QDir::toNativeSeparators(selected);
    }
    return selected;
}

bool SettingsManager::isValidBeamNgFolder(const QString& path) const {
    QString clean = path.trimmed();
    if (clean.startsWith('"') && clean.endsWith('"') && clean.size() >= 2) {
        clean = clean.mid(1, clean.size() - 2).trimmed();
    }
    if (clean.startsWith('\'') && clean.endsWith('\'') && clean.size() >= 2) {
        clean = clean.mid(1, clean.size() - 2).trimmed();
    }
    clean = QDir::cleanPath(clean);
    if (clean.isEmpty())
        return false;
    QDir dir(clean);
    return dir.exists() || dir.mkpath(".");
}

QString SettingsManager::resolveTargetFolder(const QString& categorySlug) const {
    if (m_gamePath.isEmpty())
        return {};

    QString slug = categorySlug.toLower();
    QDir baseDir(m_gamePath);

    if (slug.contains("map")) {
        return QDir::cleanPath(baseDir.filePath("content/levels"));
    }
    if (slug.contains("car") || slug.contains("jeep") || slug.contains("truck") ||
        slug.contains("racing") || slug.contains("bus") || slug.contains("tractor") ||
        slug.contains("trailer") || slug.contains("aviation") || slug.contains("machinery")) {
        return QDir::cleanPath(baseDir.filePath("content/vehicles"));
    }
    return QDir::cleanPath(baseDir.filePath("content"));
}
