/*
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "desktopplugin.h"
#include "infonotification.h"

void DesktopPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.private.desktopcontainment.desktop"));
    qmlRegisterType<InfoNotification>(uri, 0, 1, "InfoNotification");
}
