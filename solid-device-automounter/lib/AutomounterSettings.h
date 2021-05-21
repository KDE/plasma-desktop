/*
    SPDX-FileCopyrightText: 2009 Trever Fischer <wm161@wm161.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef AUTOMOUNTERSETTINGS_H
#define AUTOMOUNTERSETTINGS_H

#include "AutomounterSettingsBase.h"

#include <KConfigGroup>

#include <Solid/Device>

class AutomounterSettings : public AutomounterSettingsBase
{
public:
    using AutomounterSettingsBase::AutomounterSettingsBase;
    enum AutomountType {
        Login,
        Attach,
    };
    KConfigGroup deviceSettings(const QString &udi);
    QStringList knownDevices();
    bool deviceIsKnown(const QString &udi);
    bool shouldAutomountDevice(const QString &udi, AutomountType type);
    void setDeviceLastSeenMounted(const QString &udi, bool mounted);
    bool deviceAutomountIsForced(const QString &udi, AutomountType type);
    QString getDeviceName(const QString &udi);
    bool getDeviceForcedAutomount(const QString &udi);
    QString getDeviceIcon(const QString &udi);
    void saveDevice(const Solid::Device &dev);
};

#endif
