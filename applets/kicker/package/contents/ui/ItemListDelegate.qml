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
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

import "../code/tools.js" as Tools

Item {
    id: item

    height: isSeparator ? separatorHeight : itemHeight
    width: ListView.view.width

    enabled: !isSeparator

    signal actionTriggered(string actionId, variant actionArgument)
    signal aboutToShowActionMenu(variant actionMenu)

    property bool isSeparator: (model.isSeparator == true)
    property bool hasChildren: (model.hasChildren == true)
    property bool hasActionList: ((model.favoriteId != null)
        || (("hasActionList" in model) && (model.hasActionList == true)))
    property QtObject childDialog: null
    property Item menu: actionMenu

    Accessible.role: isSeparator ? Accessible.Separator: Accessible.MenuItem
    Accessible.name: label.text

    onHasChildrenChanged: {
        if (!hasChildren && ListView.view.currentItem == item) {
            ListView.view.currentIndex = -1;
        }
    }

    onAboutToShowActionMenu: {
        var actionList = hasActionList ? model.actionList : [];
        Tools.fillActionMenu(actionMenu, actionList, ListView.view.model.favoritesModel, model.favoriteId);
    }

    onActionTriggered: {
        Tools.triggerAction(ListView.view.model, model.index, actionId, actionArgument);
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

    MouseArea {
        id: mouseArea

        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
        }

        height: parent.height

        property int mouseCol
        property bool pressed: false
        property int pressX: -1
        property int pressY: -1

        hoverEnabled: true
        acceptedButtons: Qt.LeftButton | Qt.RightButton

        onPressed: {
            if (mouse.buttons & Qt.RightButton) {
                if (hasActionList) {
                    openActionMenu(mouseArea, mouse.x, mouse.y);
                }
            } else {
                pressed = true;
                pressX = mouse.x;
                pressY = mouse.y;
            }
        }

        onReleased: {
            if (pressed && !hasChildren) {
                item.ListView.view.model.trigger(index, "", null);
                plasmoid.expanded = false;
            }

            pressed = false;
            pressX = -1;
            pressY = -1;
        }

        onPositionChanged: {
            if (pressX != -1 && model.url && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y)) {
                dragHelper.startDrag(kicker, model.url);
                pressX = -1;
                pressY = -1;

                return;
            }

            // FIXME: Correct escape angle calc for right screen edge.
            if (justOpenedTimer.running || !hasChildren) {
                item.ListView.view.currentIndex = index;
            } else {
                mouseCol = mouse.x;

                if (index == item.ListView.view.currentIndex) {
                    updateCurrentItem();
                } else if ((index == item.ListView.view.currentIndex - 1) && mouse.y < (itemHeight - 6)
                    || (index == item.ListView.view.currentIndex + 1) && mouse.y > 5) {

                    if ((childDialog != null && childDialog.facingLeft)
                        ? mouse.x > item.ListView.view.eligibleWidth - 5 : mouse.x < item.ListView.view.eligibleWidth + 5) {
                        updateCurrentItem();
                    }
                } else if ((childDialog != null && childDialog.facingLeft)
                    ? mouse.x > item.ListView.view.eligibleWidth : mouse.x < item.ListView.view.eligibleWidth) {
                    updateCurrentItem();
                }

                updateCurrentItemTimer.start();
            }
        }

        onContainsMouseChanged: {
            if (!containsMouse) {
                pressed = false;
                pressX = -1;
                pressY = -1;
                updateCurrentItemTimer.stop();
            }
        }

        function updateCurrentItem() {
            item.ListView.view.currentIndex = index;
            item.ListView.view.eligibleWidth = Math.min(width, mouseCol);
        }

        Timer {
            id: updateCurrentItemTimer

            interval: 50
            repeat: false

            onTriggered: parent.updateCurrentItem()
        }
    }

    Row {
        anchors.left: parent.left
        anchors.leftMargin: highlightItemSvg.margins.left
        anchors.right: parent.right
        anchors.rightMargin: highlightItemSvg.margins.right

        height: parent.height

        spacing: units.smallSpacing * 2

        LayoutMirroring.enabled: (Qt.application.layoutDirection == Qt.RightToLeft)

        QIconItem {
            id: icon

            anchors.verticalCenter: parent.verticalCenter

            width: visible ? units.iconSizes.small : 0
            height: width

            visible: iconsEnabled

            icon: model.decoration
        }

        PlasmaComponents.Label {
            id: label

            enabled: !isParent || (isParent && hasChildren)

            anchors.verticalCenter: parent.verticalCenter

            width: (parent.width - icon.width - arrow.width
                - ((icon.visible ? 1 : 0) * parent.spacing) - ((arrow.visible ? 1 : 0) * parent.spacing))

            verticalAlignment: Text.AlignVCenter

            textFormat: Text.PlainText
            wrapMode: Text.NoWrap
            elide: Text.ElideRight

            text: model.display
        }

        PlasmaCore.SvgItem {
            id: arrow

            anchors.verticalCenter: parent.verticalCenter

            width: visible ? units.iconSizes.small : 0
            height: width

            visible: (item.ListView.view.currentIndex == index && hasChildren)

            svg: arrows
            elementId: (Qt.application.layoutDirection == Qt.RightToLeft) ? "left-arrow" : "right-arrow"
        }
    }

    Component {
        id: separatorComponent

        PlasmaCore.SvgItem {
            width: parent.width
            height: lineSvg.horLineHeight

            svg: lineSvg
            elementId: "horizontal-line"
        }
    }

    Loader {
        id: separatorLoader

        anchors.left: parent.left
        anchors.leftMargin: highlightItemSvg.margins.left
        anchors.right: parent.right
        anchors.rightMargin: highlightItemSvg.margins.right
        anchors.verticalCenter: parent.verticalCenter

        active: isSeparator

        asynchronous: false
        sourceComponent: separatorComponent
    }

    Keys.onPressed: {
        if (event.key == Qt.Key_Menu && hasActionList) {
            event.accepted = true;
            openActionMenu(mouseArea);
        } else if ((event.key == Qt.Key_Enter || event.key == Qt.Key_Return) && !hasChildren) {
            if (!hasChildren) {
                event.accepted = true;
                item.ListView.view.model.trigger(index, "", null);
                plasmoid.expanded = false;
            }
        }
    }
}
