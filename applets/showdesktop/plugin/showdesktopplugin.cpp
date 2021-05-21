/*
    SPDX-FileCopyrightText: 2014 Ashish Madeti <ashishmadeti@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "showdesktopplugin.h"
#include "showdesktop.h"

// Qt

void ShowDesktopPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.showdesktop"));
    qmlRegisterType<ShowDesktop>(uri, 0, 1, "ShowDesktop");
}
