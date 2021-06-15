/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "taskmanagerplugin.h"
#include "backend.h"
#include "draghelper.h"

#include "smartlaunchers/smartlauncheritem.h"

#include <QAction>

void TaskManagerPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.taskmanager"));
    qmlRegisterType<Backend>(uri, 0, 1, "Backend");
    qmlRegisterType<DragHelper>(uri, 0, 1, "DragHelper");

    qmlRegisterType<SmartLauncher::Item>(uri, 0, 1, "SmartLauncherItem");
}
