/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid 2.0

Item {
    id: root

    property QtObject menu
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
                actionClicked(actionItem.actionId);
            }
        }
    }

    Component {
        id: emptyMenuItemComponent
        PlasmaExtras.MenuItem {
            text: i18n("(Empty)")
            enabled: false
        }
    }
}
