/*
    This file is part of KIMToy, an input method frontend for KDE
    SPDX-FileCopyrightText: 2011 Ni Hui <shuizhuyuanluo@126.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "app.h"
#include "panel.h"
#include <QSessionManager>
#include <ibus.h>
#include <locale.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    ibus_init();
    // we don't need im in this.
    qputenv("QT_IM_MODULE", "compose");
    App app(argc, argv);

    QGuiApplication::setFallbackSessionManagementEnabled(false);

    auto disableSessionManagement = [](QSessionManager &sm) {
        sm.setRestartHint(QSessionManager::RestartNever);
    };

    QObject::connect(&app, &QGuiApplication::commitDataRequest, disableSessionManagement);
    QObject::connect(&app, &QGuiApplication::saveStateRequest, disableSessionManagement);

    return app.exec();
}
