/***************************************************************************
*   Copyright (C) 2009 by Trever Fischer <wm161@wm161.net>                *
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

#ifndef AUTOMOUNTERSETTINGS_H
#define AUTOMOUNTERSETTINGS_H

#include "AutomounterSettingsBase.h"

#include <KConfigGroup>

#include <Solid/Device>

class AutomounterSettings : public AutomounterSettingsBase {
    public:
        using AutomounterSettingsBase::AutomounterSettingsBase;
        enum AutomountType {
            Login,
            Attach
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
