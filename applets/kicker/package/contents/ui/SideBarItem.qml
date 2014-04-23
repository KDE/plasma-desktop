/***************************************************************************
 *   Copyright (C) 2013-2014 by Eike Hein <hein@kde.org>                   *
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
import org.kde.draganddrop 2.0

import "../code/tools.js" as Tools

MouseEventListener {
    id: item

    width: root.width
    height: root.width

    signal actionTriggered(string actionId, variant actionArgument)
    signal aboutToShowActionMenu(variant actionMenu)

    property bool hasActionList: ((model.favoriteId != null)
        || (("hasActionList" in model) && (model.hasActionList != null)))
    property int itemIndex: model.index
    property bool clicked: false

    hoverEnabled: true
    acceptedButtons: Qt.LeftButton | Qt.RightButton

    onPressed: {
        if (mouse.buttons & Qt.RightButton) {
            if (hasActionList) {
                openActionMenu(item, mouse.x, mouse.y);
            }
        } else {
            clicked = true;
        }
    }

    onReleased: {
        if (clicked) {
            repeater.model.trigger(index, "", null);
            plasmoid.expanded = false;
        }

        clicked = false;
    }

    onContainsMouseChanged: {
        if (!containsMouse) {
            clicked = false;
        }
    }

    onAboutToShowActionMenu: {
        var actionList = (model.hasActionList != null) ? model.actionList : [];
        Tools.fillActionMenu(actionMenu, actionList, model.favoriteId, model.display);
    }

    onActionTriggered: {
        Tools.triggerAction(repeater.model, model.index, actionId, actionArgument);
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

        active: containsMouse

        source: model.decoration
    }

    DragArea {
        anchors.fill: parent

        enabled: containsMouse
        supportedActions: Qt.MoveAction

        mimeData.source: item
    }
}

