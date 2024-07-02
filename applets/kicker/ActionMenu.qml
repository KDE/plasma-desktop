/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>
    SPDX-FileCopyrightText: 2024 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

pragma ComponentBehavior: Bound

import QtQuick

import org.kde.plasma.extras as PlasmaExtras

QtObject {
    id: root

    property Item visualParent

    // See tools.js for a list of possible members
    property var actionList

    readonly property bool opened: __menu ? (__menu.status !== PlasmaExtras.Menu.Closed) : false

    property PlasmaExtras.Menu __menu

    signal actionClicked(string actionId, var actionArgument)
    signal closed()

    onActionListChanged: {
        __refreshMenu();
    }

    onOpenedChanged: {
        if (!opened) {
            closed();
        }
    }

    function open(x: var, y: var): void {
        if (!actionList) {
            return;
        }

        if (x !== undefined && y !== undefined) {
            __menu.open(x, y);
        } else {
            __menu.open();
        }
    }

    function __refreshMenu(): void {
        if (__menu) {
            __menu.destroy();
        }

        if (!actionList) {
            return;
        }

        __menu = contextMenuComponent.createObject(this);

        __fillMenu(__menu, actionList);
    }

    function __fillMenu(menu: PlasmaExtras.Menu, actionItems: var): void {
        actionItems.forEach(actionItem => {
            if (actionItem.subActions) {
                // This is a menu
                const submenuItem = contextSubmenuItemComponent.createObject(menu, { actionItem });
                __fillMenu(submenuItem.submenu, actionItem.subActions);
            } else {
                contextMenuItemComponent.createObject(menu, { actionItem });
            }
        });
    }

    readonly property list<QtObject> __data: [
        Component {
            id: contextMenuComponent

            PlasmaExtras.Menu {
                visualParent: root.visualParent
            }
        },

        Component {
            id: contextSubmenuItemComponent

            PlasmaExtras.MenuItem {
                id: submenuItem

                required property var actionItem

                text: actionItem.text ?? ""
                icon: actionItem.icon ?? null

                readonly property PlasmaExtras.Menu submenu: PlasmaExtras.Menu {
                    visualParent: submenuItem.action
                }
            }
        },

        Component {
            id: contextMenuItemComponent

            PlasmaExtras.MenuItem {
                required property var actionItem

                text      : actionItem.text ?? ""
                enabled   : actionItem.type !== "title" && (actionItem.enabled ?? true)
                separator : actionItem.type === "separator"
                section   : actionItem.type === "title"
                icon      : actionItem.icon ?? null
                checkable : actionItem.checkable ?? false
                checked   : actionItem.checked ?? false

                onClicked: {
                    root.actionClicked(actionItem.actionId, actionItem.actionArgument);
                }
            }
        }
    ]
}
