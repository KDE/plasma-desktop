/*
    SPDX-FileCopyrightText: 2009 Trever Fischer <wm161@wm161.net>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2021 Ismael Asensio <isma.af@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AutomounterSettings.h"

void AutomounterSettings::usrRead()
{
    qDeleteAll(m_devices);
    m_devices.clear();
    for (const QString &udi : knownDevices()) {
        m_devices[udi] = new DeviceSettings(sharedConfig(), udi, this);
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

bool AutomounterSettings::hasDeviceInfo(const QString &udi) const
{
    return m_devices.contains(udi);
}

DeviceSettings *AutomounterSettings::deviceSettings(const QString &udi) const
{
    return m_devices.value(udi);
}

bool AutomounterSettings::shouldAutomountDevice(const QString &udi, AutomountType type) const
{
    // We auto-mount the device, if:
    // 1.) auto-mounting is forced for the specific device, or
    // 2.) auto-mounting is enabled
    //     and auto-mounting on login/attach is configured
    //     and the device is known, has been seen mounted last, or unknown devices are allowed

    switch (type) {
    case Login:
        if (hasDeviceInfo(udi) && deviceSettings(udi)->mountOnLogin()) {
            return true;
        }
        if (!automountOnLogin()) {
            return false;
        }
        break;
    case Attach:
        if (hasDeviceInfo(udi) && deviceSettings(udi)->mountOnAttach()) {
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

    return hasDeviceInfo(udi) && (deviceSettings(udi)->isKnown() || deviceSettings(udi)->lastSeenMounted());
}

void AutomounterSettings::setDeviceLastSeenMounted(const QString &udi, bool mounted)
{
    if (!m_devices.contains(udi)) {
        m_devices[udi] = new DeviceSettings(sharedConfig(), udi, this);
    }
    if (mounted) {
        deviceSettings(udi)->setIsKnown(true);
    }
    deviceSettings(udi)->setLastSeenMounted(mounted);
}

void AutomounterSettings::setDeviceInfo(const Solid::Device &dev)
{
    const QString udi = dev.udi();
    if (!m_devices.contains(udi)) {
        m_devices[udi] = new DeviceSettings(sharedConfig(), udi, this);
    }
    auto settings = deviceSettings(udi);
    settings->setName(dev.description());
    settings->setIcon(dev.icon());
}

void AutomounterSettings::removeDeviceGroup(const QString &udi)
{
    if (config()->group(QStringLiteral("Devices")).hasGroup(udi)) {
        config()->group(QStringLiteral("Devices")).group(udi).deleteGroup();
    }
}
