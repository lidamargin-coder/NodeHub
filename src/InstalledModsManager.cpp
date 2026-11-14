#include "InstalledModsManager.h"
#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QUrl>

namespace {

const QSet<QString>& getVanillaVehicles() {
    static const QSet<QString> s_vanilla = {
        "autobello", "barstow", "bastion", "bolide", "burnside", "bx", "cannon", "caravan",
        "citybus", "common", "coupe", "covet", "etk800", "etkc", "etki", "fullsize", "hatch",
        "hopper", "lansdale", "legran", "midsize", "miramar", "moonhawk", "pessima", "pickup",
        "pigeon", "roamer", "sbr", "scintilla", "semi", "sunburst", "tseries", "us_semi",
        "van", "vivace", "wendover", "wigeon", "ball", "barrels", "barrier", "barrier_plastic",
        "bollard", "boxutility", "boxutility_large", "cardboard_box", "cargotrailer", "chair",
        "christmas_tree", "cones", "containerTrailer", "couch", "crowdbarrier", "delineator",
        "dino", "dolly", "dryvan", "dumptruck", "engine_props", "eu_trailers", "flail", "flatbed",
        "flipramp", "fridge", "gate", "haybale", "inflated_mat", "kickplate", "large_angletester",
        "large_bridge", "large_cannon", "large_crusher", "large_hamster_wheel", "large_roller",
        "large_spinner", "large_tilt", "large_tire", "log_trailer", "logs", "marble_block",
        "mattress", "metal_box", "metal_ramp", "midtruck", "nine", "percheron", "piano",
        "porta_potty", "pressure_ball", "racetruck", "rallyflags", "rallysigns", "rallytape",
        "roadsigns", "rock_pile", "rockbouncer", "rocks", "rollover", "roof_crush_tester",
        "sawhorse", "shipping_container", "simple_traffic", "spikestrip", "sprinter", "steel_coil",
        "streetlight", "suspensionbridge", "tanker", "testroller", "tiltdeck", "tirestacks",
        "tirewall", "trafficbarrel", "trampoline", "trashbin", "tsfb", "tub", "tube", "tv",
        "unicycle", "utv", "wall", "weightpad", "wl40", "woodcrate", "woodplanks"
    };
    return s_vanilla;
}

const QSet<QString>& getVanillaLevels() {
    static const QSet<QString> s_vanilla = {
        "automation_test_track", "autotest", "cliff", "derby", "driver_training", "east_coast_usa",
        "garage_v2", "gridmap_v2", "hirochi_raceway", "industrial", "italy", "johnson_valley",
        "jungle_rock_island", "showroom_v2", "showroom_v2_lighting_simple", "small_island",
        "smallgrid", "template", "utah", "west_coast_usa"
    };
    return s_vanilla;
}

struct DetectedCategory {
    QString slug;
    QString name;
};

DetectedCategory detectModCategory(const QString& filename, const QString& subDir) {
    QString lower = filename.toLower();
    lower.replace('_', ' ');
    lower.replace('-', ' ');

    if (subDir.contains("levels", Qt::CaseInsensitive) ||
        lower.contains("turnpike") || lower.contains("kamurocho") ||
        lower.contains("russianroads") || lower.contains("mammoth") ||
        lower.contains("somerset") || lower.contains("road") ||
        lower.contains("карта") || lower.contains("трасса")) {
        return { "maps", QStringLiteral("Карты") };
    }

    // 1. Jeeps, SUVs, Pickups & Offroaders
    static const QStringList s_jeepKw = {
        "jeep", "suv", "niva", "patriot", "uaz", "prado", "land cruiser", "landcruiser",
        "cruiser", "lc200", "lc300", "lc100", "lc70", "bronto", "yukon", "escalade",
        "q7", "gls", "x5", "x6", "x7", "touareg", "defender", "cayenne", "macan",
        "urso", "urus", "gelandewagen", "g63", "g500", "g class", "gclass", "trx",
        "f150", "f 150", "raptor", "silverado", "c10", "tahoe", "suburban", "cherokee",
        "wrangler", "renegade", "duster", "rav4", "pajero", "cross country", "xc70",
        "xc90", "trackhawk", "4x4", "offroad", "внедорожник", "джип", "нива", "уаз",
        "discovery", "range rover", "rangerover", "evoque", "jimny", "patrol", "cullinan",
        "culin", "roroculin", "w164", "w166", "w167", "x164", "x166", "x167", "gle",
        "glc", "gla", "glb", "ml350", "ml500", "ml63", "gmt400", "gmt800", "gmt900",
        "gmc", "sierra", "2500 hd", "3500 hd", "atv", "quad", "kodiaq"
    };
    for (const QString& kw : s_jeepKw) {
        if (lower.contains(kw)) {
            return { "jeeps", QStringLiteral("Внедорожники") };
        }
    }

    // 2. Trucks, Semis, Commercial & Vans
    static const QStringList s_truckKw = {
        "truck", "kamaz", "kraz", "maz", "zil", "ural", "man", "scania", "volvo fh",
        "actros", "daf", "iveco", "peterbilt", "kenworth", "mack", "gazelle", "gaz 33", "gazel",
        "gaz 27", "dump", "tipper", "semi", "тягач", "грузовик", "самосвал", "fuso",
        "hino", "belaz", "transporter", "t5 sheik", "sprinter", "transit", "crafter",
        "dumptruck", "cargotrailer", "flatbed", "boxutility", "van"
    };
    for (const QString& kw : s_truckKw) {
        if (lower.contains(kw)) {
            return { "trucks", QStringLiteral("Грузовики") };
        }
    }

    // 3. Racing, Sports, Supercars & Drift
    static const QStringList s_raceKw = {
        "gt3", "gt4", "gt series", "amg gt", "f1", "formula", "kart", "drift", "rally",
        "nascar", "lemans", "wec", "dtm", "mclaren", "p1", "chiron", "veyron", "bugatti",
        "koenigsegg", "pagani", "ferrari", "lamborghini", "huracan", "aventador", "gallardo",
        "murcielago", "911 gt", "viper", "corvette", "saleen", "supercar", "hypercar",
        "r34", "r35", "gtr", "gt r", "918", "senna", "speedtail", "laferrari", "enzo",
        "lotus", "elise", "emira", "exige", "evora", "s14", "180sx", "200sx", "240sx",
        "s13", "s15", "silvia", "supra", "m4", "m2", "m3", "m5", "m8", "vantage",
        "granturismo", "maserati", "amvantage", "superleggera"
    };
    for (const QString& kw : s_raceKw) {
        if (lower.contains(kw)) {
            return { "racing", QStringLiteral("Спорт") };
        }
    }

    // 4. Passenger Cars (Sedans, Wagons, Coupes, Hatches)
    static const QStringList s_carKw = {
        "sedan", "coupe", "hatch", "wagon", "estate", "vaz", "lada", "priora", "granta",
        "kalina", "vesta", "2101", "2102", "2103", "2104", "2105", "2106", "2107", "2108",
        "2109", "2110", "2111", "2112", "2114", "2115", "e30", "e34", "e36", "e38", "e39",
        "e46", "e60", "e90", "e92", "f30", "f10", "g20", "g30",
        "bmw", "audi", "a3", "a4", "a5", "a6", "a7", "a8", "s3", "s4", "s5", "s6",
        "s7", "s8", "rs3", "rs4", "rs5", "rs6", "rs7", "passat", "golf", "jetta", "polo",
        "arteon", "octavia", "superb", "fabia", "rapid", "camry", "corolla", "mark ii",
        "chaser", "cresta", "skyline", "civic", "accord", "integra", "impreza", "legacy",
        "wrx", "sti", "lancer", "evolution", "mustang", "camaro", "challenger", "charger",
        "solaris", "accent", "rio", "ceed", "k4", "k5", "optima", "focus", "mondeo", "fiesta",
        "astra", "vectra", "insignia", "corsa", "w124", "w140", "w201", "w202", "w203", "w204",
        "w210", "w211", "w212", "w220", "w221", "w222", "c63", "e63", "s63", "s65", "cl65",
        "matador", "corrado", "volga", "gaz 24", "gaz24", "claria", "beetle", "aura", "s90",
        "s60", "s40", "v40", "v60", "v90", "panamera", "porsche 944", "944", "300c", "aspen",
        "alero", "crown vic", "crownvictoria", "fusion", "laurel", "c33", "taz"
    };
    for (const QString& kw : s_carKw) {
        if (lower.contains(kw)) {
            return { "cars", QStringLiteral("Легковые авто") };
        }
    }

    // 5. Unknown mod
    return { "unknown", QStringLiteral("! неизвестный") };
}

QString getRegistryFilePath() {
    QString appData = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(appData);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    return dir.filePath("installed_mods.json");
}

} // namespace

InstalledModsManager::InstalledModsManager(QObject* parent)
    : QAbstractListModel(parent) {
    loadRegistry();
}

InstalledModsManager::~InstalledModsManager() {
    saveRegistry();
}

int InstalledModsManager::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_filteredIndices.count();
}

QVariant InstalledModsManager::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_filteredIndices.count())
        return {};

    int realIdx = m_filteredIndices.at(index.row());
    if (realIdx < 0 || realIdx >= m_items.count())
        return {};

    const auto& item = m_items.at(realIdx);
    switch (role) {
    case IdRole:            return item.id;
    case ModIdRole:         return item.modId;
    case TitleRole:         return item.title;
    case FilenameRole:      return item.filename;
    case FilePathRole:      return item.filePath;
    case CategorySlugRole:  return item.categorySlug;
    case CategoryNameRole:  return item.categoryName;
    case FileSizeBytesRole: return item.fileSizeBytes;
    case FileSizeTextRole:  return item.fileSizeText;
    case IsEnabledRole:     return item.enabled;
    case ThumbnailUrlRole:  return item.thumbnailUrl;
    case VersionRole:       return item.version;
    case InstalledDateRole: return item.installedAt.toString("dd.MM.yyyy hh:mm");
    default:
        return {};
    }
}

QHash<int, QByteArray> InstalledModsManager::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[IdRole]            = "modId";
    roles[ModIdRole]         = "stmodsId";
    roles[TitleRole]         = "title";
    roles[FilenameRole]      = "filename";
    roles[FilePathRole]      = "filePath";
    roles[CategorySlugRole]  = "categorySlug";
    roles[CategoryNameRole]  = "categoryName";
    roles[FileSizeBytesRole] = "fileSizeBytes";
    roles[FileSizeTextRole]  = "fileSizeText";
    roles[IsEnabledRole]     = "isEnabled";
    roles[ThumbnailUrlRole]  = "thumbnailUrl";
    roles[VersionRole]       = "version";
    roles[InstalledDateRole] = "installedDate";
    return roles;
}

void InstalledModsManager::setCategoryFilter(const QString& slug) {
    if (m_categoryFilter != slug) {
        m_categoryFilter = slug;
        updateFilter();
        emit filterChanged();
    }
}

void InstalledModsManager::setSearchQuery(const QString& query) {
    if (m_searchQuery != query) {
        m_searchQuery = query;
        updateFilter();
        emit filterChanged();
    }
}

int InstalledModsManager::countForCategory(const QString& catSlug) const {
    if (catSlug == "all" || catSlug.isEmpty()) {
        return m_items.count();
    }
    int count = 0;
    for (const auto& it : m_items) {
        if (it.categorySlug == catSlug) {
            count++;
        }
    }
    return count;
}

void InstalledModsManager::updateFilter() {
    beginResetModel();
    m_filteredIndices.clear();
    QString query = m_searchQuery.trimmed().toLower();

    for (int i = 0; i < m_items.count(); ++i) {
        const auto& it = m_items[i];

        // 1. Category filter
        if (m_categoryFilter != "all" && !m_categoryFilter.isEmpty()) {
            if (it.categorySlug != m_categoryFilter) {
                continue;
            }
        }

        // 2. Search query filter
        if (!query.isEmpty()) {
            if (!it.title.toLower().contains(query) &&
                !it.filename.toLower().contains(query) &&
                !it.categoryName.toLower().contains(query)) {
                continue;
            }
        }

        m_filteredIndices.append(i);
    }
    endResetModel();
}

int InstalledModsManager::enabledCount() const {
    int c = 0;
    for (const auto& it : m_items) {
        if (it.enabled) c++;
    }
    return c;
}

QString InstalledModsManager::totalSizeText() const {
    qint64 total = 0;
    for (const auto& it : m_items) {
        total += it.fileSizeBytes;
    }
    return formatSize(total);
}

QString InstalledModsManager::formatSize(qint64 bytes) {
    if (bytes <= 0) return QStringLiteral("0 МБ");
    double mb = static_cast<double>(bytes) / (1024.0 * 1024.0);
    if (mb >= 1024.0) {
        double gb = mb / 1024.0;
        return QString::number(gb, 'f', 1) + QStringLiteral(" ГБ");
    }
    return QString::number(mb, 'f', 1) + QStringLiteral(" МБ");
}

QString InstalledModsManager::cleanModTitle(const QString& filename) {
    QString name = filename;
    // Strip extension
    name.remove(QRegularExpression(R"(\.zip(?:\.disabled)?$)", QRegularExpression::CaseInsensitiveOption));

    // Strip timestamps or hash suffixes like _1736885433_559914
    name.remove(QRegularExpression(R"(_\d{9,12}_\d+)"));
    // Replace underscores and dashes with spaces
    name.replace('_', ' ');
    name.replace('-', ' ');
    name = name.simplified();
    return name;
}

QString InstalledModsManager::normalizeString(const QString& str) {
    QString s = str.toLower();
    s.remove(QRegularExpression(R"([^\w\d])"));
    return s;
}

void InstalledModsManager::rescan(const QString& gamePath) {
    qDebug() << "InstalledModsManager::rescan called with gamePath=" << gamePath;
    if (gamePath.trimmed().isEmpty()) return;

    m_isScanning = true;
    emit isScanningChanged();
    m_lastGamePath = gamePath;

    // Load any existing registry first to preserve rich metadata
    QMap<QString, InstalledModItem> existingByPath;
    QMap<QString, InstalledModItem> existingByFilename;
    for (const auto& item : m_items) {
        existingByPath[QDir::cleanPath(item.filePath)] = item;
        existingByFilename[item.filename.toLower()] = item;
    }

    QList<InstalledModItem> scannedList;

    auto scanFolder = [&](const QString& subDir, const QSet<QString>& vanillaSet) {
        QDir dir(QDir::cleanPath(gamePath + "/" + subDir));
        if (!dir.exists()) return;

        QStringList filters;
        filters << "*.zip" << "*.zip.disabled";
        dir.setNameFilters(filters);
        dir.setFilter(QDir::Files | QDir::NoSymLinks);

        QFileInfoList list = dir.entryInfoList();
        for (const QFileInfo& fi : list) {
            QString fn = fi.fileName();
            QString lowerFn = fn.toLower();

            // Extract base name without .zip or .zip.disabled
            QString base = lowerFn;
            base.remove(QRegularExpression(R"(\.zip(?:\.disabled)?$)"));

            // Check if vanilla game asset
            if (vanillaSet.contains(base)) {
                continue;
            }

            QString cleanPath = QDir::cleanPath(fi.absoluteFilePath());
            bool isEnabled = !lowerFn.endsWith(".disabled");

            InstalledModItem item;
            // Check if already in registry
            if (existingByPath.contains(cleanPath)) {
                item = existingByPath[cleanPath];
            } else if (existingByFilename.contains(lowerFn)) {
                item = existingByFilename[lowerFn];
            } else {
                // New mod discovered on disk
                item.id = cleanPath;
                item.modId = cleanPath;
                item.title = cleanModTitle(fn);
                item.installedAt = fi.lastModified();
            }

            // Always run smart detection for local files to properly classify jeeps, trucks, sports cars, and unknown
            auto detected = detectModCategory(fn, subDir);
            item.categorySlug = detected.slug;
            item.categoryName = detected.name;

            item.filePath = cleanPath;
            item.filename = fn;
            item.fileSizeBytes = fi.size();
            item.fileSizeText = formatSize(fi.size());
            item.enabled = isEnabled;
            if (item.title.isEmpty()) item.title = cleanModTitle(fn);

            scannedList.append(item);
        }
    };

    scanFolder("content/vehicles", getVanillaVehicles());
    scanFolder("content/levels", getVanillaLevels());

    // Sort by modification date (newest first)
    std::sort(scannedList.begin(), scannedList.end(), [](const InstalledModItem& a, const InstalledModItem& b) {
        return a.installedAt > b.installedAt;
    });

    m_items = scannedList;
    updateFilter();

    saveRegistry();
    qDebug() << "InstalledModsManager::rescan finished. Scanned count=" << m_items.count() << "Registry path=" << getRegistryFilePath();

    m_isScanning = false;
    emit isScanningChanged();
    emit countChanged();
}

bool InstalledModsManager::isModInstalled(const QString& modId,
                                          const QString& title,
                                          const QString& filename) const {
    if (modId.isEmpty() && title.isEmpty() && filename.isEmpty()) {
        return false;
    }

    QString normModId = normalizeString(modId);
    QString normTitle = normalizeString(title);
    QString normFn = normalizeString(filename);

    for (const auto& it : m_items) {
        // 1. Exact modId match
        if (!it.modId.isEmpty() && it.modId == modId) {
            return true;
        }

        // 2. Exact filename match (with or without .disabled)
        if (!filename.isEmpty()) {
            QString itBaseFn = it.filename;
            itBaseFn.remove(QRegularExpression(R"(\.disabled$)", QRegularExpression::CaseInsensitiveOption));
            QString searchFn = filename;
            searchFn.remove(QRegularExpression(R"(\.disabled$)", QRegularExpression::CaseInsensitiveOption));
            if (itBaseFn.compare(searchFn, Qt::CaseInsensitive) == 0) {
                return true;
            }
        }

        // 3. Slug / normalized string matching
        QString itNormFn = normalizeString(it.filename);
        QString itNormTitle = normalizeString(it.title);

        if (!normFn.isEmpty() && itNormFn == normFn) {
            return true;
        }

        // Fuzzy match: if slug contains a significant part of the filename or title
        if (!normModId.isEmpty() && normModId.length() > 8) {
            if (itNormFn.contains(normModId) || normModId.contains(itNormFn)) {
                return true;
            }
        }
        if (!normTitle.isEmpty() && normTitle.length() > 6) {
            if (itNormTitle.contains(normTitle) || normTitle.contains(itNormTitle) ||
                itNormFn.contains(normTitle) || normTitle.contains(itNormFn)) {
                return true;
            }
        }
    }
    return false;
}

bool InstalledModsManager::isModEnabled(const QString& modIdOrFilename) const {
    for (const auto& it : m_items) {
        if (it.modId == modIdOrFilename || it.filePath == modIdOrFilename ||
            it.filename.compare(modIdOrFilename, Qt::CaseInsensitive) == 0) {
            return it.enabled;
        }
    }
    return false;
}

QVariantMap InstalledModsManager::getModInfo(const QString& modIdOrFilename) const {
    for (const auto& it : m_items) {
        if (it.modId == modIdOrFilename || it.filePath == modIdOrFilename ||
            it.filename.compare(modIdOrFilename, Qt::CaseInsensitive) == 0) {
            QVariantMap m;
            m["id"] = it.id;
            m["modId"] = it.modId;
            m["title"] = it.title;
            m["filename"] = it.filename;
            m["filePath"] = it.filePath;
            m["categorySlug"] = it.categorySlug;
            m["categoryName"] = it.categoryName;
            m["fileSizeBytes"] = it.fileSizeBytes;
            m["fileSizeText"] = it.fileSizeText;
            m["isEnabled"] = it.enabled;
            m["thumbnailUrl"] = it.thumbnailUrl;
            m["version"] = it.version;
            return m;
        }
    }
    return {};
}

bool InstalledModsManager::toggleMod(const QString& filePath) {
    int idx = -1;
    for (int i = 0; i < m_items.count(); ++i) {
        if (QDir::cleanPath(m_items[i].filePath) == QDir::cleanPath(filePath)) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return false;

    InstalledModItem& item = m_items[idx];
    QFileInfo fi(item.filePath);
    if (!fi.exists()) return false;

    QString newPath;
    bool newEnabled = false;

    if (item.enabled) {
        // Disable: rename .zip -> .zip.disabled
        newPath = item.filePath + ".disabled";
        newEnabled = false;
    } else {
        // Enable: rename .zip.disabled -> .zip
        if (item.filePath.endsWith(".disabled", Qt::CaseInsensitive)) {
            newPath = item.filePath.left(item.filePath.length() - 9);
        } else {
            newPath = item.filePath;
        }
        newEnabled = true;
    }

    if (!QFile::rename(item.filePath, newPath)) {
        qWarning() << "Failed to rename mod file:" << item.filePath << "to" << newPath;
        return false;
    }

    item.filePath = newPath;
    item.filename = QFileInfo(newPath).fileName();
    item.enabled = newEnabled;

    updateFilter();
    emit modToggled(newPath, newEnabled);
    emit countChanged();

    saveRegistry();
    return true;
}

bool InstalledModsManager::deleteMod(const QString& filePath) {
    int idx = -1;
    for (int i = 0; i < m_items.count(); ++i) {
        if (QDir::cleanPath(m_items[i].filePath) == QDir::cleanPath(filePath)) {
            idx = i;
            break;
        }
    }
    if (idx == -1) return false;

    QString modTitle = m_items[idx].title;
    QString modId = m_items[idx].modId;

    QFile file(m_items[idx].filePath);
    if (file.exists()) {
        file.remove();
    }

    m_items.removeAt(idx);
    updateFilter();

    saveRegistry();
    emit modUninstalled(modId, modTitle);
    emit countChanged();
    return true;
}

void InstalledModsManager::openModFolder(const QString& filePath) {
    QFileInfo fi(filePath);
    if (!fi.exists()) return;

#if defined(Q_OS_WIN)
    QString native = QDir::toNativeSeparators(fi.absoluteFilePath());
    QProcess::startDetached("explorer.exe", QStringList() << "/select," + native);
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(fi.dir().absolutePath()));
#endif
}

void InstalledModsManager::recordInstalledMod(const QString& modId,
                                              const QString& title,
                                              const QString& categorySlug,
                                              const QString& archivePath,
                                              const QString& thumbnailUrl,
                                              const QString& version) {
    QFileInfo fi(archivePath);
    if (!fi.exists()) return;

    QString cleanPath = QDir::cleanPath(archivePath);

    // Check if already in list
    for (int i = 0; i < m_items.count(); ++i) {
        if (QDir::cleanPath(m_items[i].filePath) == cleanPath) {
            m_items[i].modId = modId;
            m_items[i].title = title;
            m_items[i].thumbnailUrl = thumbnailUrl;
            m_items[i].version = version;
            m_items[i].categorySlug = categorySlug;
            emit dataChanged(index(i), index(i));
            saveRegistry();
            emit modInstalled(modId, title);
            return;
        }
    }

    InstalledModItem item;
    item.id = cleanPath;
    item.modId = modId;
    item.title = title.isEmpty() ? cleanModTitle(fi.fileName()) : title;
    item.filename = fi.fileName();
    item.filePath = cleanPath;
    item.categorySlug = categorySlug;
    item.categoryName = categorySlug.contains("map") ? QStringLiteral("Карты") : QStringLiteral("Легковые авто");
    item.fileSizeBytes = fi.size();
    item.fileSizeText = formatSize(fi.size());
    item.enabled = !fi.fileName().endsWith(".disabled", Qt::CaseInsensitive);
    item.thumbnailUrl = thumbnailUrl;
    item.version = version;
    item.installedAt = QDateTime::currentDateTime();

    m_items.prepend(item);
    updateFilter();

    saveRegistry();
    emit modInstalled(modId, title);
    emit countChanged();
}

void InstalledModsManager::loadRegistry() {
    QString regPath = getRegistryFilePath();
    QFile file(regPath);
    if (!file.open(QIODevice::ReadOnly)) return;

    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    if (!doc.isArray()) return;

    QJsonArray arr = doc.array();
    m_items.clear();

    for (const QJsonValue& val : arr) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();

        InstalledModItem it;
        it.id = obj["id"].toString();
        it.modId = obj["modId"].toString();
        it.title = obj["title"].toString();
        it.filename = obj["filename"].toString();
        it.filePath = obj["filePath"].toString();
        it.categorySlug = obj["categorySlug"].toString();
        it.categoryName = obj["categoryName"].toString();
        it.fileSizeBytes = obj["fileSizeBytes"].toVariant().toLongLong();
        it.fileSizeText = obj["fileSizeText"].toString();
        it.enabled = obj["enabled"].toBool(true);
        it.thumbnailUrl = obj["thumbnailUrl"].toString();
        it.version = obj["version"].toString();
        it.installedAt = QDateTime::fromString(obj["installedAt"].toString(), Qt::ISODate);

        // Check if file still exists on disk
        if (QFile::exists(it.filePath)) {
            // Re-detect category properly to update any old default "cars" to jeeps, trucks, maps, sport or unknown!
            QString subDir = it.filePath.contains("levels", Qt::CaseInsensitive) ? "content/levels" : "content/vehicles";
            auto detected = detectModCategory(it.filename, subDir);
            it.categorySlug = detected.slug;
            it.categoryName = detected.name;

            m_items.append(it);
        }
    }
    updateFilter();
    saveRegistry();
}

void InstalledModsManager::saveRegistry() {
    QString regPath = getRegistryFilePath();
    QFile file(regPath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return;

    QJsonArray arr;
    for (const auto& it : m_items) {
        QJsonObject obj;
        obj["id"] = it.id;
        obj["modId"] = it.modId;
        obj["title"] = it.title;
        obj["filename"] = it.filename;
        obj["filePath"] = it.filePath;
        obj["categorySlug"] = it.categorySlug;
        obj["categoryName"] = it.categoryName;
        obj["fileSizeBytes"] = it.fileSizeBytes;
        obj["fileSizeText"] = it.fileSizeText;
        obj["enabled"] = it.enabled;
        obj["thumbnailUrl"] = it.thumbnailUrl;
        obj["version"] = it.version;
        obj["installedAt"] = it.installedAt.toString(Qt::ISODate);
        arr.append(obj);
    }

    QJsonDocument doc(arr);
    file.write(doc.toJson());
    file.close();
}
