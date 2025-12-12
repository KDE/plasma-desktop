/*
    SPDX-FileCopyrightText: 2015 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents3
import org.kde.plasma.core as PlasmaCore

import "code/tools.js" as Tools

Item {
    id: item

    required property int index
    required property bool disabled
    required property bool hasActionList
    required property string favoriteId
    required property var /*QVariantList*/ actionList
    required property url url
    required property string description
    required property string decoration
    required property var model // for display, in case we port this to ItemDelegate

    property bool showLabel: true
    property int itemIndex: item.index
    property var icon: item.decoration !== undefined ? item.decoration : ""
    property var m: model

    signal interactionConcluded()

    width: GridView.view.cellWidth
    height: width

    enabled: !item.disabled

    Accessible.role: Accessible.MenuItem
    Accessible.name: model.display ?? ""

    function openActionMenu(x, y) {
        var actionList = hasActionList ? item.actionList : [];
        Tools.fillActionMenu(i18n, actionMenu, actionList, GridView.view.model.favoritesModel, item.favoriteId);
        actionMenu.visualParent = item;
        actionMenu.open(x, y);
    }

    function actionTriggered(actionId, actionArgument) {
        var close = (Tools.triggerAction(GridView.view.model, item.index, actionId, actionArgument) === true);

        if (close) {
            item.interactionConcluded()
        }
    }

    Kirigami.Icon {
        id: icon

        y: item.showLabel ? (2 * highlightItemSvg.margins.top) : undefined

        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: item.showLabel ? undefined : parent.verticalCenter

        width: iconSize
        height: width

        animated: false

        source: item.decoration
    }

    PlasmaComponents3.Label {
        id: label

        visible: item.showLabel

        anchors {
            top: icon.bottom
            topMargin: Kirigami.Units.smallSpacing
            left: parent.left
            leftMargin: highlightItemSvg.margins.left
            right: parent.right
            rightMargin: highlightItemSvg.margins.right
        }

        horizontalAlignment: Text.AlignHCenter

        maximumLineCount: 2
        elide: Text.ElideMiddle
        wrapMode: Text.Wrap

        text: item.model.display ?? ""
        textFormat: Text.PlainText
    }

    PlasmaCore.ToolTipArea {
        id: toolTip

        mainText: item.description ?? ""

        anchors.fill: parent
        mainItem: toolTipDelegate

        onContainsMouseChanged: item.GridView.view.itemContainsMouseChanged(containsMouse)
    }

    Keys.onMenuPressed: {
        if (item.hasActionList || item.favoriteId !== null) {
            item.openActionMenu(item)
        }
    }
    Keys.onEnterPressed: Keys.returnPressed()
    Keys.onReturnPressed: {
        if ("trigger" in GridView.view.model) {
            GridView.view.model.trigger(index, "", null);
            item.interactionConcluded()
        }

        itemGrid.itemActivated(index, "", null);
    }
}
