/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore

import "code/tools.js" as Tools

Item {
    id: item

    height: isSeparator ? separatorHeight : itemHeight
    width: ListView.view.width

    // if it's not disabled and is either a leaf node or a node with children
    enabled: !isSeparator && !model.disabled && (!isParent || (isParent && hasChildren))

    signal actionTriggered(string actionId, variant actionArgument)
    signal aboutToShowActionMenu(variant actionMenu)

    readonly property real fullTextWidth: Math.ceil(icon.width + label.implicitWidth + arrow.width + row.anchors.leftMargin + row.anchors.rightMargin + row.actualSpacing)
    property bool isSeparator: (model.isSeparator === true)
    property bool hasChildren: (model.hasChildren === true)
    property bool hasActionList: ((model.favoriteId !== null)
        || (("hasActionList" in model) && (model.hasActionList === true)))
    property QtObject childDialog: null
    property Item menu: actionMenu

    Accessible.role: isSeparator ? Accessible.Separator: Accessible.MenuItem
    Accessible.name: label.text

    onHasChildrenChanged: {
        if (!hasChildren && ListView.view.currentItem === item) {
            ListView.view.currentIndex = -1;
        }
    }

    onAboutToShowActionMenu: {
        var actionList = item.hasActionList ? model.actionList : [];
        Tools.fillActionMenu(i18n, actionMenu, actionList, ListView.view.model.favoritesModel, model.favoriteId);
    }

    onActionTriggered: {
        if (Tools.triggerAction(ListView.view.model, model.index, actionId, actionArgument) === true) {
            Plasmoid.expanded = false;
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
            item.actionTriggered(actionId, actionArgument);
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

        onPressed: mouse => {
            if (mouse.buttons & Qt.RightButton) {
                if (item.hasActionList) {
                    item.openActionMenu(mouseArea, mouse.x, mouse.y);
                }
            } else {
                pressed = true;
                pressX = mouse.x;
                pressY = mouse.y;
            }
        }

        onReleased: mouse => {
            if (pressed && !item.hasChildren) {
                item.ListView.view.model.trigger(index, "", null);
                Plasmoid.expanded = false;
            }

            pressed = false;
            pressX = -1;
            pressY = -1;
        }

        onPositionChanged: mouse => {
            if (pressX !== -1 && model.url && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y)) {
                dragHelper.startDrag(kicker, model.url, model.decoration);
                pressed = false;
                pressX = -1;
                pressY = -1;

                return;
            }

            // FIXME: Correct escape angle calc for right screen edge.
            if (justOpenedTimer.running || !item.hasChildren) {
                item.ListView.view.currentIndex = index;
            } else {
                mouseCol = mouse.x;

                if (index === item.ListView.view.currentIndex) {
                    updateCurrentItem();
                } else if ((index === item.ListView.view.currentIndex - 1) && mouse.y < (itemHeight - 6)
                    || (index === item.ListView.view.currentIndex + 1) && mouse.y > 5) {

                    if ((item.childDialog && item.childDialog.facingLeft)
                        ? mouse.x > item.ListView.view.eligibleWidth - 5 : mouse.x < item.ListView.view.eligibleWidth + 5) {
                        updateCurrentItem();
                    }
                } else if ((item.childDialog && item.childDialog.facingLeft)
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
        id: row

        anchors.left: parent.left
        anchors.leftMargin: highlightItemSvg.margins.left
        anchors.right: parent.right
        anchors.rightMargin: highlightItemSvg.margins.right

        height: parent.height

        spacing: PlasmaCore.Units.smallSpacing * 2
        readonly property real actualSpacing: ((icon.visible ? 1 : 0) * spacing) + ((arrow.visible ? 1 : 0) * spacing)

        LayoutMirroring.enabled: (Qt.application.layoutDirection === Qt.RightToLeft)

        PlasmaCore.IconItem {
            id: icon

            anchors.verticalCenter: parent.verticalCenter

            width: visible ? PlasmaCore.Units.iconSizes.small : 0
            height: width

            visible: iconsEnabled

            animated: false
            usesPlasmaTheme: false

            source: model.decoration
        }

        PlasmaComponents.Label {
            id: label

            enabled: !isParent || (isParent && item.hasChildren)

            anchors.verticalCenter: parent.verticalCenter

            width: parent.width - icon.width - arrow.width - parent.actualSpacing

            verticalAlignment: Text.AlignVCenter

            textFormat: Text.PlainText
            wrapMode: Text.NoWrap
            elide: Text.ElideRight

            text: model.display
        }

        PlasmaCore.SvgItem {
            id: arrow

            anchors.verticalCenter: parent.verticalCenter

            width: visible ? PlasmaCore.Units.iconSizes.small : 0
            height: width

            visible: item.hasChildren
            opacity: (item.ListView.view.currentIndex === index) ? 1.0 : 0.4

            svg: arrows
            elementId: (Qt.application.layoutDirection === Qt.RightToLeft) ? "left-arrow" : "right-arrow"
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

        active: item.isSeparator

        asynchronous: false
        sourceComponent: separatorComponent
    }

    Keys.onPressed: {
        if (event.key === Qt.Key_Menu && item.hasActionList) {
            event.accepted = true;
            item.openActionMenu(mouseArea);
        } else if ((event.key === Qt.Key_Enter || event.key === Qt.Key_Return) && !item.hasChildren) {
            if (!item.hasChildren) {
                event.accepted = true;
                item.ListView.view.model.trigger(index, "", null);
                Plasmoid.expanded = false;
            }
        }
    }
}
