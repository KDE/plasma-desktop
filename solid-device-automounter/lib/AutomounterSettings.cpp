/*
    SPDX-FileCopyrightText: 2009 Trever Fischer <wm161@wm161.net>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "AutomounterSettings.h"

KConfigGroup AutomounterSettings::deviceSettings(const QString &udi)
{
    return config()->group("Devices").group(udi);
}

QStringList AutomounterSettings::knownDevices()
{
    return config()->group("Devices").groupList();
}

bool AutomounterSettings::deviceIsKnown(const QString &udi)
{
    return config()->group("Devices").group(udi).readEntry("EverMounted", false);
}

bool AutomounterSettings::deviceAutomountIsForced(const QString &udi, AutomountType type)
{
    switch (type) {
    case Login:
        return deviceSettings(udi).readEntry("ForceLoginAutomount", false);
    case Attach:
        return deviceSettings(udi).readEntry("ForceAttachAutomount", false);
    }

    return false;
}

bool AutomounterSettings::shouldAutomountDevice(const QString &udi, AutomountType type)
{
    // We auto-mount the device, if:
    // 1.) auto-mounting is forced, or
    // 2.) auto-mounting is enabled
    //     and auto-mounting on login/attach is configured
    //     and the device is known, has been seen mounted last, or is unknown to us

    bool known = deviceIsKnown(udi);
    bool enabled = automountEnabled();
    bool automountUnknown = automountUnknownDevices();
    bool deviceAutomount = deviceAutomountIsForced(udi, type);
    bool lastSeenMounted = deviceSettings(udi).readEntry("LastSeenMounted", false);
    bool typeCondition = false;
    switch (type) {
    case Login:
        typeCondition = automountOnLogin();
        break;
    case Attach:
        typeCondition = automountOnPlugin();
        break;
    }
    bool shouldAutomount = deviceAutomount || (enabled && typeCondition && (known || lastSeenMounted || automountUnknown));

    return shouldAutomount;
}

void AutomounterSettings::setDeviceLastSeenMounted(const QString &udi, bool mounted)
{
    if (mounted) {
        deviceSettings(udi).writeEntry("EverMounted", true);
    }
    deviceSettings(udi).writeEntry("LastSeenMounted", mounted);
}

QString AutomounterSettings::getDeviceName(const QString &udi)
{
    return deviceSettings(udi).readEntry("LastNameSeen");
}

void AutomounterSettings::saveDevice(const Solid::Device &dev)
{
    KConfigGroup settings = deviceSettings(dev.udi());
    settings.writeEntry("LastNameSeen", dev.description());
    settings.writeEntry("Icon", dev.icon());
}

bool AutomounterSettings::getDeviceForcedAutomount(const QString &udi)
{
    return deviceSettings(udi).readEntry("ForceAutomount", false);
}

QString AutomounterSettings::getDeviceIcon(const QString &udi)
{
    return deviceSettings(udi).readEntry("Icon");
}
