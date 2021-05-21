/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "trashplugin.h"
#include "dirmodel.h"
#include "trash.h"

#include <QQmlEngine>

static QObject *trash_singletonProvider(QQmlEngine *engine, QJSEngine *scriptEngine)
{
    Q_UNUSED(engine)
    Q_UNUSED(scriptEngine)
    return new Trash();
}

void TrashPrivatePlugin::registerTypes(const char *uri)
{
    Q_ASSERT(QLatin1String(uri) == QLatin1String("org.kde.plasma.private.trash"));
    qmlRegisterType<DirModel>(uri, 1, 0, "DirModel");
    qmlRegisterSingletonType<Trash>(uri, 1, 0, "Trash", trash_singletonProvider);
}
