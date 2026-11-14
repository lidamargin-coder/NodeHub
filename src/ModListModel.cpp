#include "ModListModel.h"

ModListModel::ModListModel(QObject* parent)
    : QAbstractListModel(parent) {}

int ModListModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) return 0;
    return m_mods.count();
}

QVariant ModListModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() < 0 || index.row() >= m_mods.count())
        return {};

    const ModSummary& mod = m_mods.at(index.row());
    switch (role) {
    case IdRole:          return mod.id;
    case UrlRole:         return mod.url;
    case TitleRole:       return mod.title;
    case VersionRole:     return mod.version;
    case GameVersionRole: return mod.gameVersion;
    case AuthorRole:      return mod.author;
    case DateRole:        return mod.date;
    case ThumbnailRole:   return mod.thumbnailUrl;
    case SnippetRole:     return mod.snippet;
    case CategorySlugRole:return mod.categorySlug;
    default:              return {};
    }
}

QHash<int, QByteArray> ModListModel::roleNames() const {
    return {
        { IdRole,           "modId" },
        { UrlRole,          "modUrl" },
        { TitleRole,        "title" },
        { VersionRole,      "version" },
        { GameVersionRole,  "gameVersion" },
        { AuthorRole,       "author" },
        { DateRole,         "date" },
        { ThumbnailRole,    "thumbnailUrl" },
        { SnippetRole,      "snippet" },
        { CategorySlugRole, "categorySlug" }
    };
}

void ModListModel::setMods(const QList<ModSummary>& mods) {
    beginResetModel();
    m_mods = mods;
    endResetModel();
    emit countChanged();
}

void ModListModel::appendMods(const QList<ModSummary>& mods) {
    if (mods.isEmpty()) return;
    int first = m_mods.count();
    int last = first + mods.count() - 1;
    beginInsertRows(QModelIndex(), first, last);
    m_mods.append(mods);
    endInsertRows();
    emit countChanged();
}

void ModListModel::clear() {
    beginResetModel();
    m_mods.clear();
    endResetModel();
    emit countChanged();
}

QVariantMap ModListModel::get(int index) const {
    if (index < 0 || index >= m_mods.count()) return {};
    const auto& m = m_mods.at(index);
    return {
        { "modId",        m.id },
        { "modUrl",       m.url },
        { "title",        m.title },
        { "version",      m.version },
        { "gameVersion",  m.gameVersion },
        { "author",       m.author },
        { "date",         m.date },
        { "thumbnailUrl", m.thumbnailUrl },
        { "snippet",      m.snippet },
        { "categorySlug", m.categorySlug }
    };
}
