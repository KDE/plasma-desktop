/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2021 Aleix Pol Gonzalez <aleixpol@kde.org>

    Work sponsored by Technische Universität Dresden:
    SPDX-FileCopyrightText: 2022 Klarälvdalens Datakonsult AB a KDAB Group company <info@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "devicesmodel.h"

#include <utility>

#include <QDBusInterface>

#include "inputdevice.h"
#include "logging.h"

DevicesModel::DevicesModel(const QByteArray &kind, QObject *parent)
    : QAbstractListModel(parent)
    , m_kind(kind)
{
    m_deviceManager = new QDBusInterface(QStringLiteral("org.kde.KWin"),
                                         QStringLiteral("/org/kde/KWin/InputDevice"),
                                         QStringLiteral("org.kde.KWin.InputDeviceManager"),
                                         QDBusConnection::sessionBus(),
                                         this);

    resetModel();

    m_deviceManager->connection().connect(QStringLiteral("org.kde.KWin"),
                                          QStringLiteral("/org/kde/KWin/InputDevice"),
                                          QStringLiteral("org.kde.KWin.InputDeviceManager"),
                                          QStringLiteral("deviceAdded"),
                                          this,
                                          SLOT(onDeviceAdded(QString)));
    m_deviceManager->connection().connect(QStringLiteral("org.kde.KWin"),
                                          QStringLiteral("/org/kde/KWin/InputDevice"),
                                          QStringLiteral("org.kde.KWin.InputDeviceManager"),
                                          QStringLiteral("deviceRemoved"),
                                          this,
                                          SLOT(onDeviceRemoved(QString)));
}

void DevicesModel::resetModel()
{
    beginResetModel();
    m_devices.clear();

    QStringList devicesSysNames;
    const QVariant reply = m_deviceManager->property("devicesSysNames");
    if (reply.isValid()) {
        devicesSysNames = reply.toStringList();
    } else {
        qCWarning(LIBKWINDEVICES) << "Error on receiving device list from KWin.";
        return;
    }

    for (const QString &sysname : std::as_const(devicesSysNames)) {
        addDevice(sysname, false);
    }
    endResetModel();
}

QVariant DevicesModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index, CheckIndexOption::IndexIsValid) || index.column() != 0) {
        return {};
    }

    switch (role) {
    case Qt::DisplayRole:
        return m_devices.at(index.row())->name();
    case Qt::UserRole:
        return QVariant::fromValue<QObject *>(m_devices.at(index.row()).get());
    }
    return {};
}

int DevicesModel::rowCount(const QModelIndex &parent) const
{
    return parent.isValid() ? 0 : m_devices.size();
}

void DevicesModel::onDeviceAdded(const QString &sysName)
{
    if (std::any_of(m_devices.cbegin(), m_devices.cend(), [sysName](auto &t) {
            return t->sysName() == sysName;
        })) {
        return;
    }

    addDevice(sysName, true);
}

void DevicesModel::addDevice(const QString &sysName, bool tellModel)
{
    QDBusInterface deviceIface(QStringLiteral("org.kde.KWin"),
                               QStringLiteral("/org/kde/KWin/InputDevice/") + sysName,
                               QStringLiteral("org.kde.KWin.InputDevice"),
                               QDBusConnection::sessionBus(),
                               this);
    QVariant reply = deviceIface.property(m_kind);
    if (reply.isValid() && reply.toBool()) {
        auto dev = std::make_unique<InputDevice>(sysName, this);
        connect(dev.get(), &InputDevice::needsSaveChanged, this, &DevicesModel::needsSaveChanged);

        if (tellModel) {
            beginInsertRows({}, m_devices.size(), m_devices.size());
        }
        qCDebug(LIBKWINDEVICES).nospace() << "Device connected: " << dev->name() << " (" << dev->sysName() << ")";
        m_devices.push_back(std::move(dev));
        if (tellModel) {
            endInsertRows();
        }
    }
}

void DevicesModel::onDeviceRemoved(const QString &sysName)
{
    auto it = std::find_if(m_devices.cbegin(), m_devices.cend(), [sysName](auto &t) {
        return t->sysName() == sysName;
    });
    if (it == m_devices.cend()) {
        return;
    }

    auto dev = static_cast<InputDevice *>(it->get());
    qCDebug(LIBKWINDEVICES).nospace() << "Device disconnected: " << dev->name() << " (" << dev->sysName() << ")";

    int index = std::distance(m_devices.cbegin(), it);

    beginRemoveRows({}, index, index);
    m_devices.erase(it);
    endRemoveRows();
}

InputDevice *DevicesModel::deviceAt(int row) const
{
    if (row == -1) {
        return nullptr;
    }

    return m_devices.at(row).get();
}

QHash<int, QByteArray> DevicesModel::roleNames() const
{
    return {
        {Qt::DisplayRole, "display"},
    };
}

void DevicesModel::defaults()
{
    for (auto &device : m_devices) {
        device->defaults();
    }
}

void DevicesModel::load()
{
    for (auto &device : m_devices) {
        device->load();
    }
}

void DevicesModel::save()
{
    for (auto &device : m_devices) {
        device->save();
    }
}

bool DevicesModel::isDefaults() const
{
    return std::all_of(m_devices.cbegin(), m_devices.cend(), [](auto &dev) {
        return dev->isDefaults();
    });
}

bool DevicesModel::isSaveNeeded() const
{
    return std::any_of(m_devices.cbegin(), m_devices.cend(), [](auto &dev) {
        return dev->isSaveNeeded();
    });
}
