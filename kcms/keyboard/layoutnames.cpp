/*
    SPDX-FileCopyrightText: 2021 Andrey Butirsky <butirsky@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "layoutnames.h"
#include <QDBusMetaType>

void LayoutNames::registerMetaType()
{
    qDBusRegisterMetaType<LayoutNames>();
    qDBusRegisterMetaType<QList<LayoutNames>>();
}

QDBusArgument &operator<<(QDBusArgument &argument, const LayoutNames &layoutNames)
{
    argument.beginStructure();
    argument << layoutNames.shortName << layoutNames.displayName << layoutNames.longName;
    argument.endStructure();
    return argument;
}

const QDBusArgument &operator>>(const QDBusArgument &argument, LayoutNames &layoutNames)
{
    argument.beginStructure();
    argument >> layoutNames.shortName >> layoutNames.displayName >> layoutNames.longName;
    argument.endStructure();
    return argument;
}
