/*
    SPDX-FileCopyrightText: 2012-2016 Ivan Cukic <ivan.cukic@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "plugin.h"

#include <QQmlEngine>

#include "activitysettings.h"

#include <QDebug>

ActivitiesSettingsPlugin::ActivitiesSettingsPlugin(QObject *parent)
    : QQmlExtensionPlugin(parent)
{
}

static QJSValue settingsSingleton(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)

    auto result = new ActivitySettings();
    return scriptEngine->newQObject(result);
}

void ActivitiesSettingsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.activities.settings"));

    qmlRegisterSingletonType("org.kde.activities.settings", 0, 1, "ActivitySettings", settingsSingleton);
}
