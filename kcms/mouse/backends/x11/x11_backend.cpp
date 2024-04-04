/*
    SPDX-FileCopyrightText: 2017 Xuetian Weng <wengxt@gmail.com>
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "x11_backend.h"
#include "x11_libinput_backend.h"

#include "logging.h"

#include <config-X11.h>

#include <libinput-properties.h>

#include <KConfig>
#include <KConfigGroup>
#include <KLocalizedString>
#include <KSharedConfig>
#include <QFile>

#if HAVE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#endif

X11Backend *X11Backend::implementation(QObject *parent)
{
    auto dpy = QX11Info::display();
    Atom testAtom = XInternAtom(dpy, LIBINPUT_PROP_ACCEL, True);

    if (testAtom) {
        qCDebug(KCM_MOUSE) << "Using libinput driver on X11.";
        return new X11LibinputBackend(parent);
    }

    return nullptr;
}

X11Backend::X11Backend(QObject *parent)
    : InputBackend(parent)
{
}

void X11Backend::applyCursorTheme(const QString &theme, int size)
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

void X11Backend::kcmInit()
{
    auto config = KSharedConfig::openConfig("kcminputrc", KConfig::NoGlobals);
    KConfigGroup group = config->group(QStringLiteral("Mouse"));
    const QString theme = group.readEntry("cursorTheme", QStringLiteral("breeze_cursors"));
    const int size = group.readEntry("cursorSize", 24);

    // Note: If you update this code, update kapplymousetheme as well.

    applyCursorTheme(theme, size);
}

#include "moc_x11_backend.cpp"
