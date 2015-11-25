/*
*   Copyright 2013 by Sebastian Kügler <sebas@kde.org>
*   Copyright 2014 by Martin Gräßlin <mgraesslin@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Library General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

Column {
    id: tooltipContentItem

    property Item toolTip
    property variant windows
    property string mainText
    property string subText
    property variant icon

    property int preferredTextWidth: theme.mSize(theme.defaultFont).width * 30
    property int _s: units.largeSpacing / 2

    width: Math.max(windowRow.width, appLabelRow.width)  + _s
    height: childrenRect.height
    spacing: _s

    PlasmaExtras.ScrollArea {
        id: scrollArea
        anchors.horizontalCenter: parent.horizontalCenter
        width: Math.min(parent.width, windowRow.width)
        height: windowRow.height
        verticalScrollBarPolicy: Qt.ScrollBarAlwaysOff
        horizontalScrollBarPolicy: Qt.ScrollBarAlwaysOff

        Component.onCompleted: {
            flickableItem.interactive = Qt.binding(function() {
                return contentItem.width > viewport.width;
            });
        }

        Row {
            id: windowRow
            width: childrenRect.width
            height: childrenRect.height
            spacing: units.largeSpacing
            Repeater {
                model: windows

                PlasmaCore.WindowThumbnail {
                    y: _s

                    width: units.gridUnit * 15
                    height: units.gridUnit * 10

                    winId: modelData

                    MouseArea {
                        anchors.fill: parent

                        acceptedButtons: Qt.LeftButton
                        hoverEnabled: true

                        onClicked: {
                            tasks.activateWindow(modelData);
                            toolTip.hideToolTip();
                        }

                        onContainsMouseChanged: {
                            tasks.windowHovered(modelData, containsMouse);
                        }

                        Rectangle {
                            id: closeButtonBackground

                            anchors {
                                top: parent.top
                                right: parent.right
                                topMargin: units.smallSpacing
                                rightMargin: units.smallSpacing
                            }

                            width: units.iconSizes.smallMedium
                            height: width

                            opacity: parent.containsMouse ? 0.4 : 0

                            Behavior on opacity {
                                NumberAnimation { duration: units.shortDuration * 2; }
                            }

                            color: theme.backgroundColor
                            radius: units.smallSpacing
                        }

                        PlasmaComponents.ToolButton {
                            id: closeButton

                            anchors {
                                horizontalCenter: closeButtonBackground.horizontalCenter
                                verticalCenter: closeButtonBackground.verticalCenter
                            }

                            opacity: parent.containsMouse ? 1.0 : 0

                            Behavior on opacity {
                                NumberAnimation { duration: units.shortDuration * 2; }
                            }

                            width: units.iconSizes.smallMedium
                            height: width

                            iconSource: "window-close"

                            onClicked: {
                                tasks.closeByWinId(modelData);
                                toolTip.hideToolTip();
                            }
                        }
                    }
                }
            }
        }
    }

    Row {
        id: appLabelRow
        width: childrenRect.width + _s
        height: childrenRect.height + units.largeSpacing
        spacing: units.largeSpacing

        Item {
            id: imageContainer
            width: tooltipIcon.width
            height: tooltipIcon.height
            y: _s

            PlasmaCore.IconItem {
                id: tooltipIcon
                x: _s
                width: units.iconSizes.desktop
                height: width
                source: icon
            }
        }

        Column {
            id: mainColumn
            y: _s

            //This instance is purely for metrics
            PlasmaExtras.Heading {
                id: tooltipMaintextPlaceholder
                visible: false
                level: 3
                text: mainText
                textFormat: Text.PlainText
            }
            PlasmaExtras.Heading {
                id: tooltipMaintext
                level: 3
                width: Math.min(tooltipMaintextPlaceholder.width, preferredTextWidth)
                //width: 400
                elide: Text.ElideRight
                text: mainText
                textFormat: Text.PlainText
            }
            PlasmaComponents.Label {
                id: tooltipSubtext
                width: tooltipContentItem.preferredTextWidth
                wrapMode: Text.WordWrap
                text: subText
                textFormat: Text.PlainText
                opacity: 0.5
            }
        }
    }
}
