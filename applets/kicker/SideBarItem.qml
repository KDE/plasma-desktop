/*
    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.kquickcontrolsaddons as KQuickControlsAddons
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PC3

import "code/tools.js" as Tools

PC3.ToolButton {
    id: item

    signal actionTriggered(string actionId, var actionArgument)
    signal aboutToShowActionMenu(var actionMenu)
    activeFocusOnTab: false

    text: model.display
    display: PC3.AbstractButton.IconOnly
    icon.source: model.decoration
    icon.width: Kirigami.Units.iconSizes.medium
    icon.height: Kirigami.Units.iconSizes.medium


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

    function activate() : void {
        if (dragHandler.active) {
            return
        }
        repeater.model.trigger(index, "", null);
        kicker.expanded = false;
    }

    Keys.onSpacePressed: activate()
    Keys.onReturnPressed: activate()
    Keys.onEnterPressed: activate()
    onClicked: activate()

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
        "text/uri-list" : [model.url]
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.RightButton
        onPressed: mouse => item.openActionMenu(item, mouse.x, mouse.y)
    }

    PC3.ToolTip.text: item.text
    PC3.ToolTip.visible: item,hovered
    PC3.ToolTip.delay: Kirigami.Units.toolTipDelay
}
