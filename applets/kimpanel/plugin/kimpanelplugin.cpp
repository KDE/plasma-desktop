/*
    SPDX-FileCopyrightText: 2014 Weng Xuetian <wengxt@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "kimpanelplugin.h"
#include "kimpanel.h"
#include "screen.h"

void KimpanelPlugin::registerTypes(const char *uri)
{
    Q_UNUSED(uri);
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.kimpanel"));
    qmlRegisterType<Screen>(uri, 0, 1, "Screen");
    qmlRegisterType<Kimpanel>(uri, 0, 1, "Kimpanel");
}
