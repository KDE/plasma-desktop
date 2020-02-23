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

#include <QAbstractListModel>

class BalooSettings;

class FilteredFolderModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit FilteredFolderModel(BalooSettings *settings, QObject *parent);

    QStringList includeFolders() const;

    enum Roles {
        Folder = Qt::UserRole + 1,
        Url
    };

    QVariant data(const QModelIndex& idx, int role) const override;
    int rowCount(const QModelIndex& parent) const override;

    Q_INVOKABLE void addFolder(const QString& folder);
    Q_INVOKABLE void removeFolder(int row);
    QHash<int, QByteArray> roleNames() const override;

public slots:
    void updateDirectoryList();

private:
    QString folderDisplayName(const QString& url) const;
    QString fetchMountPoint(const QString& url) const;
    void showMessage(const QString& message);

    /**
     * @brief Get the theme valid icon name for \p path.
     *
     * @param path Path to be analysed.
     * @return One of: "user-home", "drive-harddisk" or "folder"
     */
    QString iconName(QString path) const;

    BalooSettings *m_settings;
    QStringList m_mountPoints;
    QStringList m_excludeList;
};

#endif // FILTEREDFOLDERMODEL_H
