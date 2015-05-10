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

import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: root

    property QtObject menu
    property Item visualParent
    property variant actionList
    property bool opened: menu ? (menu.status != PlasmaComponents.DialogStatus.Closed) : false

    signal actionClicked(string actionId, variant actionArgument)
    signal closed

    onActionListChanged: refreshMenu();

    onOpenedChanged: {
        if (!opened) {
            closed();
        }
    }

    function open(x, y) {
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

        menu = contextMenuComponent.createObject(root);

        if (!actionList || actionList.length == 0) {
            var item = emptyMenuItemComponent.createObject(menu);

            menu.addMenuItem(item);

            return;
        }

        actionList.forEach(function(actionItem) {
            var item = contextMenuItemComponent.createObject(menu, {
                "actionItem": actionItem,
            });
        });
    }

    Component {
        id: contextMenuComponent

        PlasmaComponents.ContextMenu {
            visualParent: root.visualParent
        }
    }

    Component {
        id: contextMenuItemComponent

        PlasmaComponents.MenuItem {
            property variant actionItem

            text: actionItem.text ? actionItem.text : ""
            enabled: actionItem.type != "title" && ("enabled" in actionItem ? actionItem.enabled : true)
            separator: actionItem.type == "separator"
            section: actionItem.type == "title"
            icon: actionItem.icon ? actionItem.icon : null

            onClicked: {
                actionClicked(actionItem.actionId, actionItem.actionArgument);
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
