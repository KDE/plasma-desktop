/*
 * main.cpp
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <config-X11.h>

#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kdemacros.h>
#include <QFile>

#include "mouse.h"
#include <QX11Info>

#include <klauncher_iface.h>

#include <X11/Xlib.h>
#ifdef HAVE_XCURSOR
#  include <X11/Xcursor/Xcursor.h>
#endif

extern "C"
{
  Q_DECL_EXPORT void kcminit_mouse()
  {
      KConfig *config = new KConfig("kcminputrc", KConfig::NoGlobals );
    MouseSettings settings;
    settings.load(config);
    settings.apply(true); // force

#ifdef HAVE_XCURSOR
    KConfigGroup group = config->group("Mouse");
    QString theme = group.readEntry("cursorTheme", QString());
    QString size = group.readEntry("cursorSize", QString());

    // Note: If you update this code, update kapplymousetheme as well.

    // use a default value for theme only if it's not configured at all, not even in X resources
    if( theme.isEmpty()
        && QByteArray( XGetDefault( QX11Info::display(), "Xcursor", "theme" )).isEmpty()
        && QByteArray( XcursorGetTheme( QX11Info::display())).isEmpty())
    {
        theme = "breeze_cursors";
    }

     // Apply the KDE cursor theme to ourselves
    if( !theme.isEmpty())
        XcursorSetTheme(QX11Info::display(), QFile::encodeName(theme));

    if (!size.isEmpty())
    	XcursorSetDefaultSize(QX11Info::display(), size.toUInt());

    // Load the default cursor from the theme and apply it to the root window.
    Cursor handle = XcursorLibraryLoadCursor(QX11Info::display(), "left_ptr");
    XDefineCursor(QX11Info::display(), QX11Info::appRootWindow(), handle);
    XFreeCursor(QX11Info::display(), handle); // Don't leak the cursor

    // Tell klauncher to set the XCURSOR_THEME and XCURSOR_SIZE environment
    // variables when launching applications.
    OrgKdeKLauncherInterface klauncher(QStringLiteral("org.kde.klauncher5"),
                                       QStringLiteral("/KLauncher"),
                                       QDBusConnection::sessionBus());
    if(!theme.isEmpty())
        klauncher.setLaunchEnv(QStringLiteral("XCURSOR_THEME"), theme);
    if( !size.isEmpty())
        klauncher.setLaunchEnv(QStringLiteral("XCURSOR_SIZE"), size);

#endif

    delete config;
  }
}


