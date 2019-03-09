/***************************************************************************
 *   Copyright (C) 2013 by Aurélien Gâteau <agateau@kde.org>               *
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

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
                if (plasmoid.location === PlasmaCore.Types.LeftEdge) {
                    return PlasmaCore.Types.RightPosedTopAlignedPopup;
                } else if (plasmoid.location === PlasmaCore.Types.TopEdge) {
                    return PlasmaCore.Types.BottomPosedLeftAlignedPopup;
                } else if (plasmoid.location === PlasmaCore.Types.RightEdge) {
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
