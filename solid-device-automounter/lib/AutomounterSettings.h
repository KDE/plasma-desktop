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

    /**
     * The key under which the settings of a device are stored.
     * This is the UUID of the volume, which (unlike the UDI) is stable across replugging.
     * Falls back to the UDI for volumes without a UUID.
     */
    static QString deviceKey(const Solid::Device &dev);

    bool hasDeviceInfo(const QString &key) const;
    DeviceSettings *deviceSettings(const QString &key) const;
    QStringList knownDevices() const;
    bool shouldAutomountDevice(const QString &key, AutomountType type) const;

    void setDeviceLastSeenMounted(const QString &key, bool mounted);
    void setDeviceInfo(const Solid::Device &dev);
    void removeDeviceGroup(const QString &key);

    bool usrIsSaveNeeded();

private:
    void migrateUdiGroups();
    void usrRead() override;
    bool usrSave() override;

private:
    QHash<QString, DeviceSettings *> m_devices;
};
