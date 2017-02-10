/***************************************************************************
 *   Copyright (C) 2014-2015 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.4
import QtGraphicalEffects 1.0

import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0

Item {
    id: main

    property int index: model.index
    property string name: model.blank ? "" : model.display
    property bool blank: model.blank
    property bool isDir: loader.item ? loader.item.isDir : false
    property QtObject popupDialog: loader.item ? loader.item.popupDialog : null
    property Item iconArea: loader.item ? loader.item.iconArea : null
    property Item label: loader.item ? loader.item.label : null
    property Item labelArea: loader.item ? loader.item.labelArea : null
    property Item actionsOverlay: loader.item ? loader.item.actionsOverlay : null
    property Item hoverArea: loader.item ? loader.item.hoverArea : null
    property Item toolTip: loader.item ? loader.item.toolTip : null

    function openPopup() {
        if (isDir) {
            loader.item.openPopup();
        }
    }

    Loader {
        id: loader

        anchors.fill: parent

        visible: status === Loader.Ready

        active: !model.blank

        sourceComponent: delegateImplementation

        asynchronous: true
    }

    Component {
        id: delegateImplementation

        Item {
            id: impl

            anchors.fill: parent

            property bool blank: model.blank
            property bool selected: model.blank ? false : model.selected
            property bool isDir: model.blank ? false : model.isDir
            property bool hovered: (main.GridView.view.hoveredItem == main)
            property QtObject popupDialog: null
            property Item iconArea: icon
            property Item label: label
            property Item labelArea: frameLoader.textShadow || label
            property Item actionsOverlay: actions
            property Item hoverArea: toolTip
            property Item toolTip: toolTip
            property Item selectionButton: null
            property Item popupButton: null

            onSelectedChanged: {
                if (selected && !blank) {
                    frameLoader.grabToImage(function(result) {
                        dir.addItemDragImage(positioner.map(index), main.x + frameLoader.x, main.y + frameLoader.y, frameLoader.width, frameLoader.height, result.image);
                    });
                }
            }

            onHoveredChanged: {
                if (hovered) {
                    if (plasmoid.configuration.selectionMarkers && Qt.styleHints.singleClickActivation) {
                        selectionButton = selectionButtonComponent.createObject(actions);
                    }

                    if (model.isDir) {
                        if (!main.GridView.view.isRootView || root.containsDrag) {
                            hoverActivateTimer.restart();
                        }

                        if (plasmoid.configuration.popups && !root.useListViewMode) {
                            popupButton = popupButtonComponent.createObject(actions);
                        }
                    }
                } else if (!hovered) {
                    hoverActivateTimer.stop();

                    if (popupDialog != null) {
                        popupDialog.requestDestroy();
                        popupDialog = null;
                    }

                    if (selectionButton) {
                        selectionButton.destroy();
                        selectionButton = null;
                    }

                    if (popupButton) {
                        popupButton.destroy();
                        popupButton = null;
                    }
                }
            }

            function openPopup() {
                if (folderViewDialogComponent.status == Component.Ready) {
                    impl.popupDialog = folderViewDialogComponent.createObject(impl);
                    impl.popupDialog.visualParent = icon;
                    impl.popupDialog.url = model.linkDestinationUrl;
                    impl.popupDialog.visible = true;
                }
            }

            PlasmaCore.ToolTipArea {
                id: toolTip

                x: frameLoader.x + Math.min(icon.x, label.x)
                y: frameLoader.y + icon.y

                width: Math.max(icon.width, label.width)
                height: (label.y + label.paintedHeight)

                active: (plasmoid.configuration.toolTips && popupDialog == null && !model.blank)
                interactive: false
                location: root.useListViewMode ? (plasmoid.location == PlasmaCore.Types.LeftEdge ? PlasmaCore.Types.LeftEdge : PlasmaCore.Types.RightEdge) : plasmoid.location

                onContainsMouseChanged:  {
                    if (containsMouse && !model.blank) {
                        toolTip.icon = model.decoration;
                        toolTip.mainText = model.display;

                        if (model.size != undefined) {
                                toolTip.subText = model.type + "\n" + model.size;
                        } else {
                            toolTip.subText = model.type;
                        }

                        main.GridView.view.hoveredItem = main;
                    }
                }
            }

            Loader {
                id: frameLoader

                x: root.useListViewMode ? 0 : units.smallSpacing
                y: root.useListViewMode ? 0 : units.smallSpacing

                property Item textShadow: null
                property string prefix: ""

                sourceComponent: frameComponent
                active: state !== ""
                asynchronous: true

                width: {
                    if (root.useListViewMode) {
                        if (main.GridView.view.overflowing) {
                            return parent.width - units.smallSpacing;
                        } else {
                            return parent.width;
                        }
                    }

                    return parent.width - (units.smallSpacing * 2);
                }

                height: {
                    if (root.useListViewMode) {
                        return parent.height;
                    }

                    return (icon.height + (2 * units.smallSpacing) + (label.lineCount
                    * theme.mSize(theme.defaultFont).height) + (2 * units.largeSpacing));
                }

                PlasmaCore.IconItem {
                    id: icon

                    z: 2

                    states: [
                        State { // icon view
                            when: !root.useListViewMode

                            AnchorChanges {
                                target: icon
                                anchors.top: parent.top
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                        },
                        State { // list view
                            when: root.useListViewMode

                            AnchorChanges {
                                target: icon
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                            }
                        }
                    ]

                    anchors {
                        topMargin: units.largeSpacing
                        leftMargin: units.smallSpacing
                    }

                    width: root.useListViewMode ? main.GridView.view.iconSize : (parent.width - 2 * units.smallSpacing)
                    height: main.GridView.view.iconSize

                    opacity: root.useListViewMode && selectionButton ? 0.3 : 1.0

                    animated: false
                    usesPlasmaTheme: false

                    source: model.decoration
                    overlays: model.overlays
                }

                PlasmaComponents.Label {
                    id: label

                    z: 2 // So we can position a textShadowComponent below if needed.

                    states: [
                        State { // icon view
                            when: !root.useListViewMode

                            AnchorChanges {
                                target: label
                                anchors.top: icon.bottom
                                anchors.horizontalCenter: parent.horizontalCenter
                            }
                            PropertyChanges {
                                target: label
                                anchors.topMargin: 2 * units.smallSpacing
                                width: Math.min(label.implicitWidth + units.smallSpacing, parent.width - units.smallSpacing * 8)
                                maximumLineCount: plasmoid.configuration.textLines
                                horizontalAlignment: Text.AlignHCenter
                            }
                        },
                        State { // list view
                            when: root.useListViewMode

                            AnchorChanges {
                                target: label
                                anchors.left: icon.right
                                anchors.verticalCenter: parent.verticalCenter
                            }
                            PropertyChanges {
                                target: label
                                anchors.leftMargin: units.smallSpacing * 2
                                anchors.rightMargin: units.smallSpacing * 2
                                width: parent.width - icon.width - (units.smallSpacing * 4)
                                maximumLineCount: 1
                                horizontalAlignment: Text.AlignLeft
                            }
                        }
                    ]

                    height: undefined // Unset PlasmaComponents.Label's default.

                    textFormat: Text.PlainText

                    wrapMode: Text.Wrap
                    elide: Text.ElideRight

                    color: (frameLoader.textShadow && frameLoader.textShadow.visible
                        ? "#fff" : PlasmaCore.ColorScope.textColor)

                    text: model.blank ? "" : model.display

                    font.italic: model.isLink

                    Component.onCompleted: textFix.disableMouseHandling(label) // FIXME TODO: See https://codereview.qt-project.org/#/c/113758/
                }

                Column {
                    id: actions

                    visible: {
                        if (main.GridView.view.isRootView && root.containsDrag) {
                            return false;
                        }

                        if (!main.GridView.view.isRootView && dialog.containsDrag) {
                            return false;
                        }

                        if (popupDialog) {
                            return false;
                        }

                        return true;
                    }

                    x: units.smallSpacing * 3
                    y: units.smallSpacing * 3

                    z: 3

                    anchors {
                        centerIn: root.useListViewMode ? icon : undefined
                    }

                    width: implicitWidth
                    height: implicitHeight
                }

                Component {
                    id: frameComponent

                    PlasmaCore.FrameSvgItem {
                        prefix: frameLoader.prefix

                        imagePath: "widgets/viewitem"
                    }
                }

                Component {
                    id: selectionButtonComponent

                    FolderItemActionButton {
                        element: model.selected ? "remove" : "add"

                        onClicked: dir.toggleSelected(positioner.map(index))
                    }
                }

                Component {
                    id: popupButtonComponent

                    FolderItemActionButton {
                        visible: popupDialog == null

                        element: "open"

                        onClicked: openPopup()
                    }
                }

                Component {
                    id: textShadowComponent

                    DropShadow {
                        anchors.fill: label

                        z: 1

                        horizontalOffset: 2
                        verticalOffset: 2

                        radius: 9.0
                        samples: 18
                        spread: 0.15

                        color: "black"

                        source: label
                    }
                }

                states: [
                    State {
                        name: "selected"
                        when: model.selected

                        PropertyChanges {
                            target: frameLoader
                            prefix: "selected"
                        }
                    },
                    State {
                        name: "hover"
                        when: hovered && !model.selected

                        PropertyChanges {
                            target: frameLoader
                            prefix: "hover"
                        }
                    },
                    State {
                        name: "selected+hover"
                        when: hovered && model.selected

                        PropertyChanges {
                            target: frameLoader
                            prefix: "selected+hover"
                        }
                    }
                ]
            }

            Component.onCompleted: {
                if (root.isContainment && main.GridView.view.isRootView) {
                    frameLoader.textShadow = textShadowComponent.createObject(frameLoader);
                }
            }
        }
    }
}
