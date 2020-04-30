/*
 * Copyright (c) 2019 Benjamin Port <benjamin.port@enioka.com>
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

struct IconSizeCategoryModelData
{
    QString configKey;
    QString display;
    QString configSection;
    int kIconloaderGroup;
};

Q_DECLARE_TYPEINFO(IconSizeCategoryModelData, Q_MOVABLE_TYPE);

class IconSizeCategoryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    IconSizeCategoryModel(QObject *parent);
    ~IconSizeCategoryModel() override;

    enum Roles {
        ConfigKeyRole = Qt::UserRole + 1,
        ConfigSectionRole,
        KIconLoaderGroupRole,
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    void load();

signals:
    void categorySelectedIndexChanged();

private:
    QVector<IconSizeCategoryModelData> m_data;
};
