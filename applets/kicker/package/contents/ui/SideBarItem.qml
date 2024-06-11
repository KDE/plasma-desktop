/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.kquickcontrolsaddons as KQuickControlsAddons
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid

import "code/tools.js" as Tools

Item {
    id: item

    width: root.width
    height: root.width

    signal actionTriggered(string actionId, var actionArgument)
    signal aboutToShowActionMenu(var actionMenu)

    property bool hasActionList: ((model.favoriteId !== null)
        || (("hasActionList" in model) && (model.hasActionList !== null)))
    property int itemIndex: model.index

    onAboutToShowActionMenu: actionMenu => {
        const actionList = (model.hasActionList !== null) ? model.actionList : [];
        Tools.fillActionMenu(i18n, actionMenu, actionList, repeater.model, model.favoriteId);
    }

    onActionTriggered: (actionId, actionArgument) => {
        if (Tools.triggerAction(repeater.model, model.index, actionId, actionArgument) === true) {
            kicker.expanded = false;
        }
    }

    function openActionMenu(visualParent, x, y) {
        aboutToShowActionMenu(actionMenu);
        actionMenu.visualParent = visualParent;
        actionMenu.open(x, y);
    }

    ActionMenu {
        id: actionMenu

        onActionClicked: (actionId, actionArgument) => {
            actionTriggered(actionId, actionArgument);
        }
    }

    Kirigami.Icon {
        anchors.fill: parent

        active: toolTip.containsMouse

        source: model.decoration
    }

    KQuickControlsAddons.MouseEventListener {
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

        onPressed: mouse => {
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

        onReleased: mouse => {
            if (pressed) {
                repeater.model.trigger(index, "", null);
                kicker.expanded = false;
            }

            pressed = false;
            pressX = -1;
            pressY = -1;
        }

        onContainsMouseChanged: containsMouseChanged => {
            if (!containsMouse) {
                pressed = false;
                pressX = -1;
                pressY = -1;
            }
        }

        onPositionChanged: mouse => {
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
        location: (((Plasmoid.location === PlasmaCore.Types.RightEdge)
            || (Qt.application.layoutDirection === Qt.RightToLeft))
            ? PlasmaCore.Types.RightEdge : PlasmaCore.Types.LeftEdge)

        mainItem: toolTipDelegate
    }
}
