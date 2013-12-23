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

#include "kdedactions.h"

#include <KAction>
#include <KLocale>

#include "plugins.h"

TouchpadGlobalActions::TouchpadGlobalActions(QObject *parent)
    : KActionCollection(parent)
{
    setComponentData(TouchpadPluginFactory::componentData());

    KAction *enable = addAction("Enable Touchpad");
    enable->setText(i18n("Enable Touchpad"));
    connect(enable, SIGNAL(triggered()), SIGNAL(enableTriggered()));

    KAction *disable = addAction("Disable Touchpad");
    disable->setText(i18n("Disable Touchpad"));
    connect(disable, SIGNAL(triggered()), SIGNAL(disableTriggered()));

    KAction *toggle = addAction("Toggle Touchpad");
    toggle->setText(i18n("Toggle Touchpad"));
    connect(toggle, SIGNAL(triggered()), SIGNAL(toggleTriggered()));

    Q_FOREACH (QAction *i, actions()) {
        KAction *act = qobject_cast<KAction *>(i);
        act->setGlobalShortcut(KShortcut());
        act->setShortcutConfigurable(true);
    }
}
