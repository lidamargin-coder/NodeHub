#pragma once

#include <QString>
#include <QStringList>
#include <QVariantMap>

struct ModSummary {
    QString id;           // Path on stmods, e.g. /beamng_drive/mods/1976_ibishu_claria_by_lj74_for_beamng/
    QString url;          // Full URL
    QString title;        // Mod display title
    QString version;      // e.g. 1.10.0
    QString gameVersion;  // e.g. v0.40.x
    QString author;       // e.g. LJ74
    QString date;         // e.g. вчера в 16:15
    QString thumbnailUrl; // Preview image URL
    QString snippet;      // Short description text
    QString categorySlug; // e.g. cars, maps, jeeps, trucks, racing, others
};

struct ModDetail {
    QString id;
    QString url;
    QString title;
    QString version;
    QString gameVersion;
    QString author;
    QString categorySlug;
    QString categoryName;
    QString sizeText;     // e.g. 251,44 МБ
    QString description;
    QStringList features;
    QString changelogTitle;
    QString changelogText;
    QStringList galleryUrls;
    QString downloadRoute; // e.g. https://stmods.org/download_file.php?update_id=171816&path=link1
    QString hosterName;    // e.g. ModsFire

    QVariantMap toVariantMap() const {
        QVariantMap map;
        map["id"] = id;
        map["url"] = url;
        map["title"] = title;
        map["version"] = version;
        map["gameVersion"] = gameVersion;
        map["author"] = author;
        map["categorySlug"] = categorySlug;
        map["categoryName"] = categoryName;
        map["sizeText"] = sizeText;
        map["description"] = description;
        map["features"] = features;
        map["changelogTitle"] = changelogTitle;
        map["changelogText"] = changelogText;
        map["galleryUrls"] = galleryUrls;
        map["downloadRoute"] = downloadRoute;
        map["hosterName"] = hosterName;
        return map;
    }
};
