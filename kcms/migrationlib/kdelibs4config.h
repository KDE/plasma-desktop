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

#include <KSharedConfig>
#include <KConfigGroup>
#include <Kdelibs4Migration>

class Kdelibs4SharedConfig
{
public:
    static KSharedConfig::Ptr openConfig(const QString &fileName, KConfig::OpenFlags mode=KConfig::FullConfig)
    {
        Kdelibs4Migration migration;
        QString configDirPath = migration.saveLocation("config");
        return KSharedConfig::openConfig(configDirPath + '/' + fileName);
    }

    static void syncConfigGroup(KConfigGroup *sourceGroup, const QString &fileName)
    {
        KSharedConfigPtr kde4Config = openConfig(fileName);
        KConfigGroup kde4ConfigGroup = kde4Config->group(sourceGroup->name());
        sourceGroup->copyTo(&kde4ConfigGroup);
        kde4ConfigGroup.sync();
    }

};

