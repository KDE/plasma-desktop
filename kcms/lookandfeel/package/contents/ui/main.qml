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
import QtQuick.Window 2.2
import QtQuick.Controls 1.0 as QtControls
import org.kde.kquickcontrolsaddons 2.0
import QtQuick.Controls.Private 1.0
import org.kde.kirigami 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kcm 1.0

Item {
    id: root
    implicitWidth: Units.gridUnit * 20
    implicitHeight: Units.gridUnit * 20

    ConfigModule.quickHelp: i18n("This module lets you configure the look of the whole workspace with some ready to go presets.")

    // HACK QtQuick Controls 1 Button does not update its styleOption palette when it changes, since QQC1 is basically
    // unmaintained and we're eventually moving to QQC2 and this is the first impression the user gets when playing with
    // look and feel feature, we destroy and re-create the GHNS button when its color would change to avoid this.
    SystemPalette {
        id: syspal
        onButtonChanged: {
            ghnsButtonLoader.active = false;
            ghnsButtonLoader.active = true;
        }
    }

    ColumnLayout {
        anchors.fill: parent
        QtControls.Label {
            text: i18nd("kcm_lookandfeel", "Select an overall theme for your workspace (including plasma theme, color scheme, mouse cursor, window and desktop switcher, splash screen, lock screen etc.)")
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        QtControls.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            GridView {
                id: grid
                model: kcm.lookAndFeelModel
                cellWidth: Math.floor(root.width / Math.max(Math.floor(root.width / (Units.gridUnit*12)), 3)) - Units.gridUnit
                cellHeight: cellWidth / 1.6

                onCountChanged: {
                    grid.currentIndex = kcm.selectedPluginIndex;
                    grid.positionViewAtIndex(grid.currentIndex, GridView.Visible)
                }
                delegate: Item {
                    width: grid.cellWidth
                    height: grid.cellHeight

                    Rectangle {
                        anchors {
                            fill: parent
                            margins: Units.smallSpacing
                        }
                        Connections {
                            target: kcm
                            onSelectedPluginChanged: {
                                if (kcm.selectedPlugin == model.pluginName) {
                                    grid.currentIndex = index
                                }
                            }
                        }
                        QIconItem {
                            id: icon
                            anchors.centerIn: parent
                            width: Units.iconSizes.large
                            height: width
                            icon: "view-preview"
                        }
                        Image {
                            anchors {
                                fill: parent
                                margins: Units.smallSpacing * 2
                            }
                            source: model.screenshot

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
                                        color: Qt.rgba(0, 0, 0, 0.5)
                                    }
                                }
                                QtControls.Label {
                                    anchors {
                                        horizontalCenter: parent.horizontalCenter
                                    }
                                    color: "white"
                                    text: model.display
                                }
                            }
                        }
                        Rectangle {
                            opacity: grid.currentIndex == index ? 1.0 : 0
                            anchors.fill: parent
                            border.width: Units.smallSpacing * 2
                            border.color: syspal.highlight
                            color: "transparent"
                            Behavior on opacity {
                                PropertyAnimation {
                                    duration: Units.longDuration
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
                                resetCheckbox.checked = false;
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
                                    margins: Units.smallSpacing
                                }
                                visible: model.fullScreenPreview != ""
                                iconSource: "media-playback-start"
                                tooltip: i18n("Show Preview")
                                flat: false
                                onClicked: {
                                    previewWindow.url = model.fullScreenPreview;
                                    previewWindow.showFullScreen();
                                }
                                opacity: parent.containsMouse ? 1 : 0
                                Behavior on opacity {
                                    PropertyAnimation {
                                        duration: Units.longDuration
                                        easing.type: Easing.OutQuad
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        QtControls.Label {
            text: i18nd("kcm_lookandfeel", "Warning: your Plasma Desktop layout will be lost and reset to the default layout provided by the selected theme.")
            visible: resetCheckbox.checked
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
        }
        Connections {
            target: kcm
            onNeedsSaveChanged: {
                if (!needsSave) {
                    resetCheckbox.checked = false;
                }
            }
        }
        RowLayout {
            QtControls.CheckBox {
                id: resetCheckbox
                checked: kcm.resetDefaultLayout
                text: i18n("Use Desktop Layout from theme")
                onCheckedChanged: kcm.resetDefaultLayout = checked;
            }
            Item {
                Layout.fillWidth: true
            }
            Loader {
                id: ghnsButtonLoader
                anchors.right: parent.right

                sourceComponent: QtControls.Button {
                    text: i18n("Get New Looks...")
                    iconName: "get-hot-new-stuff"
                    onClicked: kcm.getNewStuff();
                }
            }
        }
    }

    Window {
        id: previewWindow
        property alias url: previewImage.source
        color: Qt.rgba(0, 0, 0, 0.7)
        MouseArea {
            anchors.fill: parent
            Image {
                id: previewImage
                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                width: Math.min(parent.width, sourceSize.width)
                height: Math.min(parent.height, sourceSize.height)
            }
            onClicked: previewWindow.visible = false;
            QtControls.ToolButton {
                anchors {
                    top: parent.top
                    right: parent.right
                }
                iconName: "window-close"
                onClicked: previewWindow.visible = false;
            }
            QtControls.Action {
                onTriggered: previewWindow.visible = false;
                shortcut: "Esc"
            }
        }
    }
}
