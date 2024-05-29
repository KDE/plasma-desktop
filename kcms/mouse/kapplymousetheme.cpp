/*
    SPDX-FileCopyrightText: 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
    SPDX-FileCopyrightText: 2005 Lubos Lunak <l.lunak@kde.org>
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cursortheme.h"

#include <KWindowSystem>

#include <QDebug>
#include <QFile>
#include <QGuiApplication>

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

    // Note: If you update this code, update kcm.cpp as well.

    CursorTheme::applyCursorTheme(theme, size.toInt());

    return ret;
}
