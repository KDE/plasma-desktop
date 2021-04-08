/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright 2014 Sebastian Kügler <sebas@kde.org>
    Copyright (C) 2015-2018  Eike Hein <hein@kde.org>
    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3

import "code/tools.js" as Tools

Item {
    id: listItem

    enabled: !model.disabled

    width: ListView.view.width
    height: (PlasmaCore.Units.smallSpacing * 2) + Math.max(elementIcon.height+ PlasmaCore.Units.smallSpacing*2, titleElement.implicitHeight + PlasmaCore.Units.smallSpacing*2)

    signal reset
    signal actionTriggered(string actionId, variant actionArgument)
    signal aboutToShowActionMenu(variant actionMenu)
    signal addBreadcrumb(var model, string title)

    readonly property int itemIndex: model.index
    readonly property string url: model.url || ""
    readonly property var decoration: model.decoration || ""

    property bool appView: false
    property bool modelChildren: model.hasChildren || false
    property bool isCurrent: ListView.view.currentIndex === index;

    property bool hasActionList: ((model.favoriteId !== null)
        || (("hasActionList" in model) && (model.hasActionList === true)))

    // left sidebar app list
    property bool isManagerMode: false

    // left sidebar places list
    property bool managesChildrenOutside: model.managesChildrenOutside || false

    // Report list count when we first focus on an item (see more thorough explanation in KickoffListView.qml)
    Accessible.role: Accessible.MenuItem
    Accessible.name: ListView.view.accessibilityCount ? i18np("List with 1 item", "List with %1 items", ListView.view.count) : accessibleName
    Accessible.description: ListView.view.accessibilityCount ? accessibleName : ""

    property string displayName: managesChildrenOutside ? ListView.view.model.getI18nName(index) : model.display
    property string accessibleName: modelChildren || managesChildrenOutside ? i18n("%1 submenu", displayName) : displayName

    onAboutToShowActionMenu: {
        var actionList = hasActionList ? model.actionList : [];
        Tools.fillActionMenu(i18n, actionMenu, actionList, ListView.view.model.favoritesModel, model.favoriteId);
    }

    onActionTriggered: {
        if (Tools.triggerAction(ListView.view.model, model.index, actionId, actionArgument) === true) {
            if (!listItem.managesChildrenOutside) {
                plasmoid.expanded = false;
            }
        }

        if (actionId.indexOf("_kicker_favorite_") === 0) {
            switchToInitial();
        }
    }

    function activate() {
        var view = listItem.ListView.view;

        if (model.hasChildren) {
            var childModel = view.model.modelForRow(index);
            if (listItem.isManagerMode) {
                return {model : childModel, name : display};
            } else {
                listItem.addBreadcrumb(childModel, display);
                view.model = childModel;
            }
        } else {
            view.model.trigger(index, "", null);
            if (!listItem.managesChildrenOutside) {
                plasmoid.expanded = false;
            }
            listItem.reset();
        }
    }

    function openActionMenu(x, y) {
        aboutToShowActionMenu(actionMenu);
        actionMenu.visualParent = listItem;
        actionMenu.open(x, y);
    }

    ActionMenu {
        id: actionMenu

        onActionClicked: {
            actionTriggered(actionId, actionArgument);
        }
    }

    PlasmaCore.IconItem {
        id: elementIcon

        anchors {
            left: parent.left
            leftMargin: PlasmaCore.Units.smallSpacing * 6
            verticalCenter: parent.verticalCenter
        }
        width: PlasmaCore.Units.iconSizes.smallMedium
        height: width

        animated: false
        usesPlasmaTheme: false

        source: model.decoration
    }

    PlasmaComponents3.Label {
        id: titleElement

        anchors {
            left: elementIcon.right
            right: arrow.left
            leftMargin: PlasmaCore.Units.smallSpacing * 4
            rightMargin: PlasmaCore.Units.smallSpacing * 2
            verticalCenter: parent.verticalCenter
        }
        text: listItem.displayName
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignLeft
    }

    PlasmaComponents3.Label {
        id: subTitleElement

        anchors {
            left: parent.left
            leftMargin: elementIcon.anchors.leftMargin + elementIcon.width + titleElement.anchors.leftMargin + titleElement.contentWidth + titleElement.anchors.rightMargin + PlasmaCore.Units.smallSpacing * 4
            right: arrow.right
            verticalCenter: parent.verticalCenter
        }

        text: isManagerMode ? "" : model.description || ""
        opacity: isCurrent ? 0.8 : 0.6
        font: PlasmaCore.Theme.smallestFont
        elide: Text.ElideMiddle
        horizontalAlignment: Text.AlignRight
    }

    PlasmaCore.SvgItem {
        id: arrow

        anchors {
            right: parent.right
            rightMargin: PlasmaCore.Units.smallSpacing * 6
            verticalCenter: parent.verticalCenter
        }

        width: visible ? PlasmaCore.Units.iconSizes.small : 0
        height: width

        visible: listItem.managesChildrenOutside || (model.hasChildren === true)
        opacity: (listItem.ListView.view.currentIndex === index) ? 1.0 : 0.4

        svg: PlasmaCore.Svg {
            imagePath: "widgets/arrows"
            size: "16x16"
        }
        elementId: (Qt.application.layoutDirection == Qt.RightToLeft) ? "left-arrow" : "right-arrow"
    }
} // listItem
