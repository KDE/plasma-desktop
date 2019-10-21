/*
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 * KDE Frameworks 5 port Copyright (C) 2013 Jonathan Riddell <jr@jriddell.org>
 * Copyright (c) 2018 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * Requires the Qt widget libraries, available at no cost at
 * https://www.qt.io/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>

class IconsSettings;

struct IconsModelData
{
    QString display;
    QString themeName;
    QString description;
    bool removable;
    bool pendingDeletion;
};
Q_DECLARE_TYPEINFO(IconsModelData, Q_MOVABLE_TYPE);

class IconsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    IconsModel(IconsSettings *iconsSettings, QObject *parent = nullptr);
    ~IconsModel() override;

    enum Roles {
        ThemeNameRole = Qt::UserRole + 1,
        DescriptionRole,
        RemovableRole,
        PendingDeletionRole
    };

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    QStringList pendingDeletions() const;
    void removeItemsPendingDeletion();

    void load();

signals:
    void pendingDeletionsChanged();

private:
    QVector<IconsModelData> m_data;
    IconsSettings *m_settings;

};
