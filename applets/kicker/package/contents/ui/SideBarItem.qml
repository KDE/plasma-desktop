/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.kquickcontrolsaddons 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

import "code/tools.js" as Tools

Item {
    id: item

    width: root.width
    height: root.width

    signal actionTriggered(string actionId, variant actionArgument)
    signal aboutToShowActionMenu(variant actionMenu)

    property bool hasActionList: ((model.favoriteId !== null)
        || (("hasActionList" in model) && (model.hasActionList !== null)))
    property int itemIndex: model.index

    onAboutToShowActionMenu: actionMenu => {
        const actionList = (model.hasActionList !== null) ? model.actionList : [];
        Tools.fillActionMenu(i18n, actionMenu, actionList, repeater.model, model.favoriteId);
    }

    onActionTriggered: (actionId, actionArgument) => {
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
            if (pressX !== -1 && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y)) {
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
        location: (((plasmoid.location === PlasmaCore.Types.RightEdge)
            || (Qt.application.layoutDirection === Qt.RightToLeft))
            ? PlasmaCore.Types.RightEdge : PlasmaCore.Types.LeftEdge)

        mainItem: toolTipDelegate
    }
}
