/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick

import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid

Item {
    id: root

    property PlasmaExtras.Menu menu
    property Item visualParent
    property var actionList

    signal actionClicked(string actionId)

    onActionListChanged: refreshMenu();

    function open() {
        menu.openRelative();
    }

    function refreshMenu() {
        if (menu) {
            menu.destroy();
        }

        menu = contextMenuComponent.createObject(root);

        if (!actionList || actionList.length === 0) {
            const item = emptyMenuItemComponent.createObject(menu);

            menu.addMenuItem(item);

            return;
        }

        actionList.forEach(actionItem => {
            const item = contextMenuItemComponent.createObject(menu, { actionItem });

            menu.addMenuItem(item);
        });
    }

    Component {
        id: contextMenuComponent

        PlasmaExtras.Menu {
            visualParent: root.visualParent

            placement: {
                if (Plasmoid.location === PlasmaCore.Types.LeftEdge) {
                    return PlasmaExtras.Menu.RightPosedTopAlignedPopup;
                } else if (Plasmoid.location === PlasmaCore.Types.TopEdge) {
                    return PlasmaExtras.Menu.BottomPosedLeftAlignedPopup;
                } else if (Plasmoid.location === PlasmaCore.Types.RightEdge) {
                    return PlasmaExtras.Menu.LeftPosedTopAlignedPopup;
                } else {
                    return PlasmaExtras.Menu.TopPosedLeftAlignedPopup;
                }
            }
        }
    }

    Component {
        id: contextMenuItemComponent

        PlasmaExtras.MenuItem {
            property var actionItem

            text: actionItem.text ? actionItem.text : ""
            icon: actionItem.icon ? actionItem.icon : null
            checkable: actionItem.hint === "checked"
            checked: actionItem.hint === "checked"

            onClicked: {
                root.actionClicked(actionItem.actionId);
            }
        }
    }

    Component {
        id: emptyMenuItemComponent
        PlasmaExtras.MenuItem {
            text: i18nc("@label placeholder for empty menu", "(Empty)") // qmllint disable unqualified
            enabled: false
        }
    }
}
