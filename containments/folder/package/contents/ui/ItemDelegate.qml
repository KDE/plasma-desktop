/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
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

import QtQuick 2.0
import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: main

    property int index: model.index
    property string name: model.display
    property QtObject popupDialog: null
    property Item label: label
    property Item actionsOverlay: actions
    property bool hovered: (GridView.view.hoveredItem == main)
    property bool selected: model.selected

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
        if (folder.itemViewDialogComponent.status == Component.Ready) {
            main.popupDialog = folder.itemViewDialogComponent.createObject(main);
            main.popupDialog.parentDelegate = main;
            main.popupDialog.visualParent = icon;
            main.popupDialog.url = model.url;
            main.popupDialog.visible = true;
        }
    }

    Timer {
        id: openPopupTimer

        interval: units.longDuration * 3

        onTriggered: {
            main.openPopup();
        }
    }

    PlasmaCore.FrameSvgItem {
        id: frame

        x: units.smallSpacing
        y: units.smallSpacing

        width: parent.width - (2 * units.smallSpacing)
        height: (icon.height + (2 * units.smallSpacing) + (label.lineCount
            * theme.mSize(theme.defaultFont).height) + (2 * units.largeSpacing))

        imagePath: "widgets/viewitem"

        PlasmaCore.ToolTipArea {
            id: toolTip

            anchors.fill: parent

            mainItem: toolTipDelegate
            active: (popupDialog == null)
            interactive: false
            location: plasmoid.location

            onContainsMouseChanged:  {
                if (containsMouse) {
                    toolTip.icon = model.decoration;
                    toolTip.mainText = model.display;
                    toolTip.subText = model.type + "\n" + model.size + i18n(" bytes") // TODO: Format by locale.
                    main.GridView.view.hoveredItem = main;
                }
            }

            PlasmaCore.IconItem {
                id: icon

                anchors {
                    top: parent.top
                    topMargin: units.largeSpacing
                    horizontalCenter: parent.horizontalCenter
                }

                width: main.GridView.view.iconSize
                height: main.GridView.view.iconSize

                source: model.decoration
            }

            PlasmaComponents.Label {
                id: label

                anchors {
                    top: icon.bottom
                    topMargin: 2 * units.smallSpacing
                    horizontalCenter: parent.horizontalCenter
                }

                width: parent.width - units.smallSpacing * 8 // TODO: Pick a frame prefix margin to cache and use instead.
                height: undefined // Unset PlasmaComponents.Label's default.

                textFormat: Text.PlainText

                maximumLineCount: plasmoid.configuration.textLines
                wrapMode: Text.Wrap
                elide: Text.ElideRight

                horizontalAlignment: Text.AlignHCenter

                color: (folder.isContainment && main.GridView.view.isRootView) ? "white" : theme.textColor // TODO: Make configurable once we have color buttons.

                text: model.display
            }

            Column {
                id: actions

                x: units.smallSpacing * 3 // TODO: Pick a frame prefix margin to cache and use instead.
                y: units.smallSpacing * 3 // TODO: Pick a frame prefix margin to cache and use instead.

                width: implicitWidth
                height: implicitHeight

                ItemActionButton {
                    width: systemSettings.singleClick ? units.iconSizes.small : 0
                    height: width

                    visible: systemSettings.singleClick
                    opacity: (systemSettings.singleClick && main.hovered) ? 1.0 : 0.0

                    element: model.selected ? "remove" : "add"

                    onClicked: dir.toggleSelected(model.index)
                }
            }

            Component {
                id: popupButtonComponent

                ItemActionButton {
                    opacity: (plasmoid.configuration.popups && main.hovered && main.popupDialog == null) ? 1.0 : 0.0

                    element: "open"

                    onClicked: main.openPopup()
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

            Component.onCompleted: {
                if (main.GridView.view.isRootView && model.isDir) {
                    popupButtonComponent.createObject(actions);
                }
            }
        }
    }
}
