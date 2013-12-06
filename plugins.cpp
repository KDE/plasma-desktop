/*
 * Copyright (C) 2013 Alexander Mezin <mezin.alexander@gmail.com>
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

#include "plugins.h"

#include <KAboutData>
#include <KPluginFactory>

#include "touchpadconfig.h"
#include "touchpadbackend.h"
#include "kded.h"
#include "version.h"

static KAboutData buildAboutData()
{
    KAboutData data("kcm_touchpad",
                    QByteArray(),
                    ki18n("Touchpad Settings"),
                    TOUCHPAD_KCM_VERSION,
                    ki18n("Touchpad configuration module for KDE"),
                    KAboutData::License_GPL_V2,
                    ki18n("Copyright © 2013 Alexander Mezin"),
                    ki18n("This program incorporates work covered by this "
                          "copyright notice:\n"
                          "Copyright © 2002-2005,2007 Peter Osterlund"),
                    "https://github.com/sanya-m/kde-touchpad-config",
                    "mezin.alexander@gmail.com");

    data.addAuthor(ki18n("Alexander Mezin"),
                   ki18n("Developer"),
                   "mezin.alexander@gmail.com");
    data.addCredit(ki18n("Thomas Pfeiffer"), ki18n("Testing"));
    data.addCredit(ki18n("Alex Fiestas"), ki18n("Helped a bit"));
    data.addCredit(ki18n("Peter Osterlund"), ki18n("Developer of synclient"));
    data.addCredit(ki18n("Vadim Zaytsev"), ki18n("Testing"));
    data.addCredit(ki18n("Violetta Raspryagayeva"), ki18n("Testing"));

    return data;
}

K_PLUGIN_FACTORY_DEFINITION(TouchpadPluginFactory,
                            registerPlugin<TouchpadDisabler>();
                            registerPlugin<TouchpadConfig>("kcm");)
K_EXPORT_PLUGIN(TouchpadPluginFactory(buildAboutData()))

extern "C"
{
    KDE_EXPORT void kcminit_touchpad()
    {
        TouchpadBackend *backend = TouchpadBackend::self();

        if (!backend) {
            return;
        }

        {
            TouchpadParameters active;
            backend->getConfig(&active);
            active.saveAsDefaults();
        }

        TouchpadParameters config;
        backend->applyConfig(&config);
    }
}
