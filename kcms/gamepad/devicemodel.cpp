/*
    SPDX-FileCopyrightText: 2023 Joshua Goins <josh@redstrate.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "devicemodel.h"

#include <QDebug>
#include <Solid/Block>
#include <Solid/DeviceNotifier>
#include <Solid/GenericInterface>
#include <Solid/Predicate>
#include <libudev.h>

#include "joydevice.h"

DeviceModel::DeviceModel()
{
    const auto solidDevices = Solid::Device::listFromType(Solid::DeviceInterface::Block);
    for (const Solid::Device &device : solidDevices) {
        addDevice(device);
    }
}

int DeviceModel::rowCount(const QModelIndex &parent) const
{
    return m_devices.count();
}

QVariant DeviceModel::data(const QModelIndex &index, int role) const
{
    if (!checkIndex(index)) {
        return {};
    }

    switch (role) {
    case CustomRoles::NameRole:
        return m_devices[index.row()]->getName();
    case CustomRoles::DeviceRole:
        return QVariant::fromValue(m_devices[index.row()]);
    default:
        return {};
    }
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    return {{CustomRoles::NameRole, "name"}, {CustomRoles::DeviceRole, "device"}};
}

void DeviceModel::addDevice(const Solid::Device &device)
{
    auto generic = device.as<Solid::GenericInterface>();
    if (generic && !generic->property("ID_INPUT_JOYSTICK").toBool()) {
        return;
    }

    if (device.udi().contains("js")) {
        return;
    }

    qDebug() << "Adding" << device.udi();

    m_devices.push_back(new JoyDevice(device, this));
}
