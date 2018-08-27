/*
 *
 *  Copyright (C) 2014 David Edmundson <davidedmundson@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef KDELIBS4CONFIG_H
#define KDELIBS4CONFIG_H

#include <KSharedConfig>
#include <KConfigGroup>
#include <Kdelibs4Migration>

class Kdelibs4SharedConfig
{
public:
    static void syncConfigGroup(const QLatin1String &sourceGroup, const QString &fileName)
    {
        Kdelibs4Migration migration;
        QString configDirPath = migration.saveLocation("config");
        KSharedConfigPtr kde4Config = KSharedConfig::openConfig(configDirPath + '/' + fileName);
        KSharedConfigPtr simpleConfig = KSharedConfig::openConfig(fileName, KConfig::SimpleConfig);
        KConfigGroup simpleConfigGroup(simpleConfig, sourceGroup);
        KConfigGroup kde4ConfigGroup = kde4Config->group(sourceGroup);
        simpleConfigGroup.copyTo(&kde4ConfigGroup);
        kde4ConfigGroup.sync();
    }

};

#endif
