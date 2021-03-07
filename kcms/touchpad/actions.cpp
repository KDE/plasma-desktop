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

#include "actions.h"

#include <KGlobalAccel>
#include <KLocalizedString>
#include <QAction>
#include <QDebug>

TouchpadGlobalActions::TouchpadGlobalActions(bool isConfiguration, QObject *parent)
    : KActionCollection(parent)
{
    // setComponentName(TouchpadPluginFactory::componentData());
    setComponentName("kcm_touchpad");
    setComponentDisplayName(i18n("Touchpad"));

    QAction *enable = addAction("Enable Touchpad");
    enable->setText(i18n("Enable Touchpad"));
    connect(enable, SIGNAL(triggered()), SIGNAL(enableTriggered()));
    bool okEnable = KGlobalAccel::setGlobalShortcut(enable, QKeySequence(Qt::Key_TouchpadOn));
    if (!okEnable) {
        qWarning() << "Couldn't set global shortcut to Qt::Key_TouchpadOn. There's another program using it, otherwise file a bug against kcm_touchpad";
    }

    QAction *disable = addAction("Disable Touchpad");
    disable->setText(i18n("Disable Touchpad"));
    connect(disable, SIGNAL(triggered()), SIGNAL(disableTriggered()));
    bool okDisable = KGlobalAccel::setGlobalShortcut(disable, QKeySequence(Qt::Key_TouchpadOff));
    if (!okDisable) {
        qWarning() << "Couldn't set global shortcut to Qt::Key_TouchpadOff. There's another program using it, otherwise file a bug against kcm_touchpad";
    }

    QAction *toggle = addAction("Toggle Touchpad");
    toggle->setText(i18n("Toggle Touchpad"));
    connect(toggle, SIGNAL(triggered()), SIGNAL(toggleTriggered()));
    bool okToggle = KGlobalAccel::setGlobalShortcut(toggle, QKeySequence(Qt::Key_TouchpadToggle));
    if (!okToggle) {
        qWarning() << "Couldn't set global shortcut to Qt::Key_TouchpadToggle. There's another program using it, otherwise file a bug against kcm_touchpad";
    }

    const auto actionsList = actions();
    for (QAction *act : actionsList) {
        KActionCollection::setShortcutsConfigurable(act, true);
        if (isConfiguration) {
            act->setProperty("isConfigurationAction", true);
        }
    }
}
