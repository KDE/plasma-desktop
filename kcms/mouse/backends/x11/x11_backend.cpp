/*
 * Copyright 2017 Xuetian Weng <wengxt@gmail.com>
 * Copyright 2018 Roman Gilg <subdiff@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "x11_backend.h"
#include "x11_evdev_backend.h"
#include "x11_libinput_backend.h"

#include "logging.h"

#include <config-X11.h>

#include <libinput-properties.h>

#include <QFile>
#include <QX11Info>
#include <KLocalizedString>
#include <KConfig>
#include <KSharedConfig>
#include <KConfigGroup>

#include <updatelaunchenvjob.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/extensions/XInput2.h>
#include <X11/extensions/XI.h>
#include <X11/Xutil.h>
#ifdef HAVE_XCURSOR
#include <X11/Xcursor/Xcursor.h>
#include <X11/extensions/XInput.h>
#endif

X11Backend *X11Backend::implementation(QObject *parent)
{
    auto dpy = QX11Info::display();
    Atom testAtom = XInternAtom(dpy, LIBINPUT_PROP_ACCEL, True);

    //There are multiple possible drivers
    if (testAtom) {
        qCDebug(KCM_MOUSE) << "Using libinput driver on X11.";
        return new X11LibinputBackend(parent);
    }
    else {
        qCDebug(KCM_MOUSE) << "Using evdev driver on X11.";
        return new X11EvdevBackend(parent);
    }
}

X11Backend::X11Backend(QObject* parent)
    : InputBackend(parent)
{
    m_platformX11 = QX11Info::isPlatformX11();
    if (m_platformX11) {
        m_dpy = QX11Info::display();
    } else {
        // TODO: remove this - not needed anymore with Wayland backend!
        // let's hope we have a compatibility system like Xwayland ready
        m_dpy = XOpenDisplay(nullptr);
    }
}

X11Backend::~X11Backend()
{
    if (!m_platformX11 && m_dpy) {
        XCloseDisplay(m_dpy);
    }
}

QString X11Backend::currentCursorTheme()
{
    if (!m_dpy) {
        return QString();
    }

    QByteArray name = XGetDefault(m_dpy, "Xcursor", "theme");
#ifdef HAVE_XCURSOR
    if (name.isEmpty()) {
        name = QByteArray(XcursorGetTheme(m_dpy));
    }
#endif
    return QFile::decodeName(name);
}

void X11Backend::applyCursorTheme(const QString& theme, int size)
{
#ifdef HAVE_XCURSOR

    // Apply the KDE cursor theme to ourselves
    if (m_dpy) {
        return;
    }
    if (!theme.isEmpty()) {
        XcursorSetTheme(m_dpy, QFile::encodeName(theme));
    }

    if (size >= 0) {
        XcursorSetDefaultSize(m_dpy, size);
    }

    // Load the default cursor from the theme and apply it to the root window.
    Cursor handle = XcursorLibraryLoadCursor(m_dpy, "left_ptr");
    XDefineCursor(m_dpy, DefaultRootWindow(m_dpy), handle);
    XFreeCursor(m_dpy, handle); // Don't leak the cursor
    XFlush(m_dpy);
#endif
}

void X11Backend::kcmInit()
{
    auto config = KSharedConfig::openConfig("kcminputrc", KConfig::NoGlobals);
    KConfigGroup group = config->group("Mouse");
    QString theme = group.readEntry("cursorTheme", QString());
    const int size = group.readEntry("cursorSize", 24);

    // Note: If you update this code, update kapplymousetheme as well.

    // use a default value for theme only if it's not configured at all, not even in X resources
    if (theme.isEmpty() && currentCursorTheme().isEmpty()) {
        theme = "breeze_cursors";
    }
    applyCursorTheme(theme, size);

    // Tell klauncher to set the XCURSOR_THEME and XCURSOR_SIZE environment
    // variables when launching applications.
    if (!theme.isEmpty()) {
        UpdateLaunchEnvJob launchEnvJob(QStringLiteral("XCURSOR_THEME"), theme);
    }
    UpdateLaunchEnvJob launchEnvJob(QStringLiteral("XCURSOR_SIZE"), QByteArray::number(size));
}
