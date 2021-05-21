/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
