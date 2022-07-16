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

    property int currentIndex: -1

    text: i18nd("plasma_lookandfeel_org.kde.lookandfeel", "Desktop Session: %1", instantiator.objectAt(currentIndex).text || "")
    visible: menu.count > 1

    Component.onCompleted: {
        currentIndex = sessionModel.lastIndex
    }
    checkable: true
    checked: menu.opened
    onToggled: {
        if (checked) {
            menu.popup(root, 0, 0)
        } else {
            menu.dismiss()
        }
    }

    signal sessionChanged()

    PlasmaComponents.Menu {
        PlasmaCore.ColorScope.colorGroup: PlasmaCore.Theme.NormalColorGroup
        PlasmaCore.ColorScope.inherit: false

        id: menu
        Instantiator {
            id: instantiator
            model: sessionModel
            onObjectAdded: menu.insertItem(index, object)
            onObjectRemoved: menu.removeItem(object)
            delegate: PlasmaComponents.MenuItem {
                text: model.name
                onTriggered: {
                    root.currentIndex = model.index
                    sessionChanged()
                }
            }
        }
    }
}
