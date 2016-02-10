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

    Loader {
        id: loader

        anchors.fill: parent

        active: !model.blank

        sourceComponent: delegateImplementation
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
            property Item labelArea: textBackground
            property Item actionsOverlay: actions
            property Item hoverArea: toolTip
            property Item toolTip: toolTip
            property Item popupButton: null

            onSelectedChanged: {
                if (selected && !blank) {
                    frame.grabToImage(function(result) {
                        dir.addItemDragImage(positioner.map(index), main.x + frame.x, main.y + frame.y, frame.width, frame.height, result.image);
                    });
                }
            }

            onIsDirChanged: {
                if (isDir && main.GridView.view.isRootView && impl.popupButton == null) {
                    impl.popupButton = popupButtonComponent.createObject(actions);
                } else if (impl.popupButton) {
                    impl.popupButton.destroy();
                    impl.popupButton = null;
                }
            }

            onHoveredChanged: {
                if (hovered && !main.GridView.view.isRootView && model.isDir) {
                    openPopupTimer.start();
                } else if (!hovered)
                    openPopupTimer.stop();

                    if (popupDialog != null) {
                        popupDialog.destroy();
                        popupDialog = null;
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

            Timer {
                id: openPopupTimer

                interval: units.longDuration * 3

                onTriggered: {
                    impl.openPopup();
                }
            }

            PlasmaCore.ToolTipArea {
                id: toolTip

                x: frame.x + Math.min(icon.x, label.x)
                y: frame.y + icon.y

                width: Math.max(icon.width, label.width)
                height: (label.y + label.paintedHeight)

                mainItem: toolTipDelegate
                active: (plasmoid.configuration.toolTips && popupDialog == null && !model.blank)
                interactive: false
                location: root.isPopup ? (plasmoid.location == PlasmaCore.Types.LeftEdge ? PlasmaCore.Types.LeftEdge : PlasmaCore.Types.RightEdge) : plasmoid.location

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

            PlasmaCore.FrameSvgItem {
                id: frame

                x: root.isPopup ? 0 : units.smallSpacing
                y: root.isPopup ? 0 : units.smallSpacing

                width: {
                    if (root.isPopup) {
                        if (main.GridView.view.overflowing) {
                            return parent.width - units.smallSpacing;
                        } else {
                            return parent.width;
                        }
                    }

                    return parent.width - (units.smallSpacing * 2);
                }

                height: {
                    if (root.isPopup) {
                        return parent.height;
                    }

                    return (icon.height + (2 * units.smallSpacing) + (label.lineCount
                    * theme.mSize(theme.defaultFont).height) + (2 * units.largeSpacing));
                }

                visible: !model.blank
                enabled: visible

                imagePath: "widgets/viewitem"

                PlasmaCore.ColorScope {
                    anchors.fill: parent

                    colorGroup: ((root.isContainment && main.GridView.view.isRootView) ? PlasmaCore.Theme.ComplementaryColorGroup
                        : PlasmaCore.Theme.NormalColorGroup)

                    QIconItem {
                        id: icon

                        anchors {
                            top: root.isPopup ? undefined : parent.top
                            topMargin: units.largeSpacing
                            left: root.isPopup ? parent.left : undefined
                            leftMargin: units.smallSpacing
                            horizontalCenter: root.isPopup ? undefined : parent.horizontalCenter
                            verticalCenter: root.isPopup ? parent.verticalCenter : undefined
                        }

                        width: main.GridView.view.iconSize
                        height: main.GridView.view.iconSize

                        opacity: root.isPopup ? (1.3 - selectionButton.opacity) : 1.0

                        icon: model.decoration
                    }

                    Rectangle {
                        id: textBackground

                        visible: root.isContainment

                        anchors {
                            left: label.left
                            leftMargin: -units.smallSpacing
                            top: label.top
                            topMargin: -units.smallSpacing
                            right: label.right
                            rightMargin: -units.smallSpacing
                            bottom: label.bottom
                            bottomMargin: -units.smallSpacing
                        }

                        color: (root.isContainment && main.GridView.view.isRootView) ? PlasmaCore.ColorScope.backgroundColor : "transparent"
                        radius: units.smallSpacing
                        opacity: 0.4
                    }

                    TextMetrics {
                        id: labelMetrics

                        font: label.font
                        elide: Text.ElideNone
                        text: label.text
                    }

                    PlasmaComponents.Label {
                        id: label

                        anchors {
                            top: root.isPopup ? undefined : icon.bottom
                            topMargin: 2 * units.smallSpacing
                            left: root.isPopup ? icon.right : undefined
                            leftMargin: units.smallSpacing * 2
                            horizontalCenter: root.isPopup ? undefined : parent.horizontalCenter
                            verticalCenter: root.isPopup ? parent.verticalCenter : undefined
                        }

                        width: {
                            if (root.isPopup) {
                                return parent.width - icon.width - (units.smallSpacing * 4);
                            }

                            return Math.min(labelMetrics.advanceWidth + units.smallSpacing, parent.width - units.smallSpacing * 8);
                        }

                        height: undefined // Unset PlasmaComponents.Label's default.

                        textFormat: Text.PlainText

                        maximumLineCount: root.isPopup ? 1 : plasmoid.configuration.textLines
                        wrapMode: Text.Wrap
                        elide: Text.ElideRight

                        horizontalAlignment: root.isPopup ? Text.AlignHLeft : Text.AlignHCenter

                        color: PlasmaCore.ColorScope.textColor

                        text: model.blank ? "" : model.display

                        Component.onCompleted: textFix.disableMouseHandling(label) // FIXME TODO: See https://codereview.qt-project.org/#/c/113758/
                    }
                }

                Column {
                    id: actions

                    x: units.smallSpacing * 3
                    y: units.smallSpacing * 3

                    anchors {
                        centerIn: root.isPopup ? icon : undefined
                    }

                    width: implicitWidth
                    height: implicitHeight

                    FolderItemActionButton {
                        id: selectionButton

                        visible: plasmoid.configuration.selectionMarkers && systemSettings.singleClick()
                        opacity: (visible && impl.hovered) ? 1.0 : 0.0

                        element: model.selected ? "remove" : "add"

                        onClicked: dir.toggleSelected(positioner.map(index))
                    }
                }

                Component {
                    id: popupButtonComponent

                    FolderItemActionButton {
                        visible: !root.isPopup

                        opacity: (plasmoid.configuration.popups && impl.hovered && impl.popupDialog == null) ? 1.0 : 0.0

                        element: "open"

                        onClicked: impl.openPopup()
                    }
                }

                states: [
                    State {
                        name: "selected"
                        when: model.selected

                        PropertyChanges {
                            target: frame
                            prefix: "selected"
                        }
                    },
                    State {
                        name: "hover"
                        when: hovered && !model.selected

                        PropertyChanges {
                            target: frame
                            prefix: "hover"
                        }
                    },
                    State {
                        name: "selected+hover"
                        when: hovered && model.selected

                        PropertyChanges {
                            target: frame
                            prefix: "selected+hover"
                        }
                    }
                ]
            }
        }
    }
}
