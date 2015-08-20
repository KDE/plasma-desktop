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
        enum AutomountType {
            Login,
            Attach
        };
        static KConfigGroup deviceSettings(const QString &udi);
        static QStringList knownDevices();
        static bool deviceIsKnown(const QString &udi);
        static bool shouldAutomountDevice(const QString &udi, AutomountType type);
        static void setDeviceLastSeenMounted(const QString &udi, bool mounted);
        static bool deviceAutomountIsForced(const QString &udi, AutomountType type);
        static QString getDeviceName(const QString &udi);
        static bool getDeviceForcedAutomount(const QString &udi);
        static QString getDeviceIcon(const QString &udi);
        static void saveDevice(const Solid::Device &dev);
};

#endif
