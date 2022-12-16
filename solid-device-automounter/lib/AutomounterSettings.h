/*
    SPDX-FileCopyrightText: 2009 Trever Fischer <wm161@wm161.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "AutomounterSettingsBase.h"
#include "DeviceSettings.h"

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

    bool hasDeviceInfo(const QString &udi) const;
    DeviceSettings *deviceSettings(const QString &udi) const;
    QStringList knownDevices() const;
    bool shouldAutomountDevice(const QString &udi, AutomountType type) const;

    void setDeviceLastSeenMounted(const QString &udi, bool mounted);
    void setDeviceInfo(const Solid::Device &dev);
    void removeDeviceGroup(const QString &udi);

    bool usrIsSaveNeeded();

private:
    void usrRead() override;
    bool usrSave() override;

private:
    QHash<QString, DeviceSettings *> m_devices;
};
