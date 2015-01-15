/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright 2014 Sebastian Kügler <sebas@kde.org>

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
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kquickcontrolsaddons 2.0
import org.kde.draganddrop 2.0

Item {
    id: listItem

    width: ListView.view.width
    height: (units.smallSpacing * 2) + elementIcon.height

    property bool dropEnabled: false
    property bool modelChildren: hasModelChildren
    property bool appView: false
    property bool isCurrent: listItem.ListView.view.currentIndex === index;
    property string url: model["url"]

    function activate() {
        if (hasModelChildren) {
            listItem.ListView.view.addBreadcrumb(listItem.ListView.view.model.modelIndex(index), display);
            listItem.ListView.view.model.rootIndex = listItem.ListView.view.model.modelIndex(index);
        } else {
            launcher.openUrl(model["url"]);
            plasmoid.expanded = false;

            var view = listItem.ListView.view;
            if (view.model.hasOwnProperty("rootIndex")) { // only VisualDataModel has that
                view.model.rootIndex = 0;
            }
            if (view.clearBreadCrumbs) {
                view.clearBreadCrumbs();
            }
        }
    }

    function openContextMenu() {
        print("Opening context menu for " + model["url"]);
        contextMenu.visualParent = mouseArea;
        contextMenu.openAt(display, model, listItem.width / 4, listItem.height/2);
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

            hoverEnabled: true
            acceptedButtons: Qt.LeftButton | Qt.RightButton

            onEntered: {
                listItem.ListView.view.currentIndex = index;
            }

            onExited: {
                listItem.ListView.view.currentIndex = -1;
            }

            onClicked: {
                if (mouse.button == Qt.LeftButton) {
                    if (appView) {
                        appViewScrollArea.state = "OutgoingLeft";
                    } else {
                        listItem.activate();
                    }
                    listItem.ListView.view.currentIndex = -1;
                } else if (mouse.button == Qt.RightButton) {
                    // don't show a context menu for container
                    if (hasModelChildren || typeof(contextMenu) === "undefined") {
                        return;
                    }

                    contextMenu.visualParent = mouseArea
                    contextMenu.openAt(titleElement.text, model, mouse.x, mouse.y);
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

            source: model.iconName
        }

        PlasmaComponents.Label {
            id: titleElement

            y: Math.round((parent.height - titleElement.height - ( (subTitleElement.text != "") ? subTitleElement.paintedHeight : 0) ) / 2)
            anchors {
                //bottom: elementIcon.verticalCenter
                left: elementIcon.right
                right: parent.right
                leftMargin: units.gridUnit
                rightMargin: units.gridUnit * 2
            }
            height: paintedHeight
            // TODO: games should always show the by name!
            text: {
                if (hasModelChildren) {
                    return display;
                } else {
                    plasmoid.configuration.showAppsByName || display.length == 0 ? (subtitle == undefined ? display : subtitle) :
                                                                    display
                }
            }
            elide: Text.ElideRight
        }
        PlasmaComponents.Label {
            id: subTitleElement

            anchors {
                left: titleElement.left
                right: parent.right
                rightMargin: units.gridUnit * 2
                top: titleElement.bottom
            }
            height: paintedHeight

            text: {
                if (hasModelChildren) {
                    return subtitle;
                } else {
                    plasmoid.configuration.showAppsByName || subtitle == undefined ? (display.length != 0 ? display : subtitle) : subtitle;
                }
            }
            opacity: isCurrent ? 0.6 : 0.3
            font.pointSize: theme.smallestFont.pointSize
            elide: Text.ElideMiddle
        }
    } // listItemDelegate
} // listItem
