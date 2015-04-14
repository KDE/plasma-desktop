/*
   Copyright (c) 2014 Marco Martin <mart@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.0 as QtControls
import org.kde.kquickcontrolsaddons 2.0
import QtQuick.Controls.Private 1.0
//We need units from it
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    implicitWidth: units.gridUnit * 20
    implicitHeight: units.gridUnit * 20

    SystemPalette {id: syspal}
    QtControls.ScrollView {
        anchors.fill: parent
        GridView {
            id: grid
            model: kcm.splashModel
            cellWidth: Math.floor(grid.width / Math.max(Math.floor(grid.width / (units.gridUnit*12)), 3))
            cellHeight: cellWidth / 1.6

            delegate: Item {
                width: grid.cellWidth
                height: grid.cellHeight
                Rectangle {
                    anchors {
                        fill: parent
                        margins: units.smallSpacing
                    }

                    Connections {
                        target: kcm
                        onSelectedPluginChanged: {
                            if (kcm.selectedPlugin == model.pluginName) {
                                makeCurrentTimer.pendingIndex = index
                            }
                        }
                    }
                    Component.onCompleted: {
                        if (kcm.selectedPlugin == model.pluginName) {
                            makeCurrentTimer.pendingIndex = index
                        }
                    }
                    QIconItem {
                        id: icon
                        anchors.centerIn: parent
                        width: units.iconSizes.large
                        height: width
                        icon: "view-preview"
                    }
                    Image {
                        id: image
                        anchors {
                            fill: parent
                            margins: units.smallSpacing * 2
                        }
                        source: model.screenshot || ""
                        Rectangle {
                            anchors {
                                left: parent.left
                                right: parent.right
                                bottom: parent.bottom
                            }
                            height: childrenRect.height
                            gradient: Gradient {
                                GradientStop {
                                    position: 0.0
                                    color: "transparent"
                                }
                                GradientStop {
                                    position: 1.0
                                    color: image.status == Image.Ready ? Qt.rgba(0, 0, 0, 0.5) : "transparent"
                                }
                            }
                            QtControls.Label {
                                anchors {
                                    horizontalCenter: parent.horizontalCenter
                                }
                                color: image.status == Image.Ready ? "white" : "gray"
                                text: model.display
                            }
                        }
                    }
                    Rectangle {
                        opacity: grid.currentIndex == index ? 1.0 : 0
                        anchors.fill: parent
                        border.width: units.smallSpacing * 2
                        border.color: syspal.highlight
                        color: "transparent"
                        Behavior on opacity {
                            PropertyAnimation {
                                duration: units.longDuration
                                easing.type: Easing.OutQuad
                            }
                        }
                    }
                    MouseArea {
                        anchors.fill: parent
                        hoverEnabled: true
                        onClicked: {
                            grid.currentIndex = index
                            kcm.selectedPlugin = model.pluginName
                        }
                        Timer {
                            interval: 1000 // FIXME TODO: Use platform value for tooltip activation delay.

                            running: parent.containsMouse && !parent.pressedButtons

                            onTriggered: {
                                Tooltip.showText(parent, Qt.point(parent.mouseX, parent.mouseY), model.display);
                            }
                        }
                        PlasmaComponents.ToolButton {
                            anchors {
                                top: parent.top
                                right: parent.right
                                margins: units.smallSpacing
                            }
                            visible: model.pluginName != "None"
                            iconSource: "media-playback-start"
                            tooltip: i18n("Test Splashscreen")
                            flat: false
                            onClicked: kcm.test(model.pluginName)
                            opacity: parent.containsMouse ? 1 : 0
                            Behavior on opacity {
                                PropertyAnimation {
                                    duration: units.longDuration
                                    easing.type: Easing.OutQuad
                                }
                            }
                        }
                    }
                }
            }
            Timer {
                id: makeCurrentTimer
                interval: 100
                repeat: false
                property int pendingIndex
                onPendingIndexChanged: makeCurrentTimer.restart()
                onTriggered: {
                    grid.currentIndex = pendingIndex
                }
            }
        }
    }
}
