/*
 * Copyright (C) 2007 Matthew Woehlke <mw_triad@users.sourceforge.net>
 * Copyright (C) 2007 Jeremy Whiting <jpwhiting@kde.org>
 * Copyright (C) 2016 Olivier Churlaud <olivier@churlaud.com>
 * Copyright (C) 2019 Kai Uwe Broulik <kde@privat.broulik.de>
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

struct ColorsModelData
{
    QString display;
    QString schemeName;
    QPalette palette;
    QColor activeTitleBarBackground;
    QColor activeTitleBarForeground;
    bool removable;
    bool pendingDeletion;
};
Q_DECLARE_TYPEINFO(ColorsModelData, Q_MOVABLE_TYPE);

class ColorsModel : public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString selectedScheme READ selectedScheme WRITE setSelectedScheme NOTIFY selectedSchemeChanged)
    Q_PROPERTY(int selectedSchemeIndex READ selectedSchemeIndex NOTIFY selectedSchemeIndexChanged)

public:
    ColorsModel(QObject *parent);
    ~ColorsModel() override;

    enum Roles {
        SchemeNameRole = Qt::UserRole + 1,
        PaletteRole,
        // Colors which aren't in QPalette
        ActiveTitleBarBackgroundRole,
        ActiveTitleBarForegroundRole,
        RemovableRole,
        PendingDeletionRole
    };

    int rowCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    QHash<int, QByteArray> roleNames() const override;

    QString selectedScheme() const;
    void setSelectedScheme(const QString &scheme);

    int indexOfScheme(const QString &scheme) const;
    int selectedSchemeIndex() const;

    QStringList pendingDeletions() const;
    void removeItemsPendingDeletion();

    void load();

Q_SIGNALS:
    void selectedSchemeChanged(const QString &scheme);
    void selectedSchemeIndexChanged();

    void pendingDeletionsChanged();

private:
    QString m_selectedScheme;

    QVector<ColorsModelData> m_data;

};
