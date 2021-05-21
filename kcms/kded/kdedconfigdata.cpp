/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2021 Cyril Rossi <cyril.rossi@enioka.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "kdedconfigdata.h"

#include <KConfig>
#include <KConfigGroup>

KDEDConfigData::KDEDConfigData(QObject *parent, const QVariantList &args)
    : KCModuleData(parent, args)
{
}

bool KDEDConfigData::isDefaults() const
{
    KConfig kdedrc(QStringLiteral("kded5rc"), KConfig::NoGlobals);
    const QStringList groupList = kdedrc.groupList();
    for (auto &groupName : groupList) {
        if (groupName.startsWith(QStringLiteral("Module-"))) {
            KConfigGroup cg(&kdedrc, groupName);
            if (!cg.readEntry(QStringLiteral("autoload"), true)) {
                return false;
            }
        }
    }

    return true;
}
