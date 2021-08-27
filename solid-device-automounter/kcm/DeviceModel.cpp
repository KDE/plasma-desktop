/*
    SPDX-FileCopyrightText: 2009-2010 Trever Fischer <tdfischer@fedoraproject.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DeviceModel.h"

#include <QIcon>

#include <KLocalizedString>
#include <Solid/Device>
#include <Solid/DeviceNotifier>
#include <Solid/StorageAccess>
#include <Solid/StorageVolume>

#include "AutomounterSettings.h"

DeviceModel::DeviceModel(AutomounterSettings *m_settings, QObject *parent)
    : QAbstractItemModel(parent)
    , m_settings(m_settings)
{
    reload();

    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceAdded, this, &DeviceModel::deviceAttached);
    connect(Solid::DeviceNotifier::instance(), &Solid::DeviceNotifier::deviceRemoved, this, &DeviceModel::deviceRemoved);
}

void DeviceModel::forgetDevice(const QString &udi)
{
    if (m_disconnected.contains(udi)) {
        const int deviceIndex = m_disconnected.indexOf(udi);
        beginRemoveRows(index(RowDetached, 0), deviceIndex, deviceIndex);
        m_disconnected.removeOne(udi);
        endRemoveRows();
    } else if (m_attached.contains(udi)) {
        const int deviceIndex = m_attached.indexOf(udi);
        beginRemoveRows(index(RowAttached, 0), deviceIndex, deviceIndex);
        m_attached.removeOne(udi);
        endRemoveRows();
    }
    m_loginForced.remove(udi);
    m_attachedForced.remove(udi);
}

QVariant DeviceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return i18n("Device");
        case 1:
            return i18nc("As in automount on login", "On Login");
        case 2:
            return i18nc("As in automount on attach", "On Attach");
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
            beginRemoveRows(index(RowDetached, 0), deviceIndex, deviceIndex);
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

        beginRemoveRows(index(RowAttached, 0), deviceIndex, deviceIndex);
        m_attached.removeOne(udi);
        endRemoveRows();

        // We move the device to the "Disconnected" section only if it
        // is a known device, meaning we have some setting for this device.
        // Otherwise the device is not moved to the "Disconnected" section
        // because we need to check whether the device that just got detached is ignored
        // (don't show partition tables and other garbage) but this information
        // is no longer available once the device is gone
        if (m_settings->knownDevices().contains(udi)) {
            beginInsertRows(index(RowDetached, 0), m_disconnected.size(), m_disconnected.size());
            m_disconnected << udi;
            endInsertRows();
        }
    }
}

void DeviceModel::addNewDevice(const QString &udi)
{
    m_settings->load();

    if (!m_loginForced.contains(udi)) {
        m_loginForced[udi] = m_settings->deviceAutomountIsForced(udi, AutomounterSettings::Login);
    }
    if (!m_attachedForced.contains(udi)) {
        m_loginForced[udi] = m_settings->deviceAutomountIsForced(udi, AutomounterSettings::Attach);
    }

    const Solid::Device dev(udi);
    if (dev.isValid()) {
        if (dev.is<Solid::StorageAccess>()) {
            const Solid::StorageAccess *access = dev.as<Solid::StorageAccess>();
            if (!access->isIgnored() || !access->isAccessible()) {
                beginInsertRows(index(RowAttached, 0), m_attached.size(), m_attached.size());
                m_attached << udi;
                endInsertRows();
            }
        }
    } else {
        beginInsertRows(index(RowDetached, 0), m_disconnected.size(), m_disconnected.size());
        m_disconnected << udi;
        endInsertRows();
    }
}

void DeviceModel::reload()
{
    beginResetModel();
    m_loginForced.clear();
    m_attachedForced.clear();
    m_attached.clear();
    m_disconnected.clear();

    m_automaticLogin = m_settings->automountOnLogin();
    m_automaticAttached = m_settings->automountOnPlugin();

    const auto knownDevices = m_settings->knownDevices();
    for (const QString &dev : knownDevices) {
        addNewDevice(dev);
    }
    const auto keys = m_loginForced.keys();
    for (const QString &udi : keys) {
        m_loginForced[udi] = m_settings->deviceAutomountIsForced(udi, AutomounterSettings::Login);
        m_attachedForced[udi] = m_settings->deviceAutomountIsForced(udi, AutomounterSettings::Attach);
    }
    endResetModel();
}

QModelIndex DeviceModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column < 0 || column >= columnCount()) {
        return QModelIndex();
    }
    if (parent.isValid()) {
        if (parent.column() > 0) {
            return QModelIndex();
        }

        const int deviceCount = (parent.row() == RowAttached) ? m_attached.size() : m_disconnected.size();
        if (row < deviceCount) {
            return createIndex(row, column, parent.row());
        }
    } else {
        if (row < rowCount()) {
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
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    if (!index.parent().isValid()) {
        return (m_automaticLogin && m_automaticAttached) ? Qt::NoItemFlags : Qt::ItemIsEnabled;
    }

    // Select only detached devices to be removed
    Qt::ItemFlag selectableFlag = index.parent().row() == RowDetached ? Qt::ItemIsSelectable : Qt::NoItemFlags;

    switch (index.column()) {
    case 0:
        if (m_automaticLogin && m_automaticAttached) {
            return Qt::NoItemFlags;
        }
        return selectableFlag | Qt::ItemIsEnabled;
    case 1:
        return Qt::ItemIsUserCheckable | selectableFlag | (m_automaticLogin ? Qt::NoItemFlags : Qt::ItemIsEnabled);
    case 2:
        return Qt::ItemIsUserCheckable | selectableFlag | (m_automaticAttached ? Qt::NoItemFlags : Qt::ItemIsEnabled);
    default:
        Q_UNREACHABLE();
    }
}

bool DeviceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::CheckStateRole || index.column() == 0) {
        return false;
    }

    const QString &udi = index.data(Qt::UserRole).toString();
    switch (index.column()) {
    case 1:
        m_loginForced[udi] = value.toInt() == Qt::Checked;
        break;
    case 2:
        m_attachedForced[udi] = value.toInt() == Qt::Checked;
        break;
    }

    Q_EMIT dataChanged(index, index);
    return true;
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

    if (!index.parent().isValid()) {
        if (role == Qt::DisplayRole && index.column() == 0) {
            switch (index.row()) {
            case RowAttached:
                return i18n("Attached Devices");
            case RowDetached:
                return i18n("Disconnected Devices");
            }
        }
        return QVariant();
    }

    if (index.parent().row() > RowDetached || index.column() > 2) {
        return QVariant();
    }

    const bool isAttached = index.parent().row() == RowAttached;
    if (role == TypeRole) {
        return isAttached ? Attached : Detached;
    }

    const QString &udi = isAttached ? m_attached.at(index.row()) : m_disconnected.at(index.row());
    if (role == Qt::UserRole) {
        return udi;
    }

    if (index.column() == 0) {
        if (isAttached) {
            Solid::Device dev(udi);
            switch (role) {
            case Qt::DisplayRole:
                return dev.description();
            case Qt::ToolTipRole:
                return i18n("UDI: %1", udi);
            case Qt::DecorationRole:
                return QIcon::fromTheme(dev.icon());
            }
        } else {
            switch (role) {
            case Qt::DisplayRole:
                return m_settings->getDeviceName(udi);
            case Qt::ToolTipRole:
                return i18n("UDI: %1", udi);
            case Qt::DecorationRole:
                return QIcon::fromTheme(m_settings->getDeviceIcon(udi));
            }
        }
    } else if (index.column() == 1) {
        const bool automount = m_loginForced[udi] || m_settings->shouldAutomountDevice(udi, AutomounterSettings::Login);
        switch (role) {
        case Qt::CheckStateRole:
            return m_loginForced[udi] || m_automaticLogin ? Qt::Checked : Qt::Unchecked;
        case Qt::ToolTipRole:
            return automount ? i18n("This device will be automatically mounted at login.") : i18n("This device will not be automatically mounted at login.");
        }
    } else if (index.column() == 2) {
        const bool automount = m_attachedForced[udi] || m_settings->shouldAutomountDevice(udi, AutomounterSettings::Attach);
        switch (role) {
        case Qt::CheckStateRole:
            return m_attachedForced[udi] || m_automaticAttached ? Qt::Checked : Qt::Unchecked;
        case Qt::ToolTipRole:
            return automount ? i18n("This device will be automatically mounted when attached.")
                             : i18n("This device will not be automatically mounted when attached.");
        }
    }
    return QVariant();
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) {
        return 2;
    }
    if (parent.internalId() < 3 || parent.column() > 0) {
        return 0;
    }

    switch (parent.row()) {
    case RowAttached:
        return m_attached.size();
    case RowDetached:
        return m_disconnected.size();
    }

    return 0;
}

int DeviceModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return 3;
}

void DeviceModel::setAutomaticMountOnLogin(bool automaticLogin)
{
    if (m_automaticLogin != automaticLogin) {
        m_automaticLogin = automaticLogin;
        for (int parent = 0; parent < rowCount(); parent++) {
            const auto parentIndex = index(parent, 0);
            Q_EMIT dataChanged(index(0, 1, parentIndex), index(rowCount(parentIndex), 1, parentIndex));
        }
    }
}

void DeviceModel::setAutomaticMountOnPlugin(bool automaticAttached)
{
    if (m_automaticAttached != automaticAttached) {
        m_automaticAttached = automaticAttached;
        for (int parent = 0; parent < rowCount(); parent++) {
            const auto parentIndex = index(parent, 0);
            Q_EMIT dataChanged(index(0, 2, parentIndex), index(rowCount(parentIndex), 2, parentIndex));
        }
    }
}
