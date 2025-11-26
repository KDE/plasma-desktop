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
    enabled: !isSeparator && !disabled && (!isParent || (isParent && hasChildren))

    signal actionTriggered(string actionId, var actionArgument)
    signal aboutToShowActionMenu(var actionMenu)

    required property int index
    required property bool isSeparator
    required property bool hasChildren
    required property bool isParent
    required property bool disabled
    required property bool hasActionList
    required property string favoriteId
    required property var /*QVariantList*/ actionList
    required property url url
    required property string description
    required property string decoration
    required property string display

    property alias pressed: tapHandler.pressed
    property alias hovered: mouseArea.containsMouse
    property alias hoverEnabled: mouseArea.hoverEnabled

    readonly property bool iconAndLabelsShouldlookSelected: pressed && !hasChildren
    readonly property real fullTextWidth: Math.ceil(icon.width + label.implicitWidth + arrow.width + row.anchors.leftMargin + row.anchors.rightMargin + row.actualSpacing)

    property bool showSeparators: true
    property QtObject childDialog: null
    property ActionMenu menu: actionMenu
    property bool dialogDefaultRight: Qt.application.layoutDirection !== Qt.RightToLeft

    Accessible.role: isSeparator ? Accessible.Separator : Accessible.MenuItem
    Accessible.name: label.text

    onHasChildrenChanged: {
        if (!hasChildren && ListView.view.currentItem === item) {
            ListView.view.currentIndex = -1;
        }
    }

    onAboutToShowActionMenu: actionMenu => {
        var actionList = item.hasActionList ? item.actionList : [];
        Tools.fillActionMenu(i18n, actionMenu, actionList, ListView.view.model.favoritesModel, item.favoriteId);
    }

    onActionTriggered: (actionId, actionArgument) => {
        if (Tools.triggerAction(ListView.view.model, item.index, actionId, actionArgument) === true) {
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

    DragHandler {
        target: null
        onActiveChanged: {
            if (active && url) {
                // we need dragHelper and can't use attached Drag; submenus are destroyed too soon and Plasma crashes
                dragHelper.startDrag(kicker, url, decoration)
            }
        }
    }

    TapHandler {
        id: tapHandler
        onTapped: {
            if (!item.hasChildren) {
                item.ListView.view.model.trigger(index, "", null);
                kicker.expanded = false;
            }
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        hoverEnabled: true
        acceptedButtons: Qt.RightButton

        onPressed: mouse => {
            if (item.hasActionList || favoriteId !== null) {
                item.openActionMenu(mouseArea, mouse.x, mouse.y);
            }
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
            source: item.decoration
        }

        PlasmaComponents3.Label {
            id: label

            enabled: !item.isParent || (item.isParent && item.hasChildren)
            LayoutMirroring.enabled: (Qt.application.layoutDirection === Qt.RightToLeft)

            anchors.verticalCenter: parent.verticalCenter

            width: parent.width - icon.width - arrow.width - parent.actualSpacing

            verticalAlignment: Text.AlignVCenter

            textFormat: Text.PlainText
            wrapMode: Text.NoWrap
            elide: Text.ElideRight
            color: item.iconAndLabelsShouldlookSelected ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor

            text: item.display ?? ""
        }

        Kirigami.Icon {
            id: arrow

            anchors.verticalCenter: parent.verticalCenter

            width: visible ? Kirigami.Units.iconSizes.small : 0
            height: width

            visible: item.hasChildren
            opacity: (item.ListView.view.currentIndex === item.index) ? 1.0 : 0.4
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
        active: item.isSeparator && item.showSeparators

        asynchronous: false
        sourceComponent: separatorComponent
    }

    Keys.onPressed: event => {
        if (event.key === Qt.Key_Menu && (item.hasActionList || model.favoriteId !== null)) {
            event.accepted = true;
            item.openActionMenu(mouseArea);
        } else if ((event.key === Qt.Key_Enter || event.key === Qt.Key_Return) && !item.hasChildren) {
            if (!item.hasChildren) {
                event.accepted = true;
                item.ListView.view.model.trigger(item.index, "", null);
                kicker.expanded = false;
            }
        }
    }
}
