/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright 2014 Sebastian Kügler <sebas@kde.org>
    Copyright (C) 2015  Eike Hein <hein@kde.org>

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
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.draganddrop 2.0

import "../code/tools.js" as Tools

Item {
    id: listItem

    width: ListView.view.width
//     height: listItemDelegate.height // + listItemDelegate.anchors.margins*2
    height: (units.smallSpacing * 2) + Math.max(elementIcon.height, titleElement.height + subTitleElement.height)

    signal actionTriggered(string actionId, variant actionArgument)
    signal aboutToShowActionMenu(variant actionMenu)

    readonly property int itemIndex: model.index

    property bool dropEnabled: false
    property bool appView: false
    property bool modelChildren: model.hasChildren || false
    property bool isCurrent: listItem.ListView.view.currentIndex === index;
    property string url: model.url || ""
    property bool showAppsByName: plasmoid.configuration.showAppsByName

    property bool hasActionList: ((model.favoriteId != null)
        || (("hasActionList" in model) && (model.hasActionList == true)))
    property Item menu: actionMenu

    onAboutToShowActionMenu: {
        var actionList = hasActionList ? model.actionList : [];
        Tools.fillActionMenu(actionMenu, actionList, ListView.view.model.favoritesModel, model.favoriteId);
    }

    onActionTriggered: {
        Tools.triggerAction(ListView.view.model, model.index, actionId, actionArgument);

        if (actionId.indexOf("_kicker_favorite_") === 0) {
            switchToFavorites();
        }
    }

    function activate() {
        var view = listItem.ListView.view;

        if (model.hasChildren) {
            var childModel = view.model.modelForRow(index);

            view.addBreadcrumb(childModel, display);
            view.model = childModel;
        } else {
            view.model.trigger(index, "", null);
            plasmoid.expanded = false;

            if (view.reset) {
                view.reset();
            }
        }
    }

    function openActionMenu(visualParent, x, y) {
        aboutToShowActionMenu(actionMenu);
        actionMenu.visualParent = visualParent != undefined ? visualParent : mouseArea;
        actionMenu.open(x, y);
    }

    ActionMenu {
        id: actionMenu

        onActionClicked: {
            actionTriggered(actionId, actionArgument);
        }
    }

    Item {
        id: listItemDelegate

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            bottom: parent.bottom
            //margins: units.smallSpacing
        }

        MouseArea {
            id: mouseArea

            anchors.fill: parent
            //anchors.margins: -8

            property bool pressed: false
            property int pressX: -1
            property int pressY: -1

            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onEntered: {
                listItem.ListView.view.currentIndex = index;
            }

            onExited: {
                listItem.ListView.view.currentIndex = -1;
            }

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
                if (pressed) {
                    if (appView) {
                        appViewScrollArea.state = "OutgoingLeft";
                    } else {
                        listItem.activate();
                    }

                    listItem.ListView.view.currentIndex = -1;
                }

                pressed = false;
                pressX = -1;
                pressY = -1;
            }

            onPositionChanged: {
                if (pressX != -1 && model.url && dragHelper.isDrag(pressX, pressY, mouse.x, mouse.y)) {
                    kickoff.dragSource = listItem;
                    dragHelper.startDrag(root, model.url, model.decoration);
                    pressed = false;
                    pressX = -1;
                    pressY = -1;
                }
            }

            onContainsMouseChanged: {
                if (!containsMouse) {
                    pressed = false;
                    pressX = -1;
                    pressY = -1;
                }
            }
        }

        PlasmaCore.IconItem {
            id: elementIcon

            anchors {
                left: parent.left
                leftMargin: (units.gridUnit * 4) - units.iconSizes.medium
                verticalCenter: parent.verticalCenter
            }
            width: units.iconSizes.medium
            height: width

            animated: false
            usesPlasmaTheme: false

            source: model.decoration
        }
        PlasmaComponents.Label {
            id: titleElement

            y: Math.round((parent.height - titleElement.height - ( (subTitleElement.text != "") ? subTitleElement.paintedHeight : 0) ) / 2)
            anchors {
                //bottom: elementIcon.verticalCenter
                left: elementIcon.right
                right: arrow.left
                leftMargin: units.gridUnit
                rightMargin: units.gridUnit * 2
            }
            height: paintedHeight
            // TODO: games should always show the by name!
            text: model.display
            elide: Text.ElideRight
        }
        PlasmaComponents.Label {
            id: subTitleElement

            anchors {
                left: titleElement.left
                right: arrow.left
                rightMargin: units.gridUnit * 2
                top: titleElement.bottom
            }
            height: paintedHeight

            text: model.description
            opacity: isCurrent ? 0.6 : 0.3
            font.pointSize: theme.smallestFont.pointSize
            elide: Text.ElideMiddle
        }

        PlasmaCore.SvgItem {
            id: arrow

            anchors {
                right: parent.right
                rightMargin: units.gridUnit * 2
                verticalCenter: parent.verticalCenter
            }

            width: visible ? units.iconSizes.small : 0
            height: width

            visible: (model.hasChildren == true)
            opacity: (listItem.ListView.view.currentIndex == index) ? 1.0 : 0.4

            svg: arrowsSvg
            elementId: (Qt.application.layoutDirection == Qt.RightToLeft) ? "left-arrow" : "right-arrow"
        }
    } // listItemDelegate

    Keys.onPressed: {
        if (event.key == Qt.Key_Menu && hasActionList) {
            event.accepted = true;
            openActionMenu(mouseArea);
        } else if ((event.key == Qt.Key_Enter || event.key == Qt.Key_Return) && !modelChildren) {
            if (!modelChildren) {
                event.accepted = true;
                listItem.activate();
            }
        }
    }
} // listItem
