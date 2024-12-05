/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2013 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "taskmanagerplugin.h"
#include "backend.h"

#include "smartlaunchers/smartlauncheritem.h"

#include <QAction>

void TaskManagerPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<Backend>(uri, 0, 1, "Backend");

    qmlRegisterType<SmartLauncher::Item>(uri, 0, 1, "SmartLauncherItem");
}

#include "moc_taskmanagerplugin.cpp"
