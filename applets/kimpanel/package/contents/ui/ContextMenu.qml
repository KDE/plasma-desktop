/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

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
                kimpanel.showAction(actionItem.key);
            }
        }
    }

    Component {
        id: contextMenuComponent

        PlasmaExtras.Menu {
            visualParent: root.visualParent
            property variant actionItem
            property Item separator: separatorItem
            property QtObject showMenu: subShowMenu

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
                text: i18n("Show")

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
                text: i18n("Hide %1", actionItem.label);
                onClicked: {
                    kimpanel.hideAction(actionItem.key);
                }
                enabled: actionItem.key !== 'kimpanel-placeholder'
                visible: enabled
            }

            PlasmaExtras.MenuItem {
                text: i18n("Configure Input Method")
                icon: "configure"
                onClicked: {
                    helper.configure();
                }
            }

            PlasmaExtras.MenuItem {
                text: i18n("Reload Config")
                icon: "view-refresh"
                onClicked: {
                    helper.reloadConfig();
                }
            }

            PlasmaExtras.MenuItem {
                text: i18n("Exit Input Method")
                icon: "application-exit"
                onClicked: {
                    helper.exit();
                }
            }

            PlasmaExtras.MenuItem {
                property QtObject configureAction: null

                enabled: configureAction && configureAction.enabled

                text: configureAction ? configureAction.text : ""
                icon: configureAction ? configureAction.icon : ""

                onClicked: configureAction.trigger()

                Component.onCompleted: configureAction = Plasmoid.internalAction("configure")
            }
        }
    }
}
