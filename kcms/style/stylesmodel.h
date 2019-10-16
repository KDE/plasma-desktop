/*
 * Copyright (C) 2019 Kai Uwe Broulik <kde@pbroulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QAbstractListModel>
#include <QString>
#include <QVector>

struct StylesModelData
{
    QString display;
    QString styleName;
    QString description;
    QString configPage;
};
Q_DECLARE_TYPEINFO(StylesModelData, Q_MOVABLE_TYPE);

class StylesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString selectedStyle READ selectedStyle WRITE setSelectedStyle NOTIFY selectedStyleChanged)
    Q_PROPERTY(int selectedStyleIndex READ selectedStyleIndex NOTIFY selectedStyleIndexChanged)

public:
    StylesModel(QObject *parent);
    ~StylesModel() override;

    enum Roles {
        StyleNameRole = Qt::UserRole + 1,
        DescriptionRole,
        ConfigurableRole
    };

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QHash<int, QByteArray> roleNames() const override;

    QString selectedStyle() const;
    void setSelectedStyle(const QString &style);

    int indexOfStyle(const QString &style) const;
    int selectedStyleIndex() const;

    QString styleConfigPage(const QString &style) const;

    void load();

Q_SIGNALS:
    void selectedStyleChanged(const QString &style);
    void selectedStyleIndexChanged();

private:
    QString m_selectedStyle;

    QVector<StylesModelData> m_data;

};
