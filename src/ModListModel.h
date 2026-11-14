#pragma once

#include "ModItem.h"
#include <QAbstractListModel>
#include <QList>

class ModListModel : public QAbstractListModel {
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        UrlRole,
        TitleRole,
        VersionRole,
        GameVersionRole,
        AuthorRole,
        DateRole,
        ThumbnailRole,
        SnippetRole,
        CategorySlugRole
    };

    explicit ModListModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setMods(const QList<ModSummary>& mods);
    void appendMods(const QList<ModSummary>& mods);
    void clear();

    Q_INVOKABLE QVariantMap get(int index) const;

signals:
    void countChanged();

private:
    QList<ModSummary> m_mods;
};
