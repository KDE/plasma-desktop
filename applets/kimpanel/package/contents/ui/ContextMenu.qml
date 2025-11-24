/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
pragma ComponentBehavior: Bound

import QtQuick

import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid
import org.kde.plasma.private.kimpanel as Kimpanel

Item {
    id: root

    signal showAction(string key)
    signal hideAction(string key)

    required property Kimpanel.Kimpanel helper

    property ContextMenuComponent menu
    property Item visualParent
    property variant actionList

    function open(item, actionItem) {
        visualParent = item;
        refreshMenu(actionItem);
        menu.openRelative();
    }

    function refreshMenu(actionItem) {
        if (menu) {
            menu.destroy();
        }

        menu = contextMenuComponent.createObject(root, {"actionItem": actionItem});

        if (actionList && actionList.length > 0) {
            menu.separator.visible = true;
            actionList.forEach(function(actionItem) {
                var item = contextMenuItemComponent.createObject(menu.showMenu, {
                    "actionItem": actionItem,
                });
            });
        }
    }

    Component {
        id: contextMenuItemComponent

        PlasmaExtras.MenuItem {
            property variant actionItem

            text: actionItem.label
            icon: actionItem.icon

            onClicked: {
                root.showAction(actionItem.key);
            }
        }
    }

    component ContextMenuComponent: PlasmaExtras.Menu {
        property variant actionItem
        property PlasmaExtras.MenuItem separator
        property PlasmaExtras.Menu showMenu
    }

    Component {
        id: contextMenuComponent

        ContextMenuComponent {
            id: menu
            visualParent: root.visualParent
            separator: separatorItem
            showMenu: subShowMenu

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

            PlasmaExtras.MenuItem {
                id: showItem
                visible: separatorItem.visible
                text: i18nc("@action:inmenu", "Show") // qmllint disable unqualified

                property PlasmaExtras.Menu _subShowMenu: PlasmaExtras.Menu {
                    id: subShowMenu
                    visualParent: showItem.action
                }
            }

            PlasmaExtras.MenuItem {
                id: separatorItem
                separator: true
                visible: false
            }

            PlasmaExtras.MenuItem {
                text: i18nc("@action:inmenu", "Hide %1", menu.actionItem.label) // qmllint disable unqualified
                onClicked: {
                    root.hideAction(menu.actionItem.key);
                }
                enabled: menu.actionItem.key !== 'kimpanel-placeholder'
                visible: enabled
            }

            PlasmaExtras.MenuItem {
                text: i18nc("@action:inmenu", "Configure Input Method") // qmllint disable unqualified
                icon: "configure"
                onClicked: {
                    root.helper.configure();
                }
            }

            PlasmaExtras.MenuItem {
                text: i18nc("@action:inmenu", "Reload Config") // qmllint disable unqualified
                icon: "view-refresh"
                onClicked: {
                    root.helper.reloadConfig();
                }
            }

            PlasmaExtras.MenuItem {
                text: i18nc("@action:inmenu", "Exit Input Method") // qmllint disable unqualified
                icon: "application-exit"
                onClicked: {
                    root.helper.exit();
                }
            }

            PlasmaExtras.MenuItem {
                property PlasmaCore.Action configureAction: null

                enabled: configureAction && configureAction.enabled

                text: configureAction ? configureAction.text : ""
                icon: configureAction ? configureAction.icon : ""

                onClicked: configureAction.trigger()

                Component.onCompleted: configureAction = Plasmoid.internalAction("configure")
            }
        }
    }
}
