/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.components 2.0 as PlasmaComponents // for Menu + MenuItem

Item {
    id: actionMenuRoot

    property QtObject menu
    property Item visualParent
    property variant actionList

    signal actionClicked(string actionId, variant actionArgument)

    onActionListChanged: refreshMenu();

    function open(x, y) {
        if (!actionList || !actionList.length) {
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

        menu = contextMenuComponent.createObject(actionMenuRoot);

        // actionList.forEach(function(actionItem) {
        //     var item = contextMenuItemComponent.createObject(menu, {
        //         "actionItem": actionItem,
        //     });
        // });

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

        PlasmaComponents.Menu {
            visualParent: actionMenuRoot.visualParent
        }
    }

    Component {
        id: contextSubmenuItemComponent

        PlasmaComponents.MenuItem {
            id: submenuItem

            property variant actionItem

            text: actionItem.text ? actionItem.text : ""
            icon: actionItem.icon ? actionItem.icon : null

            property variant submenu : submenu_

            PlasmaComponents.Menu {
                id: submenu_
                visualParent: submenuItem.action
            }
        }
    }

    Component {
        id: contextMenuItemComponent

        PlasmaComponents.MenuItem {
            property variant actionItem

            text      : actionItem.text ? actionItem.text : ""
            enabled   : actionItem.type !== "title" && ("enabled" in actionItem ? actionItem.enabled : true)
            separator : actionItem.type === "separator"
            section   : actionItem.type === "title"
            icon      : actionItem.icon ? actionItem.icon : null
            checkable : actionItem.checkable ? actionItem.checkable : false
            checked   : actionItem.checked ? actionItem.checked : false

            onClicked: {
                actionMenuRoot.actionClicked(actionItem.actionId, actionItem.actionArgument);
            }
        }
    }
}
