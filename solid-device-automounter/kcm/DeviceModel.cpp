/*
    SPDX-FileCopyrightText: 2009-2010 Trever Fischer <tdfischer@fedoraproject.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2021 Ismael Asensio <isma.af@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "DeviceModel.h"

#include <QIcon>
#include <QTimer>

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
}

QVariant DeviceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
        case 0:
            return i18n("Automount Device");
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
    const Solid::Device device(udi);
    auto volume = device.as<Solid::StorageVolume>();

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

    // The kded module might not have updated the settings yet with the new device.
    // Let's try again  for a limited number of times
    static int loadTryouts = 0;
    if (!m_settings->hasDeviceInfo(udi)) {
        if (loadTryouts < 5) {
            loadTryouts++;
            QTimer::singleShot(100, this, [this, udi]() {
                addNewDevice(udi);
            });
        }
        return;
    }
    loadTryouts = 0;

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
    m_attached.clear();
    m_disconnected.clear();

    const auto knownDevices = m_settings->knownDevices();
    for (const QString &dev : knownDevices) {
        addNewDevice(dev);
    }
    endResetModel();
}

QModelIndex DeviceModel::index(int row, int column, const QModelIndex &parent) const
{
    if (column < 0 || column >= columnCount()) {
        return QModelIndex();
    }
    if (parent.isValid()) {
        if (parent.column() > 0 || parent.row() == RowAll) {
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
        if (index.row() == RowAll) {
            return Qt::ItemIsEnabled | (index.column() > 0 ? Qt::ItemIsUserCheckable : Qt::NoItemFlags);
        }
        return (m_settings->automountOnLogin() && m_settings->automountOnPlugin()) ? Qt::NoItemFlags : Qt::ItemIsEnabled;
    }

    // Select only detached devices to be removed
    Qt::ItemFlag selectableFlag = index.parent().row() == RowDetached ? Qt::ItemIsSelectable : Qt::NoItemFlags;

    switch (index.column()) {
    case 0:
        if (m_settings->automountOnLogin() && m_settings->automountOnPlugin()) {
            return Qt::NoItemFlags;
        }
        return selectableFlag | Qt::ItemIsEnabled;
    case 1:
        return Qt::ItemIsUserCheckable | selectableFlag | (m_settings->automountOnLogin() ? Qt::NoItemFlags : Qt::ItemIsEnabled);
    case 2:
        return Qt::ItemIsUserCheckable | selectableFlag | (m_settings->automountOnPlugin() ? Qt::NoItemFlags : Qt::ItemIsEnabled);
    default:
        Q_UNREACHABLE();
    }
}

bool DeviceModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::CheckStateRole || index.column() == 0) {
        return false;
    }

    if (!index.parent().isValid() && index.row() == RowAll) {
        switch (index.column()) {
        case 1:
            setAutomaticMountOnLogin(value.toInt() == Qt::Checked);
            break;
        case 2:
            setAutomaticMountOnPlugin(value.toInt() == Qt::Checked);
            break;
        }
        Q_EMIT dataChanged(index, index);
        return true;
    }

    const QString &udi = index.data(Qt::UserRole).toString();
    Q_ASSERT(m_settings->hasDeviceInfo(udi));

    switch (index.column()) {
    case 1:
        m_settings->deviceSettings(udi)->setMountOnLogin(value.toInt() == Qt::Checked);
        break;
    case 2:
        m_settings->deviceSettings(udi)->setMountOnAttach(value.toInt() == Qt::Checked);
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
            case RowAll:
                return m_settings->automountUnknownDevices() ? i18n("All Devices") : i18n("All Known Devices");
            case RowAttached:
                return i18n("Attached Devices");
            case RowDetached:
                return i18n("Disconnected Devices");
            }
        }
        if (role == Qt::CheckStateRole && index.row() == RowAll) {
            if (index.column() == 1) {
                return m_settings->automountOnLogin() ? Qt::Checked : Qt::Unchecked;
            } else if (index.column() == 2) {
                return m_settings->automountOnPlugin() ? Qt::Checked : Qt::Unchecked;
            }
        }
        return QVariant();
    }

    if (index.parent().row() > RowDetached || index.column() >= columnCount() || index.row() >= rowCount(index.parent())) {
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

    Q_ASSERT(m_settings->hasDeviceInfo(udi));

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
                return m_settings->deviceSettings(udi)->name();
            case Qt::ToolTipRole:
                return i18n("UDI: %1", udi);
            case Qt::DecorationRole:
                return QIcon::fromTheme(m_settings->deviceSettings(udi)->icon());
            }
        }
    } else if (index.column() == 1) {
        const bool automount = m_settings->shouldAutomountDevice(udi, AutomounterSettings::Login);
        switch (role) {
        case Qt::CheckStateRole:
            return automount ? Qt::Checked : Qt::Unchecked;
        case Qt::ToolTipRole:
            return automount ? i18n("This device will be automatically mounted at login.") : i18n("This device will not be automatically mounted at login.");
        }
    } else if (index.column() == 2) {
        const bool automount = m_settings->shouldAutomountDevice(udi, AutomounterSettings::Attach);
        switch (role) {
        case Qt::CheckStateRole:
            return automount ? Qt::Checked : Qt::Unchecked;
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
        return 3;
    }
    if (parent.internalId() < 3 || parent.column() > 0) {
        return 0;
    }

    switch (parent.row()) {
    case RowAll:
        return 0;
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
    if (m_settings->automountOnLogin() == automaticLogin) {
        return;
    }

    m_settings->setAutomountOnLogin(automaticLogin);
    updateCheckedColumns(1);
}

void DeviceModel::setAutomaticMountOnPlugin(bool automaticAttached)
{
    if (m_settings->automountOnPlugin() == automaticAttached) {
        return;
    }

    m_settings->setAutomountOnPlugin(automaticAttached);
    updateCheckedColumns(2);
}

void DeviceModel::setAutomaticUnknown(bool automaticUnknown)
{
    if (m_settings->automountUnknownDevices() == automaticUnknown) {
        return;
    }

    m_settings->setAutomountUnknownDevices(automaticUnknown);
    Q_EMIT dataChanged(index(0, 0), index(0, 0), {Qt::DisplayRole});
    updateCheckedColumns();
}

void DeviceModel::updateCheckedColumns(int column)
{
    for (int parent = RowAttached; parent < rowCount(); parent++) {
        const auto parentIndex = index(parent, 0);
        Q_EMIT dataChanged(index(0, (column > 0 ? column : 1), parentIndex),
                           index(rowCount(parentIndex), column > 0 ? column : 2, parentIndex),
                           {Qt::CheckStateRole, Qt::ToolTipRole});
    }
}

#include "moc_DeviceModel.cpp"
