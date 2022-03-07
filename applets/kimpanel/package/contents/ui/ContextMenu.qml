/*
    SPDX-FileCopyrightText: 2013 Aurélien Gâteau <agateau@kde.org>
    SPDX-FileCopyrightText: 2014-2015 Eike Hein <hein@kde.org>

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

        PlasmaComponents.MenuItem {
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

        PlasmaComponents.ContextMenu {
            visualParent: root.visualParent
            property variant actionItem
            property Item separator: separatorItem
            property QtObject showMenu: subShowMenu

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

            PlasmaComponents.MenuItem {
                id: showItem
                visible: separatorItem.visible
                text: i18n("Show")

                PlasmaComponents.ContextMenu {
                    id: subShowMenu
                    visualParent: showItem.action
                }
            }

            PlasmaComponents.MenuItem {
                id: separatorItem
                separator: true
                visible: false
            }

            PlasmaComponents.MenuItem {
                text: i18n("Hide %1", actionItem.label);
                onClicked: {
                    kimpanel.hideAction(actionItem.key);
                }
                enabled: actionItem.key !== 'kimpanel-placeholder'
                visible: enabled
            }

            PlasmaComponents.MenuItem {
                text: i18n("Configure Input Method")
                icon: "configure"
                onClicked: {
                    helper.configure();
                }
            }

            PlasmaComponents.MenuItem {
                text: i18n("Reload Config")
                icon: "view-refresh"
                onClicked: {
                    helper.reloadConfig();
                }
            }

            PlasmaComponents.MenuItem {
                text: i18n("Exit Input Method")
                icon: "application-exit"
                onClicked: {
                    helper.exit();
                }
            }

            PlasmaComponents.MenuItem {
                property QtObject configureAction: null

                enabled: configureAction && configureAction.enabled

                text: configureAction ? configureAction.text : ""
                icon: configureAction ? configureAction.icon : ""

                onClicked: configureAction.trigger()

                Component.onCompleted: configureAction = Plasmoid.action("configure")
            }
        }
    }
}
