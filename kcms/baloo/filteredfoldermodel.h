/*
    This file is part of the KDE Baloo project
    SPDX-FileCopyrightText: 2014 Vishesh Handa <me@vhanda.in>
    SPDX-FileCopyrightText: 2020 Benjamin Port <benjamin.port@enioka.com>

    SPDX-License-Identifier: LGPL-2.1-or-later

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
