/*
 * This file is part of the KDE Baloo project
 * Copyright (C) 2014  Vishesh Handa <me@vhanda.in>
 * Copyright (c) 2020 Benjamin Port <benjamin.port@enioka.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef FILTEREDFOLDERMODEL_H
#define FILTEREDFOLDERMODEL_H

#include <Baloo/IndexerConfig>
#include <QAbstractListModel>

class BalooSettings;

class FilteredFolderModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit FilteredFolderModel(BalooSettings *settings, QObject *parent);

    enum Roles {
        Folder = Qt::UserRole + 1,
        Url,
        EnableIndex,
        Deletable,
    };

    QVariant data(const QModelIndex &idx, int role) const override;
    bool setData(const QModelIndex &idx, const QVariant &value, int role) override;
    int rowCount(const QModelIndex &parent) const override;

    Q_INVOKABLE void addFolder(const QString &folder, const bool included);
    Q_INVOKABLE void removeFolder(int row);
    QHash<int, QByteArray> roleNames() const override;

public Q_SLOTS:
    void updateDirectoryList();

private:
    BalooSettings *m_settings;
    Baloo::IndexerConfig m_runtimeConfig;

    struct FolderInfo {
        QString url;
        QString displayName;
        QString icon;
        bool enableIndex;
        bool isFromConfig;
    };

    QVector<FolderInfo> m_folderList;
    QStringList m_deletedSettings; //< track deleted entries

    void syncFolderConfig(const FolderInfo &entry);
};

#endif // FILTEREDFOLDERMODEL_H
