/*
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * Copyright (C) 2007 Jeremy Whiting <jpwhiting@kde.org>
 * Copyright (C) 2016 Olivier Churlaud <olivier@churlaud.com>
 * Copyright (C) 2019 Kai Uwe Broulik <kde@privat.broulik.de>
 * Copyright (C) 2019 David Redondo <kde@david-redondo.de>
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
#include <QPalette>
#include <QVector>

#include <memory>

struct ThemesModelData;

class ThemesModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString selectedTheme READ selectedTheme WRITE setSelectedTheme NOTIFY selectedThemeChanged)
    Q_PROPERTY(int selectedThemeIndex READ selectedThemeIndex NOTIFY selectedThemeChanged)

public:
    ThemesModel(QObject *parent);
    ~ThemesModel() override;

    enum Roles {
        PluginNameRole = Qt::UserRole + 1,
        ThemeNameRole,
        DescriptionRole,
        FollowsSystemColorsRole,
        ColorTypeRole,
        IsLocalRole,
        PendingDeletionRole
    };
    Q_ENUM(Roles)
    enum ColorType {
        LightTheme,
        DarkTheme,
        FollowsColorTheme
    };
    Q_ENUM(ColorType)


    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    QHash<int, QByteArray> roleNames() const override;

    QString selectedTheme() const;
    void setSelectedTheme(const QString &pluginName);

    int pluginIndex(const QString &pluginName) const;
    int selectedThemeIndex() const;

    QStringList pendingDeletions() const;
    void removeRow(int row);

    void load();

Q_SIGNALS:
    void selectedThemeChanged(const QString &pluginName);
    void selectedThemeIndexChanged();

    void pendingDeletionsChanged();

private:
    QString m_selectedTheme;
    //Can't use QVector because unique_ptr causes deletion of copy-ctor
    QVector<ThemesModelData> m_data;

};

struct ThemesModelData
{
    QString display;
    QString pluginName;
    QString description;
    ThemesModel::ColorType type;
    bool isLocal;
    bool pendingDeletion;
};
