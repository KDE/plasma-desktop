/*
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2005 Lubos Lunak <l.lunak@kde.org>
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "backends/x11/x11_backend.h"

#include <KWindowSystem/kwindowsystem.h>

#include <QFile>
#include <QGuiApplication>

#include <QDebug>

int main(int argc, char *argv[])
{
    int ret = 0;
    QGuiApplication::setDesktopSettingsAware(false);
    QGuiApplication app(argc, argv);
    if (argc != 3)
        return 1;
    QString theme = QFile::decodeName(argv[1]);
    QString size = QFile::decodeName(argv[2]);

    if (!KWindowSystem::isPlatformX11()) {
        qDebug() << "X11 backend not detected. Exit.";
        return 2;
    }

    X11Backend *backend = X11Backend::implementation();

    if (!backend->isValid()) {
        return 2;
    }

    // Note: If you update this code, update main.cpp as well.

    // use a default value for theme only if it's not configured at all, not even in X resources
    if (theme.isEmpty() && backend->currentCursorTheme().isEmpty()) {
        theme = "breeze_cursors";
        ret = 10; // means to switch to default
    }

    backend->applyCursorTheme(theme, size.toInt());

    delete backend;
    return ret;
}
