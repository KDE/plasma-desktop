/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma Singleton // NOTE: Singletons are shared between all instances of a plasmoid

import QtQuick 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PC2 // for Menu + MenuItem
import "code/tools.js" as Tools

Item {
    id: root
    property var actionList: menu.visualParent ? menu.visualParent.actionList : null

    // Workaround for `plasmoid` context property not working in singletons.
    // Only one action menu can be open at a time, so this should be safe to use.
    property Plasmoid plasmoid: null

    // Not a QQC1 Menu. It's actually a custom QObject that uses a QMenu.
    readonly property PC2.Menu menu: PC2.Menu {
        id: menu
        visualParent: null
        placement: PlasmaCore.Types.BottomPosedLeftAlignedPopup
    }

    visible: false

    Instantiator {
        active: actionList != null
        model: actionList
        delegate: menuItemComponent
        onObjectAdded: menu.addMenuItem(object)
        onObjectRemoved: menu.removeMenuItem(object)
    }

    Component { id: menuComponent; PC2.Menu {} }

    Component {
        id: menuItemComponent
        PC2.MenuItem {
            id: menuItem
            required property var modelData
            property PC2.Menu subMenu: if (modelData.subActions) {
                return menuComponent.createObject(menuItem, {visualParent: menuItem.action})
            } else {
                return null
            }

            text: modelData.text ? modelData.text : ""
            enabled: modelData.type !== "title" && ("enabled" in modelData ? modelData.enabled : true)
            separator: modelData.type === "separator"
            section: modelData.type === "title"
            icon: modelData.icon ? modelData.icon : null
            checkable: modelData.hasOwnProperty("checkable") ? modelData.checkable : false
            checked: modelData.hasOwnProperty("checked") ? modelData.checked : false

            Instantiator {
                active: menuItem.subMenu != null
                model: modelData.subActions
                delegate: menuItemComponent
                onObjectAdded: subMenu.addMenuItem(object)
                onObjectRemoved: subMenu.removeMenuItem(object)
            }

            onClicked: {
                const modelActionTriggered = Tools.triggerAction(
                    menu.visualParent.view.model,
                    menu.visualParent.index,
                    modelData.actionId,
                    modelData.actionArgument
                )
                if (modelActionTriggered) {
                    root.plasmoid.expanded = false
                }
            }
        }
    }
}
