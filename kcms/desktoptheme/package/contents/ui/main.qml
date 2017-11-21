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
import org.kde.kirigami 2.0 // for units
import org.kde.plasma.components 2.0 as PlasmaComponents //the round toolbutton

Item {
    implicitWidth: Units.gridUnit * 20
    implicitHeight: Units.gridUnit * 20

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
                cellWidth: Math.floor(grid.width / Math.max(Math.floor(grid.width / (Units.gridUnit * 12)), 3))
                cellHeight: cellWidth / 1.6

                onCountChanged: {
                    grid.currentIndex = kcm.indexOf(kcm.selectedPlugin);
                    grid.positionViewAtIndex(grid.currentIndex, GridView.Visible)
                }

                delegate: Item {
                    property bool isLocal : model.isLocal
                    property string pluginName : model.pluginName
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
                                    makeCurrentTimer.pendingIndex = index
                                }
                            }
                        }
                        Component.onCompleted: {
                            if (kcm.selectedPlugin == model.pluginName) {
                                makeCurrentTimer.pendingIndex = index
                            }
                        }

                        MouseArea {
                            anchors {
                                fill: parent
                                margins: Units.smallSpacing * 2
                            }
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

                            PlasmaComponents.ToolButton {
                                anchors {
                                    bottom: preview.bottom
                                    right: preview.right
                                    margins: units.smallSpacing
                                }
                                iconSource: "document-edit"
                                tooltip: i18("Edit theme")
                                flat: false
                                onClicked: kcm.editTheme(model.pluginName)
                                visible: kcm.canEditThemes
                                opacity: parent.containsMouse ? 1 : 0
                                Behavior on opacity {
                                    PropertyAnimation {
                                        duration: units.longDuration
                                        easing.type: Easing.OutQuad
                                    }
                                }
                            }

                            QtControls.Label {
                                id: label
                                anchors {
                                    bottom: parent.bottom
                                    horizontalCenter: parent.horizontalCenter
                                    leftMargin: Units.smallSpacing * 2
                                    rightMargin: Units.smallSpacing * 2
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
                text: i18n("Get New Themes...")
                iconName: "get-hot-new-stuff"
                onClicked: kcm.getNewThemes()
            }

            QtControls.Button {
                text: i18n("Install from File...")
                iconName: "document-import"
                onClicked: fileDialogLoader.active = true;
            }

            QtControls.Button {
                text: i18n("Remove Theme")
                iconName: "edit-delete"
                enabled: grid.currentItem && grid.currentItem.isLocal
                onClicked: {
                    kcm.removeTheme(grid.currentItem.pluginName);
                    kcm.selectedPlugin = grid.currentItem.pluginName
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
