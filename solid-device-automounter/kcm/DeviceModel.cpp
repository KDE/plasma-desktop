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

#include "DeviceModel.h"

#include <QIcon>

#include <KLocalizedString>
#include <Solid/DeviceNotifier>
#include <Solid/Device>
#include <Solid/StorageVolume>

#include "AutomounterSettings.h"

DeviceModel::DeviceModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    reload();
    
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, &DeviceModel::deviceAttached);
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved, this, &DeviceModel::deviceRemoved);
}

void DeviceModel::forgetDevice(const QString &udi)
{
    if (m_disconnected.contains(udi)) {
        const int deviceIndex = m_disconnected.indexOf(udi);
        beginRemoveRows(index(1, 0), deviceIndex, deviceIndex);
        m_disconnected.removeOne(udi);
        endRemoveRows();
    } else if (m_attached.contains(udi)) {
        const int deviceIndex = m_attached.indexOf(udi);
        beginRemoveRows(index(0, 0), deviceIndex, deviceIndex);
        m_attached.removeOne(udi);
        endRemoveRows();
    }
    m_loginForced.remove(udi);
    m_attachedForced.remove(udi);
}

QVariant DeviceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(section) {
        case 0: return i18n("Device");
        case 1: return i18n("Automount on Login");
        case 2: return i18n("Automount on Attach");
        }
    }
    return QVariant();
}

void DeviceModel::deviceAttached(const QString &udi)
{
    const Solid::Device devive(udi);
    auto volume = devive.as<Solid::StorageVolume>();

    if (volume && !volume->isIgnored()) {
        if (m_disconnected.contains(udi)) {
            const int deviceIndex = m_disconnected.indexOf(udi);
            beginRemoveRows(index(1, 0), deviceIndex, deviceIndex);
            m_disconnected.removeOne(udi);
            endRemoveRows();
        }

        addNewDevice(udi);
    }
}

void DeviceModel::deviceRemoved(const QString &udi)
{
    if (m_attached.contains(udi)) {
        const int deviceIndex = m_attached.indexOf(udi);

        beginRemoveRows(index(0, 0), deviceIndex, deviceIndex);
        m_attached.removeOne(udi);
        endRemoveRows();

        // NOTE the device is not moved to the "Disconnected" section
        // when removing it while the KCM is opened because we need to check
        // whether the device that just got detached is ignored
        // (don't show partition tables and other garbage) but this information
        // is no longer available when the device is gone
    }
}

void DeviceModel::addNewDevice(const QString &udi)
{
    AutomounterSettings::self()->load();

    if (!m_loginForced.contains(udi)) {
        m_loginForced[udi] = AutomounterSettings::deviceAutomountIsForced(udi, AutomounterSettings::Login);
    }
    if (!m_attachedForced.contains(udi)) {
        m_loginForced[udi] = AutomounterSettings::deviceAutomountIsForced(udi, AutomounterSettings::Attach);
    }

    const Solid::Device dev(udi);
    if (dev.isValid()) {
        beginInsertRows(index(0, 0), m_attached.size(), m_attached.size());
        m_attached << udi;
    } else {
        beginInsertRows(index(1, 0), m_disconnected.size(), m_disconnected.size());
        m_disconnected << udi;
    }

    endInsertRows();
}

void DeviceModel::reload()
{
    beginResetModel();
    m_loginForced.clear();
    m_attachedForced.clear();
    m_attached.clear();
    m_disconnected.clear();

    foreach (const QString &dev, AutomounterSettings::knownDevices()) {
        addNewDevice(dev);
    }
    foreach (const QString &udi, m_loginForced.keys()) {
        m_loginForced[udi] = AutomounterSettings::deviceAutomountIsForced(udi, AutomounterSettings::Login);
        m_attachedForced[udi] = AutomounterSettings::deviceAutomountIsForced(udi, AutomounterSettings::Attach);
    }
    endResetModel();
}

QModelIndex DeviceModel::index(int row, int column, const QModelIndex &parent) const
{
    if (parent.isValid()) {
        if (parent.column() > 0) {
            return QModelIndex();
        }

        if (parent.row() == 0) {
            if (row >= 0 && row < m_attached.size() && column >= 0 && column <= 2) {
                return createIndex(row, column, static_cast<quintptr>(0));
            }
        } else if (parent.row() == 1) {
            if (row >= 0 && row < m_disconnected.size() && column >= 0 && column <= 2) {
                return createIndex(row, column, 1);
            }
        }
    } else {
         if ((row == 0 || row == 1) && column >= 0 && column <= 2) {
            return createIndex(row, column, 3);
         }
    }
    return QModelIndex();
}

QModelIndex DeviceModel::parent(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (index.internalId() == 3)
            return QModelIndex();
        return createIndex(index.internalId(), 0, 3);
    }
    return QModelIndex();
}

Qt::ItemFlags DeviceModel::flags(const QModelIndex &index) const
{
    if (index.isValid()) {
        if (index.parent().isValid()) {
            if (index.column() > 0) {
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
            } else if (index.column() == 0) {
                return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
            }
        } else {
            return Qt::ItemIsEnabled;
        }
    }

    return Qt::NoItemFlags;
}

bool DeviceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::CheckStateRole && index.column() > 0) {
        const QString &udi = index.data(Qt::UserRole).toString();
        switch(index.column()) {
        case 1:
            m_loginForced[udi] = (value.toInt() == Qt::Checked) ? true : false;
            break;
        case 2:
            m_attachedForced[udi] = (value.toInt() == Qt::Checked) ? true : false;
            break;
        }
        emit dataChanged(index, index);
        return true;
    }

    return false;
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid() && index.parent().isValid()) {
        if (index.parent().row() == 0) {
            if (role == TypeRole) {
				return Attached;
            }

            const QString &udi = m_attached.at(index.row());
            Solid::Device dev(udi);

            if (role == Qt::UserRole) {
                return udi;
            }

            if (index.column() == 0) {
                switch(role) {
                case Qt::DisplayRole:
                    return dev.description();
                case Qt::ToolTipRole:
                    return i18n("UDI: %1", udi);
                case Qt::DecorationRole:
                    return QIcon::fromTheme(dev.icon());
                }
            } else if (index.column() == 1) {
                switch(role) {
                case Qt::CheckStateRole:
                    return m_loginForced[udi] ? Qt::Checked : Qt::Unchecked;
                case Qt::ToolTipRole:
                    if (m_loginForced[udi] || AutomounterSettings::shouldAutomountDevice(udi, AutomounterSettings::Login))
                        return i18n("This device will be automatically mounted at login.");
                    return i18n("This device will not be automatically mounted at login.");
                }
            } else if (index.column() == 2) {
                switch(role) {
                case Qt::CheckStateRole:
                    return m_attachedForced[udi] ? Qt::Checked : Qt::Unchecked;
                case Qt::ToolTipRole:
                    if (m_attachedForced[udi] || AutomounterSettings::shouldAutomountDevice(udi, AutomounterSettings::Attach))
                        return i18n("This device will be automatically mounted when attached.");
                    return i18n("This device will not be automatically mounted when attached.");
                }
            }
        } else if (index.parent().row() == 1) {
            if (role == TypeRole) {
				return Detatched;
            }

            const QString &udi = m_disconnected[index.row()];

            if (role == Qt::UserRole) {
                return udi;
            }

            if (index.column() == 0) {
                switch(role) {
                case Qt::DisplayRole:
                    return AutomounterSettings::getDeviceName(udi);
                case Qt::ToolTipRole:
                    return i18n("UDI: %1", udi);
                case Qt::DecorationRole:
                    return QIcon::fromTheme(AutomounterSettings::getDeviceIcon(udi));
                }
            } else if (index.column() == 1) {
                switch(role) {
                case Qt::CheckStateRole:
                    return m_loginForced[udi] ? Qt::Checked : Qt::Unchecked;
                case Qt::ToolTipRole:
                    if (m_loginForced[udi] || AutomounterSettings::shouldAutomountDevice(udi, AutomounterSettings::Login))
                        return i18n("This device will be automatically mounted at login.");
                    return i18n("This device will not be automatically mounted at login.");
                }
            } else if (index.column() == 2) {
                switch(role) {
                case Qt::CheckStateRole:
                    return m_attachedForced[udi] ? Qt::Checked : Qt::Unchecked;
                case Qt::ToolTipRole:
                    if (m_attachedForced[udi] || AutomounterSettings::shouldAutomountDevice(udi, AutomounterSettings::Attach))
                        return i18n("This device will be automatically mounted when attached.");
                    return i18n("This device will not be automatically mounted when attached.");
                }
            }
        }
    } else if (index.isValid()) {
        if (role == Qt::DisplayRole && index.column() == 0) {
            if (index.row() == 0) {
                return i18n("Attached Devices");
            } else if (index.row() == 1) {
                return i18n("Disconnected Devices");
            }
        }
    }
    return QVariant();
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) {
        if (parent.internalId() < 3 || parent.column() > 0) {
            return 0;
        }
        if (parent.row() == 0) {
            return m_attached.size();
        }

        return m_disconnected.size();
    }

    return 2;
}

int DeviceModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return 3;
}
