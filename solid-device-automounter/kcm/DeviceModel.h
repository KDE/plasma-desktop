/**************************************************************************
 *   Copyright (C) 2009-2010 Trever Fischer <tdfischer@fedoraproject.org>  *
 *   Copyright (C) 2015 Kai Uwe Broulik <kde@privat.broulik.de>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H

#include <QAbstractItemModel>
#include <QHash>
#include <QList>
#include <QModelIndex>
#include <QVariant>

class AutomounterSettings;

class DeviceModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit DeviceModel(AutomounterSettings *m_settings, QObject *parent = nullptr);
    ~DeviceModel() override = default;

    enum DeviceType {
        Attached,
        Detatched,
    };

    enum {
        UdiRole = Qt::UserRole,
        TypeRole,
    };

    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void setAutomaticMountOnLogin(bool automaticLogin);
    void setAutomaticMountOnPlugin(bool automaticAttached);

public Q_SLOTS:
    void forgetDevice(const QString &udi);
    void reload();

private Q_SLOTS:
    void deviceAttached(const QString &udi);
    void deviceRemoved(const QString &udi);

private:
    void addNewDevice(const QString &udi);

    QList<QString> m_attached;
    QList<QString> m_disconnected;
    QHash<QString, bool> m_loginForced;
    QHash<QString, bool> m_attachedForced;
    bool m_automaticLogin;
    bool m_automaticAttached;
    AutomounterSettings *m_settings;
};

#endif
