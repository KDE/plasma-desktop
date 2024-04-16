/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cursortheme.h"

#include <config-X11.h>

#include <QFile>
#include <QGuiApplication>

#include <KConfigGroup>
#include <KSharedConfig>

#if HAVE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#endif

void CursorTheme::applyCursorTheme(const QString &theme, int size)
{
#if HAVE_XCURSOR
    QNativeInterface::QX11Application *x11App = qGuiApp->nativeInterface<QNativeInterface::QX11Application>();
    if (!x11App) {
        return;
    }

    // Apply the KDE cursor theme to ourselves
    if (!theme.isEmpty()) {
        XcursorSetTheme(x11App->display(), QFile::encodeName(theme));
    }

    if (size >= 0) {
        XcursorSetDefaultSize(x11App->display(), size);
    }

    // Load the default cursor from the theme and apply it to the root window.
    Cursor handle = XcursorLibraryLoadCursor(x11App->display(), "left_ptr");
    XDefineCursor(x11App->display(), DefaultRootWindow(x11App->display()), handle);
    XFreeCursor(x11App->display(), handle); // Don't leak the cursor
    XFlush(x11App->display());
#endif
}
