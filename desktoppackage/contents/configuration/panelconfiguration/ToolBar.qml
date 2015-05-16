/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.configuration 2.0

Item {
    id: root
    state: parent.state
    implicitWidth: Math.max(buttonsLayout.width, row.width) + units.smallSpacing * 2
    implicitHeight: row.height + 20

    readonly property string addWidgetsButtonText: i18nd("plasma_shell_org.kde.plasma.desktop", "Add Widgets...")
    readonly property string addSpacerButtonText: i18nd("plasma_shell_org.kde.plasma.desktop", "Add Spacer")
    readonly property string settingsButtonText: i18nd("plasma_shell_org.kde.plasma.desktop", "More Settings...")

    GridLayout {
        id: row
        columns: dialogRoot.vertical ? 1 : 2
        rows: dialogRoot.vertical ? 2 : 1
        anchors.centerIn: parent

        rowSpacing: units.smallSpacing
        columnSpacing: units.smallSpacing

        EdgeHandle {
          Layout.alignment: Qt.AlignHCenter
        }
        SizeHandle {
          Layout.alignment: Qt.AlignHCenter
        }
    }

    PlasmaComponents.Label {
        id: placeHolder
        visible: false
        text: addWidgetsButtonText + addSpacerButtonText + settingsButtonText
    }

    Connections {
        target: configDialog
        onVisibleChanged: {
            if (!configDialog.visible) {
                contextMenuLoader.close()
            }
        }
    }

    GridLayout {
        id: buttonsLayout
        rows: 1
        columns: 1
        flow: plasmoid.formFactor == PlasmaCore.Types.Horizontal ? GridLayout.TopToBottom : GridLayout.LeftToRight

        anchors.margins: rowSpacing

        property bool showText: plasmoid.formFactor == PlasmaCore.Types.Vertical || (row.x + row.width < root.width - placeHolder.width - units.iconSizes.small*4 - units.largeSpacing*5)

        rowSpacing: units.smallSpacing
        columnSpacing: units.smallSpacing

        PlasmaComponents.Button {
            text: buttonsLayout.showText ? root.addWidgetsButtonText : ""
            tooltip: buttonsLayout.showText ? "" : root.addWidgetsButtonText
            iconSource: "list-add"
            Layout.fillWidth: true
            onClicked: {
                configDialog.close();
                configDialog.showAddWidgetDialog();
            }
        }

        PlasmaComponents.Button {
            iconSource: "distribute-horizontal-x"
            text: buttonsLayout.showText ? root.addSpacerButtonText : ""
            tooltip: buttonsLayout.showText ? "" : root.addSpacerButtonText
            Layout.fillWidth: true
            onClicked: {
                configDialog.addPanelSpacer();
            }
        }

        PlasmaComponents.Button {
            id: settingsButton
            iconSource: "configure"
            text: buttonsLayout.showText ? root.settingsButtonText : ""
            tooltip: buttonsLayout.showText ? "" : root.settingsButtonText
            Layout.fillWidth: true
            onClicked: {
                if (contextMenuLoader.opened) {
                    contextMenuLoader.close()
                } else {
                    contextMenuLoader.open()
                }
            }
        }

        PlasmaComponents.ToolButton {
            parent: plasmoid.formFactor == PlasmaCore.Types.Horizontal ? buttonsLayout : root
            anchors.right: plasmoid.formFactor == PlasmaCore.Types.Horizontal ? undefined : parent.right
            iconSource: "window-close"
            tooltip: i18nd("plasma_shell_org.kde.plasma.desktop", "Close")
            onClicked: {
                configDialog.close()
            }
        }

        Loader {
            id: contextMenuLoader
            property bool opened: item && item.visible
            source: "MoreSettingsMenu.qml"
            active: false

            function open() {
                active = true
                item.visible = true
            }
            function close() {
                if (item) {
                    item.visible = false
                }
            }
        }

    }
//BEGIN States
    states: [
        State {
            name: "TopEdge"
            PropertyChanges {
                target: root
                height: root.implicitHeight
            }
            AnchorChanges {
                target: root
                anchors {
                    top: undefined
                    bottom: root.parent.bottom
                    left: root.parent.left
                    right: root.parent.right
                }
            }
            AnchorChanges {
                target: buttonsLayout
                anchors {
                    verticalCenter: root.verticalCenter
                    top: undefined
                    bottom: undefined
                    left: undefined
                    right: root.right
                }
            }
            PropertyChanges {
                target: buttonsLayout
                width: buttonsLayout.implicitWidth
            }
        },
        State {
            name: "BottomEdge"
            PropertyChanges {
                target: root
                height: root.implicitHeight
            }
            AnchorChanges {
                target: root
                anchors {
                    top: root.parent.top
                    bottom: undefined
                    left: root.parent.left
                    right: root.parent.right
                }
            }
            AnchorChanges {
                target: buttonsLayout
                anchors {
                    verticalCenter: root.verticalCenter
                    top: undefined
                    bottom: undefined
                    left: undefined
                    right: root.right
                }
            }
            PropertyChanges {
                target: buttonsLayout
                width: buttonsLayout.implicitWidth
            }
        },
        State {
            name: "LeftEdge"
            PropertyChanges {
                target: root
                width: root.implicitWidth
            }
            AnchorChanges {
                target: root
                anchors {
                    top: root.parent.top
                    bottom: root.parent.bottom
                    left: undefined
                    right: root.parent.right
                }
            }
            AnchorChanges {
                target: buttonsLayout
                anchors {
                    verticalCenter: undefined
                    top: undefined
                    bottom: root.bottom
                    left: root.left
                    right: root.right
                }
            }
        },
        State {
            name: "RightEdge"
            PropertyChanges {
                target: root
                width: root.implicitWidth
            }
            AnchorChanges {
                target: root
                anchors {
                    top: root.parent.top
                    bottom: root.parent.bottom
                    left: root.parent.left
                    right: undefined
                }
            }
            AnchorChanges {
                target: buttonsLayout
                anchors {
                    verticalCenter: undefined
                    top: undefined
                    bottom: root.bottom
                    left: root.left
                    right: root.right
                }
            }
        }
    ]
//END States
}
