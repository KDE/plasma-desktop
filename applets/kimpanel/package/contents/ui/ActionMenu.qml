/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.plasmoid 2.0

Item {
    id: root

    property QtObject menu
    property Item visualParent
    property variant actionList

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
            var item = emptyMenuItemComponent.createObject(menu);

            menu.addMenuItem(item);

            return;
        }

        actionList.forEach(function(actionItem) {
            var item = contextMenuItemComponent.createObject(menu, {
                "actionItem": actionItem,
            });

            menu.addMenuItem(item);
        });
    }

    Component {
        id: contextMenuComponent

        PlasmaComponents.ContextMenu {
            visualParent: root.visualParent

            placement: {
                if (Plasmoid.location === PlasmaCore.Types.LeftEdge) {
                    return PlasmaCore.Types.RightPosedTopAlignedPopup;
                } else if (Plasmoid.location === PlasmaCore.Types.TopEdge) {
                    return PlasmaCore.Types.BottomPosedLeftAlignedPopup;
                } else if (Plasmoid.location === PlasmaCore.Types.RightEdge) {
                    return PlasmaCore.Types.LeftPosedTopAlignedPopup;
                } else {
                    return PlasmaCore.Types.TopPosedLeftAlignedPopup;
                }
            }
        }
    }

    Component {
        id: contextMenuItemComponent

        PlasmaComponents.MenuItem {
            property variant actionItem

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
        PlasmaComponents.MenuItem {
            text: i18n("(Empty)")
            enabled: false
        }
    }
}
