/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "cursortheme.h"

#include <config-X11.h>

#include <QFile>

#include <KConfigGroup>
#include <KSharedConfig>

#include <private/qtx11extras_p.h>

#if HAVE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#endif

void CursorTheme::applyCursorTheme(const QString &theme, int size)
{
#if HAVE_XCURSOR
    // Apply the KDE cursor theme to ourselves
    if (!theme.isEmpty()) {
        XcursorSetTheme(QX11Info::display(), QFile::encodeName(theme));
    }

    if (size >= 0) {
        XcursorSetDefaultSize(QX11Info::display(), size);
    }

    // Load the default cursor from the theme and apply it to the root window.
    Cursor handle = XcursorLibraryLoadCursor(QX11Info::display(), "left_ptr");
    XDefineCursor(QX11Info::display(), DefaultRootWindow(QX11Info::display()), handle);
    XFreeCursor(QX11Info::display(), handle); // Don't leak the cursor
    XFlush(QX11Info::display());
#endif
}
