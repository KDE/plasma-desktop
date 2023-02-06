/*
    SPDX-FileCopyrightText: 2016 David Edmundson <davidedmundson@kde.org>
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

PlasmaComponents.ToolButton {
    id: root

    property int currentIndex: keyboard.currentLayout
    onCurrentIndexChanged: keyboard.currentLayout = currentIndex

    text: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Keyboard Layout: %1", keyboard.layouts[currentIndex].longName)
    visible: menu.count > 1

    checkable: true
    checked: menu.opened
    onToggled: {
        if (checked) {
            menu.popup(root, 0, 0)
        } else {
            menu.dismiss()
        }
    }

    signal keyboardLayoutChanged()

    PlasmaComponents.Menu {
        id: menu
        PlasmaCore.ColorScope.colorGroup: PlasmaCore.Theme.NormalColorGroup
        PlasmaCore.ColorScope.inherit: false

        Instantiator {
            id: instantiator
            model: {
                let layouts = keyboard.layouts;
                layouts.sort((a, b) => a.longName.localeCompare(b.longName));
                return layouts;
            }
            onObjectAdded: menu.insertItem(index, object)
            onObjectRemoved: menu.removeItem(object)
            delegate: PlasmaComponents.MenuItem {
                text: modelData.longName
                onTriggered: {
                    keyboard.currentLayout = keyboard.layouts.indexOf(modelData)
                    root.keyboardLayoutChanged()
                }
            }
        }
    }
}
