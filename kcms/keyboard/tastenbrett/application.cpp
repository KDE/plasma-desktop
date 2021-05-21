/*
    SPDX-FileCopyrightText: 2019 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "application.h"

#include <QKeyEvent>

bool Application::notify(QObject *receiver, QEvent *event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {
        Q_EMIT keyEvent(dynamic_cast<QKeyEvent *>(event));
    }
    return QGuiApplication::notify(receiver, event);
}

Application *Application::instance()
{
    return qobject_cast<Application *>(qGuiApp);
}
