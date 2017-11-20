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

#include <KConfig>
#include <KConfigGroup>
#include <QFile>

#include "mousesettings.h"
#include "mousebackend.h"

#include <klauncher_iface.h>

extern "C"
{
    Q_DECL_EXPORT void kcminit_mouse()
    {
        KConfig* config = new KConfig("kcminputrc", KConfig::NoGlobals);

        auto backend = MouseBackend::implementation();

        MouseSettings settings;
        settings.load(config, backend);
        settings.apply(backend, true);    // force

        KConfigGroup group = config->group("Mouse");
        QString theme = group.readEntry("cursorTheme", QString());
        QString size = group.readEntry("cursorSize", QString());
        if (backend) {
            int intSize = -1;
            if (size.isEmpty()) {
                bool ok;
                uint value = size.toUInt(&ok);
                if (ok) {
                    intSize = value;
                }
            }
            // Note: If you update this code, update kapplymousetheme as well.

            // use a default value for theme only if it's not configured at all, not even in X resources
            if (theme.isEmpty() && backend->currentCursorTheme().isEmpty()) {
                theme = "breeze_cursors";
            }
            backend->applyCursorTheme(theme, intSize);
        }

        // Tell klauncher to set the XCURSOR_THEME and XCURSOR_SIZE environment
        // variables when launching applications.
        OrgKdeKLauncherInterface klauncher(QStringLiteral("org.kde.klauncher5"),
                                           QStringLiteral("/KLauncher"),
                                           QDBusConnection::sessionBus());
        if (!theme.isEmpty()) {
            klauncher.setLaunchEnv(QStringLiteral("XCURSOR_THEME"), theme);
        }
        if (!size.isEmpty()) {
            klauncher.setLaunchEnv(QStringLiteral("XCURSOR_SIZE"), size);
        }

        delete config;
    }
}
