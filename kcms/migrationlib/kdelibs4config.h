/*

    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later

*/

#ifndef KDELIBS4CONFIG_H
#define KDELIBS4CONFIG_H

#include <KConfigGroup>
#include <KSharedConfig>
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
