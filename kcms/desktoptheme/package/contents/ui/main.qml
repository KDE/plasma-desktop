/*
   Copyright (c) 2014 Marco Martin <mart@kde.org>
   Copyright (c) 2016 David Rosca <nowrep@gmail.com>

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
import QtQuick.Dialogs 1.0
import QtQuick.Controls.Private 1.0
import QtQuick.Controls 1.0 as QtControls

import org.kde.kcm 1.0
import org.kde.plasma.core 2.0 // for units

Item {
    implicitWidth: units.gridUnit * 20
    implicitHeight: units.gridUnit * 20

    ConfigModule.quickHelp: i18n("This module lets you configure the desktop theme.")

    SystemPalette {
        id: syspal
    }

    ColumnLayout {
        anchors.fill: parent
        QtControls.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            verticalScrollBarPolicy: Qt.ScrollBarAlwaysOn
            GridView {
                id: grid
                model: kcm.desktopThemeModel
                cellWidth: Math.floor(grid.width / Math.max(Math.floor(grid.width / (units.gridUnit * 12)), 3))
                cellHeight: cellWidth / 1.6

                delegate: Item {
                    property bool isLocal : model.isLocal
                    property string pluginName : model.pluginName
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

                        Item {
                            anchors {
                                fill: parent
                                margins: units.smallSpacing * 2
                            }
                            ThemePreview {
                                id: preview
                                anchors {
                                    top: parent.top
                                    left: parent.left
                                    right: parent.right
                                    bottom: label.top
                                }
                                themeName: model.pluginName
                            }
                            QtControls.Label {
                                id: label
                                anchors {
                                    bottom: parent.bottom
                                    horizontalCenter: parent.horizontalCenter
                                    leftMargin: units.smallSpacing * 2
                                    rightMargin: units.smallSpacing * 2
                                }
                                height: paintedHeight
                                width: parent.width
                                color: "black"
                                text: model.themeName
                                elide: Text.ElideRight
                                horizontalAlignment: Text.AlignHCenter
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
                                interval: 1000
                                running: parent.containsMouse && !parent.pressedButtons
                                onTriggered: {
                                    Tooltip.showText(parent, Qt.point(parent.mouseX, parent.mouseY), model.themeName);
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
        RowLayout {
            QtControls.Button {
                text: i18n("Get new Theme")
                iconName: "get-hot-new-stuff"
                onClicked: kcm.getNewThemes()
            }

            QtControls.Button {
                text: i18n("Install from File")
                iconName: "document-import"
                onClicked: fileDialogLoader.active = true;
            }

            QtControls.Button {
                text: i18n("Remove Theme")
                iconName: "edit-delete"
                enabled: grid.currentItem && grid.currentItem.isLocal
                onClicked: {
                    kcm.removeTheme(grid.currentItem.pluginName);
                    updateSelectedPluginTimer.restart();
                }
            }

            Item {
                Layout.fillWidth: true
            }

            QtControls.Label {
                id: infoLabel
            }
        }
    }

    Connections {
        target: kcm
        onShowInfoMessage: {
            infoLabel.text = infoMessage;
            hideInfoMessageTimer.restart();
        }
    }

    Timer {
        id: updateSelectedPluginTimer
        interval: 100
        onTriggered: kcm.selectedPlugin = grid.currentItem.pluginName
    }

    Timer {
        id: hideInfoMessageTimer
        interval: 20 * 1000
        onTriggered: {
            infoLabel.text = ""
        }
    }

    Loader {
        id: fileDialogLoader
        active: false
        sourceComponent: FileDialog {
            visible: true
            title: i18n("Open Theme")
            folder: shortcuts.home
            nameFilters: [ i18n("Theme Files (*.zip *.tar.gz *.tar.bz2)") ]
            onAccepted: {
                kcm.installThemeFromFile(fileUrls[0])
                fileDialogLoader.active = false
            }
            onRejected: {
                fileDialogLoader.active = false
            }
        }
    }
}
