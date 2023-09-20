/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: root

    property QtObject menu
    property Item visualParent
    property variant actionList
    property bool opened: menu ? (menu.status !== PlasmaExtras.Menu.Closed) : false

    signal actionClicked(string actionId, variant actionArgument)
    signal closed

    onActionListChanged: refreshMenu();

    onOpenedChanged: {
        if (!opened) {
            closed();
        }
    }

    function open(x, y) {
        if (!actionList) {
            return;
        }

        if (x && y) {
            menu.open(x, y);
        } else {
            menu.open();
        }
    }

    function refreshMenu() {
        if (menu) {
            menu.destroy();
        }

        if (!actionList) {
            return;
        }

        menu = contextMenuComponent.createObject(root);

        fillMenu(menu, actionList);
    }

    function fillMenu(menu, items) {
        items.forEach(function(actionItem) {
            if (actionItem.subActions) {
                // This is a menu
                var submenuItem = contextSubmenuItemComponent.createObject(
                                          menu, { "actionItem" : actionItem });

                fillMenu(submenuItem.submenu, actionItem.subActions);

            } else {
                var item = contextMenuItemComponent.createObject(
                                menu,
                                {
                                    "actionItem": actionItem,
                                }
                );
            }
        });

    }

    Component {
        id: contextMenuComponent

        PlasmaExtras.Menu {
            visualParent: root.visualParent
        }
    }

    Component {
        id: contextSubmenuItemComponent

        PlasmaExtras.MenuItem {
            id: submenuItem

            property variant actionItem

            text: actionItem.text ? actionItem.text : ""
            icon: actionItem.icon ? actionItem.icon : null

            property PlasmaExtras.Menu submenu: PlasmaExtras.Menu {
                visualParent: submenuItem.action
            }
        }
    }

    Component {
        id: contextMenuItemComponent

        PlasmaExtras.MenuItem {
            property variant actionItem

            text      : actionItem.text ? actionItem.text : ""
            enabled   : actionItem.type !== "title" && ("enabled" in actionItem ? actionItem.enabled : true)
            separator : actionItem.type === "separator"
            section   : actionItem.type === "title"
            icon      : actionItem.icon ? actionItem.icon : null
            checkable : actionItem.checkable ? actionItem.checkable : false
            checked   : actionItem.checked ? actionItem.checked : false

            onClicked: {
                root.actionClicked(actionItem.actionId, actionItem.actionArgument);
            }
        }
    }
}
