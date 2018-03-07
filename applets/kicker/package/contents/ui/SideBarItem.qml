/***************************************************************************
 *   Copyright (C) 2013-2015 by Eike Hein <hein@kde.org>                   *
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
import org.kde.kquickcontrolsaddons 2.0

import "code/tools.js" as Tools

Item {
    id: item

    width: root.width
    height: root.width

    signal actionTriggered(string actionId, variant actionArgument)
    signal aboutToShowActionMenu(variant actionMenu)

    property bool hasActionList: ((model.favoriteId != null)
        || (("hasActionList" in model) && (model.hasActionList != null)))
    property int itemIndex: model.index

    onAboutToShowActionMenu: {
        var actionList = (model.hasActionList != null) ? model.actionList : [];
        Tools.fillActionMenu(i18n, actionMenu, actionList, repeater.model, model.favoriteId);
    }

    onActionTriggered: {
        if (Tools.triggerAction(repeater.model, model.index, actionId, actionArgument) === true) {
            plasmoid.expanded = false;
        }
    }

    function openActionMenu(visualParent, x, y) {
        aboutToShowActionMenu(actionMenu);
        actionMenu.visualParent = visualParent;
        actionMenu.open(x, y);
    }

    ActionMenu {
        id: actionMenu

        onActionClicked: {
            actionTriggered(actionId, actionArgument);
        }
    }

    PlasmaCore.IconItem {
        anchors.fill: parent

        active: toolTip.containsMouse

        source: model.decoration

        usesPlasmaTheme: repeater.usesPlasmaTheme
    }

    MouseEventListener {
        id: listener

        anchors {
            fill: parent
            leftMargin: - sideBar.margins.left
            rightMargin: - sideBar.margins.right
        }

        enabled: (item.parent && !item.parent.animating)

        property bool pressed: false
        property int pressX: -1
        property int pressY: -1

        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onPressed: {
            if (mouse.buttons & Qt.RightButton) {
                if (item.hasActionList) {
                    item.openActionMenu(item, mouse.x, mouse.y);
                }
            } else {
                pressed = true;
                pressX = mouse.x;
                pressY = mouse.y;
            }
        }

        onReleased: {
            if (pressed) {
                repeater.model.trigger(index, "", null);
                plasmoid.expanded = false;
            }

            pressed = false;
            pressX = -1;
            pressY = -1;
        }

        onContainsMouseChanged: {
            if (!containsMouse) {
                pressed = false;
                pressX = -1;
                pressY = -1;
            }
        }

        onPositionChanged: {
            if (pressX != -1 && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y)) {
                kicker.dragSource = item;
                dragHelper.startDrag(kicker, model.url, model.icon);
                pressed = false;
                pressX = -1;
                pressY = -1;

                return;
            }
        }
    }

    PlasmaCore.ToolTipArea {
        id: toolTip

        property string text: model.display

        anchors {
            fill: parent
            leftMargin: - sideBar.margins.left
            rightMargin: - sideBar.margins.right
        }

        interactive: false
        location: (((plasmoid.location == PlasmaCore.Types.RightEdge)
            || (Qt.application.layoutDirection == Qt.RightToLeft))
            ? PlasmaCore.Types.RightEdge : PlasmaCore.Types.LeftEdge)

        mainItem: toolTipDelegate
    }
}

