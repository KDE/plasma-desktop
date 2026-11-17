/*
    SPDX-FileCopyrightText: 2009 Trever Fischer <wm161@wm161.net>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2021 Ismael Asensio <isma.af@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AutomounterSettings.h"

#include <Solid/StorageVolume>

QString AutomounterSettings::deviceKey(const Solid::Device &dev)
{
    if (const auto volume = dev.as<Solid::StorageVolume>()) {
        if (const QString uuid = volume->uuid(); !uuid.isEmpty()) {
            return uuid;
        }
    }
    return dev.udi();
}

void AutomounterSettings::migrateUdiGroups()
{
    // Settings used to be stored per UDI, which is not stable. Move the groups of all currently
    // attached devices over to their UUID. Groups of devices which are not attached can't be
    // mapped and are left alone.
    auto devices = config()->group(QStringLiteral("Devices"));
    const auto groups = devices.groupList();
    if (groups.isEmpty()) {
        return;
    }

    const auto volumes = Solid::Device::listFromType(Solid::DeviceInterface::StorageVolume);
    for (const Solid::Device &volume : volumes) {
        const auto udi = volume.udi();
        const auto key = deviceKey(volume);
        if (key == udi || !groups.contains(udi)) {
            continue;
        }
        if (!devices.hasGroup(key)) {
            auto target = devices.group(key);
            devices.group(udi).copyTo(&target);
        }
        devices.deleteGroup(udi);
    }
}

void AutomounterSettings::usrRead()
{
    migrateUdiGroups();
    qDeleteAll(m_devices);
    m_devices.clear();
    for (const QString &key : knownDevices()) {
        m_devices[key] = new DeviceSettings(sharedConfig(), key, this);
    }
}

bool AutomounterSettings::usrSave()
{
    bool saveOk = true;
    for (const auto settings : m_devices.values()) {
        saveOk &= settings->save();
    }
    return saveOk;
}

bool AutomounterSettings::usrIsSaveNeeded()
{
    return isSaveNeeded() || std::any_of(m_devices.cbegin(), m_devices.cend(), [](const DeviceSettings *device) {
               return device->isSaveNeeded();
           });
}

QStringList AutomounterSettings::knownDevices() const
{
    return config()->group(QStringLiteral("Devices")).groupList();
}

bool AutomounterSettings::hasDeviceInfo(const QString &key) const
{
    return m_devices.contains(key);
}

DeviceSettings *AutomounterSettings::deviceSettings(const QString &key) const
{
    return m_devices.value(key);
}

bool AutomounterSettings::shouldAutomountDevice(const QString &key, AutomountType type) const
{
    // We auto-mount the device, if:
    // 1.) auto-mounting is forced for the specific device, or
    // 2.) auto-mounting is enabled
    //     and auto-mounting on login/attach is configured
    //     and the device is known, has been seen mounted last, or unknown devices are allowed

    switch (type) {
    case Login:
        if (hasDeviceInfo(key) && deviceSettings(key)->mountOnLogin()) {
            return true;
        }
        if (!automountOnLogin()) {
            return false;
        }
        break;
    case Attach:
        if (hasDeviceInfo(key) && deviceSettings(key)->mountOnAttach()) {
            return true;
        }
        if (!automountOnPlugin()) {
            return false;
        }
        break;
    }

    if (automountUnknownDevices()) {
        return true;
    }

    return hasDeviceInfo(key) && (deviceSettings(key)->isKnown() || deviceSettings(key)->lastSeenMounted());
}

void AutomounterSettings::setDeviceLastSeenMounted(const QString &key, bool mounted)
{
    if (!m_devices.contains(key)) {
        m_devices[key] = new DeviceSettings(sharedConfig(), key, this);
    }
    if (mounted) {
        deviceSettings(key)->setIsKnown(true);
    }
    deviceSettings(key)->setLastSeenMounted(mounted);
}

void AutomounterSettings::setDeviceInfo(const Solid::Device &dev)
{
    const QString key = deviceKey(dev);
    if (!m_devices.contains(key)) {
        m_devices[key] = new DeviceSettings(sharedConfig(), key, this);
    }
    auto settings = deviceSettings(key);
    settings->setName(dev.description());
    settings->setIcon(dev.icon());
}

void AutomounterSettings::removeDeviceGroup(const QString &key)
{
    if (config()->group(QStringLiteral("Devices")).hasGroup(key)) {
        config()->group(QStringLiteral("Devices")).group(key).deleteGroup();
    }
}
