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
import QtQuick.Layouts 1.1
import QtQuick.Window 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: root
    Layout.minimumWidth: PlasmaCore.Units.gridUnit * 15
    Layout.minimumHeight: PlasmaCore.Units.gridUnit * 15
    Layout.maximumWidth: PlasmaCore.Units.gridUnit * 15
    Layout.maximumHeight: PlasmaCore.Units.gridUnit * 15

    property var reason
    property var errorInformation: {}

    clip: true

    PlasmaComponents.ToolButton {
        icon.name: "arrow-up"
        visible: !dialog.visible
        onClicked: dialog.visible = true

        anchors {
            bottom: parent.bottom
            right: parent.right
        }
    }

    RowLayout {
        anchors.fill: parent

        ColumnLayout {
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            Layout.fillWidth: true

            PlasmaCore.IconItem {
                Layout.minimumWidth: PlasmaCore.Units.iconSizes.huge
                Layout.minimumHeight: PlasmaCore.Units.iconSizes.huge
                source: "dialog-error"
                Layout.alignment: Qt.AlignHCenter
            }

            PlasmaExtras.Heading {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Sorry! There was an error loading %1.", root.errorInformation.appletName)
                level: 2
                wrapMode: Text.Wrap
                horizontalAlignment: Text.AlignHCenter
                Layout.fillWidth: true
            }

            PlasmaComponents.Button {
                text: i18nd("plasma_shell_org.kde.plasma.desktop", "Copy Error Details to Clipboard")
                icon.name: "edit-copy"

                onClicked: copyHelper.copyText()
                Layout.alignment: Qt.AlignHCenter
            }

            Window {
                id: dialog

                width: PlasmaCore.Units.gridUnit * 20
                height: PlasmaCore.Units.gridUnit * 20
                visible: false

                color: PlasmaCore.Theme.backgroundColor

                PlasmaComponents.TextArea {
                    anchors.fill: parent

                    text: root.errorInformation.errors.join("\n\n")
                    readOnly: true
                    wrapMode: TextEdit.Wrap

                    background: null
                }
            }
            PlasmaComponents.TextArea {
                id: copyHelper
                visible: root.errorInformation.isDebugMode

                wrapMode: TextEdit.Wrap

                text: root.errorInformation.errors.join("\n\n")
                readOnly: true
                Layout.fillHeight: true
                Layout.fillWidth: true

                function copyText() {
                    selectAll()
                    copy()
                }
            }
        }
    }

}
