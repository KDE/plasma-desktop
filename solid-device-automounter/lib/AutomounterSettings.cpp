/***************************************************************************
*   Copyright (C) 2009 by Trever Fischer <wm161@wm161.net>                *
*   Copyright (C) 2015 by Kai Uwe Broulik <kde@privat.broulik.de>         *
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
    switch(type) {
    case Login:
        typeCondition = automountOnLogin();
        break;
    case Attach:
        typeCondition = automountOnPlugin();
        break;
    }
    bool shouldAutomount = deviceAutomount || (enabled && typeCondition
                                   && (known || lastSeenMounted || automountUnknown));

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
