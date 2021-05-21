/*
    SPDX-FileCopyrightText: 2014 Ivan Cukic <ivan.cukic(at)kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "activityswitcherextensionplugin.h"

#include <QQmlEngine>

#include "switcherbackend.h"

ActivitySwitcherExtensionPlugin::ActivitySwitcherExtensionPlugin(QObject *parent)
    : QQmlExtensionPlugin(parent)
{
}

void ActivitySwitcherExtensionPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.activityswitcher"));

    qmlRegisterSingletonType<SwitcherBackend>(uri, 1, 0, "Backend", SwitcherBackend::instance);
}
