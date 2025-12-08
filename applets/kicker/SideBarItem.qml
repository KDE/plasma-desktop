/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PC3

import "code/tools.js" as Tools

PC3.ToolButton {
    id: item

    required property int index
    required property var model // for display, which conflicts with ToolButton
    required property bool hasActionList
    required property string favoriteId
    required property string decoration
    required property var /*QVariantList*/ actionList
    required property url url

    required property var favoritesModel

    readonly property int itemIndex: model.index

    signal interactionConcluded

    activeFocusOnTab: false

    text: model.display
    display: PC3.AbstractButton.IconOnly
    Accessible.role: Accessible.ListItem
    icon.source: item.decoration
    icon.width: Kirigami.Units.iconSizes.medium
    icon.height: Kirigami.Units.iconSizes.medium



    function activate() : void {
        if (dragHandler.active) {
            return
        }
        favoritesModel.trigger(index, "", null);
        item.interactionConcluded()
    }

    Keys.onSpacePressed: activate()
    Keys.onReturnPressed: activate()
    Keys.onEnterPressed: activate()
    onClicked: activate()

    function openActionMenu(visualParent, x, y) {
        const actionList = (item.hasActionList !== null) ? item.actionList : [];
        Tools.fillActionMenu(i18n, actionMenu, actionList, favoritesModel, item.favoriteId);
        actionMenu.visualParent = visualParent;
        actionMenu.open(x, y);
    }

    ActionMenu {
        id: actionMenu

        onActionClicked: (actionId, actionArgument) => {
            if (Tools.triggerAction(item.favoritesModel, item.index, actionId, actionArgument) === true) {
                item.interactionConcluded()
            }
        }
    }

    DragHandler {
        id: dragHandler
        target: null
        onActiveChanged: if (active) {
            item.contentItem.grabToImage(function(result) {
                item.Drag.imageSource = result.url
                item.Drag.active = true // using a binding can cause loop warnings when unexpanding
            })
        } else {
            item.Drag.active = false
        }
    }
    Drag.dragType: Drag.Automatic
    Drag.mimeData: {
        "text/uri-list" : [item.url]
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onPressed: mouse => item.openActionMenu(item, mouse.x, mouse.y)
    }

    PC3.ToolTip {
        text: item.text
    }
}
