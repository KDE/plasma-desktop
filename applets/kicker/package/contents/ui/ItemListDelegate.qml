/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg

import "code/tools.js" as Tools

Item {
    id: item

    height: isSeparator ? separatorHeight : itemHeight
    width: ListView.view.width

    // if it's not disabled and is either a leaf node or a node with children
    enabled: !isSeparator && !model.disabled && (!isParent || (isParent && hasChildren))
    focus: item.ListView.isCurrentItem // Make key events handled in this item

    readonly property real fullTextWidth: Math.ceil(icon.width + label.implicitWidth + arrow.width + row.anchors.leftMargin + row.anchors.rightMargin + row.actualSpacing)
    property bool isSeparator: (model.isSeparator === true)
    property bool sorted: (model.sorted === true)
    property bool hasChildren: (model.hasChildren === true)
    property bool hasActionList: ((model.favoriteId !== null)
        || (("hasActionList" in model) && (model.hasActionList === true)))
    property Item menu: actionMenu

    Accessible.role: isSeparator ? Accessible.Separator: Accessible.MenuItem
    Accessible.name: label.text

    Keys.onMenuPressed: event => {
        if (menuHandler.enabled) {
            item.openActionMenu(item);
        } else {
            event.accepted = false;
        }
    }
    Keys.onLeftPressed: event => {
        if (item.ListView.view.dialog?.focusParent) {
            item.ListView.view.dialog.destroy();
        } else {
            event.accepted = false;
        }
    }
    Keys.onRightPressed: event => {
        if (item.hasChildren) {
            item.ListView.view.spawnDialog(true)
        } else {
            event.accepted = false;
        }
    }
    Keys.onUpPressed: event => {
        item.ListView.view.showChildDialogs = false; // Do not automatically expand the child menu
        event.accepted = false; // Pass the event to ListView
    }
    Keys.onDownPressed: event => Keys.onUpPressed(event)
    Keys.onReturnPressed: event => {
        if (item.hasChildren) {
            Keys.onRightPressed(event);
        } else {
            item.ListView.view.model.trigger(index, "", null);
            kicker.expanded = false;
        }
    }
    Keys.onEnterPressed: event => Keys.onReturnPressed(event)

    onActiveFocusChanged: if (activeFocus) {
        ListView.view.currentIndex = index;
    }

    function openActionMenu(visualParent, x = 0, y = 0) {
        const actionList = item.hasActionList ? model.actionList : [];
        Tools.fillActionMenu(i18n, actionMenu, actionList, ListView.view.model.favoritesModel, model.favoriteId);
        actionMenu.visualParent = visualParent;
        actionMenu.open(x, y);
    }

    ActionMenu {
        id: actionMenu

        onActionClicked: (actionId, actionArgument) => {
            if (Tools.triggerAction(item.ListView.view.model, model.index, actionId, actionArgument) === true) {
                kicker.expanded = false;
            }
        }
    }

    HoverHandler {
        enabled: item.enabled && !item.ListView.isCurrentItem
        onHoveredChanged: if (hovered) {
            item.ListView.view.currentIndex = index;
        }
    }

    TapHandler {
        id: actionHandler
        enabled: !item.hasChildren
        acceptedButtons: Qt.LeftButton
        onTapped: item.Keys.onReturnPressed(null)
    }

    TapHandler {
        id: menuHandler
        enabled: item.hasActionList
        acceptedButtons: Qt.RightButton
        gesturePolicy: TapHandler.WithinBounds // Release grab when menu appears
        onPressedChanged: if (pressed) {
            item.openActionMenu(item, point.position.x, point.position.y);
        }
    }

    Row {
        id: row

        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            leftMargin: highlightItemSvg.margins.left
            right: parent.right
            rightMargin: highlightItemSvg.margins.right
        }

        spacing: Kirigami.Units.smallSpacing * 2
        readonly property real actualSpacing: ((icon.visible ? 1 : 0) * spacing) + ((arrow.visible ? 1 : 0) * spacing)

        LayoutMirroring.enabled: (Qt.application.layoutDirection === Qt.RightToLeft)

        DragHandler {
            id: dragHandler
            enabled: !item.hasChildren && model.url?.toString().length > 0
            onActiveChanged: if (active) {
                icon.grabToImage((result) => {
                    if (!dragHandler.active) {
                        return;
                    }
                    dragSource.Drag.imageSource = result.url;
                    dragSource.Drag.mimeData = {
                        "text/uri-list": model.url.toString(),
                        "text/x-orgkdeplasmakicker_favoriteid": model.favoriteId,
                    };
                    dragSource.sourceItem = item;
                    dragSource.Drag.active = dragHandler.active;
                });
            } else {
                dragSource.Drag.active = false;
            }
        }

        Kirigami.Icon {
            id: icon

            anchors.verticalCenter: parent.verticalCenter

            width: visible ? Kirigami.Units.iconSizes.small : 0
            height: width

            visible: iconsEnabled

            animated: false

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

            text: model.display
        }

        Kirigami.Icon {
            id: arrow

            anchors.verticalCenter: parent.verticalCenter

            width: visible ? Kirigami.Units.iconSizes.small : 0
            height: width

            visible: item.hasChildren
            opacity: item.ListView.isCurrentItem ? 1.0 : 0.4

            source: Qt.application.layoutDirection !== Qt.RightToLeft
                ? "go-next-symbolic"
                : "go-next-rtl-symbolic"
        }
    }

    Component {
        id: separatorComponent

        KSvg.SvgItem {
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

        // Separator positions don't make sense when sorting everything alphabetically
        active: item.isSeparator && !item.sorted

        asynchronous: false
        sourceComponent: separatorComponent
    }
}
