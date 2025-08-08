/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.components as PlasmaComponents3

import "code/tools.js" as Tools

Item {
    id: item

    height: isSeparator ? separatorHeight : itemHeight
    width: ListView.view.width

    // if it's not disabled and is either a leaf node or a node with children
    enabled: !isSeparator && !model.disabled && (!isParent || (isParent && hasChildren))

    signal actionTriggered(string actionId, var actionArgument)
    signal aboutToShowActionMenu(var actionMenu)

    readonly property real fullTextWidth: Math.ceil(icon.width + label.implicitWidth + arrow.width + row.anchors.leftMargin + row.anchors.rightMargin + row.actualSpacing)
    property bool isSeparator: (model.isSeparator === true)
    property bool sorted: (model.sorted === true)
    property bool hasChildren: (model.hasChildren === true)
    property bool hasActionList: ((model.favoriteId !== null)
        || (("hasActionList" in model) && (model.hasActionList === true)))
    property QtObject childDialog: null
    property ActionMenu menu: actionMenu
    property bool dialogDefaultRight: Qt.application.layoutDirection !== Qt.RightToLeft
    readonly property bool pressed: mouseArea.pressed
    readonly property bool iconAndLabelsShouldlookSelected: mouseArea.pressed && !hasChildren
    readonly property alias hovered: mouseArea.containsMouse

    Accessible.role: isSeparator ? Accessible.Separator : Accessible.MenuItem
    Accessible.name: label.text

    onHasChildrenChanged: {
        if (!hasChildren && ListView.view.currentItem === item) {
            ListView.view.currentIndex = -1;
        }
    }

    onAboutToShowActionMenu: actionMenu => {
        var actionList = item.hasActionList ? model.actionList : [];
        Tools.fillActionMenu(i18n, actionMenu, actionList, ListView.view.model.favoritesModel, model.favoriteId);
    }

    onActionTriggered: (actionId, actionArgument) => {
        if (Tools.triggerAction(ListView.view.model, model.index, actionId, actionArgument) === true) {
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
        property bool mousePressed: false
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
                mousePressed = true;
                pressX = mouse.x;
                pressY = mouse.y;
            }
        }

        onReleased: mouse => {
            if (mousePressed && !item.hasChildren) {
                item.ListView.view.model.trigger(index, "", null);
                kicker.expanded = false;
            }

            mousePressed = false;
            pressX = -1;
            pressY = -1;
        }

        onPositionChanged: mouse => {
            if (!item.ListView.view.mouseMoved) {
                item.ListView.view.mouseMoved = true;
                return;
            }
            if (pressX !== -1 && model.url && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y)) {
                dragHelper.startDrag(kicker, model.url, model.decoration);
                mousePressed = false;
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
                mousePressed = false;
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

        spacing: Kirigami.Units.smallSpacing * 2
        readonly property real actualSpacing: ((icon.visible ? 1 : 0) * spacing) + ((arrow.visible ? 1 : 0) * spacing)

        LayoutMirroring.enabled: (Qt.application.layoutDirection === Qt.RightToLeft)

        Kirigami.Icon {
            id: icon

            anchors.verticalCenter: parent.verticalCenter

            width: visible ? Kirigami.Units.iconSizes.small : 0
            height: width

            visible: iconsEnabled

            animated: false
            selected: item.iconAndLabelsShouldlookSelected
            source: model.decoration
        }

        PlasmaComponents3.Label {
            id: label

            enabled: !isParent || (isParent && item.hasChildren)

            anchors.verticalCenter: parent.verticalCenter

            width: parent.width - icon.width - arrow.width - parent.actualSpacing

            verticalAlignment: Text.AlignVCenter

            textFormat: Text.PlainText
            wrapMode: Text.NoWrap
            elide: Text.ElideRight
            color: item.iconAndLabelsShouldlookSelected ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor

            text: model?.display ?? ""
        }

        Kirigami.Icon {
            id: arrow

            anchors.verticalCenter: parent.verticalCenter

            width: visible ? Kirigami.Units.iconSizes.small : 0
            height: width

            visible: item.hasChildren
            opacity: (item.ListView.view.currentIndex === index) ? 1.0 : 0.4
            selected: item.iconAndLabelsShouldlookSelected
            source: item.dialogDefaultRight
                ? "go-next-symbolic"
                : "go-next-rtl-symbolic"
        }
    }

    Component {
        id: separatorComponent

        KSvg.SvgItem {
            width: parent.width

            imagePath: "widgets/line"
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

        // Separator positions don't make sense when sorting everything alphabetically
        active: item.isSeparator && !item.sorted

        asynchronous: false
        sourceComponent: separatorComponent
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Menu && item.hasActionList) {
            event.accepted = true;
            item.openActionMenu(mouseArea);
        } else if ((event.key === Qt.Key_Enter || event.key === Qt.Key_Return) && !item.hasChildren) {
            if (!item.hasChildren) {
                event.accepted = true;
                item.ListView.view.model.trigger(index, "", null);
                kicker.expanded = false;
            }
        }
    }
}
