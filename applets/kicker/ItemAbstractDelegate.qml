/*
 *    SPDX-FileCopyrightText: 2013-2015 Eike Hein <hein@kde.org>
 *
 *    SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick

import org.kde.plasma.components as PlasmaComponents3

import "code/tools.js" as Tools

PlasmaComponents3.ItemDelegate {
    id: item

    required property int index
    required property bool isSeparator
    required property bool hasChildren
    required property bool isParent
    required property bool disabled
    required property bool hasActionList
    required property bool isNewlyInstalled
    required property string favoriteId
    required property var /*QVariantList*/ actionList
    required property url url
    required property string description
    required property string decoration
    required property string compactName
    required property var model // for display, which would shadow ItemDelegate
    required property var favoritesModel
    required property var baseModel

    property bool dialogDefaultRight: Application.layoutDirection !== Qt.RightToLeft

    signal interactionConcluded

    text: model.display
    icon.name: decoration
    hoverEnabled: true

    function openActionMenu(x: real, y: real) : void {
        const actionList = item.hasActionList ? item.actionList : [];
        Tools.fillActionMenu(i18n, ActionMenu, actionList, item.favoritesModel, item.favoriteId);
        if (!ActionMenu.actionList.length) {
            return
        }
        ActionMenu.visualParent = item;
        ActionMenu.delegate = item;
        ActionMenu.open(x, y);
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        acceptedButtons: Qt.RightButton

        onPressed: mouse => {
            if (item.hasActionList || item.favoriteId !== null) {
                item.openActionMenu(mouse.x, mouse.y);
            }
        }
    }

    contentItem: null

    Keys.onMenuPressed: {
        if (item.hasActionList || item.favoriteId !== null) {
            item.openActionMenu()
        }
    }
}
