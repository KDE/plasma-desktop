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
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

import org.kde.private.desktopcontainment.folder 0.1 as Folder
import org.kde.plasma.core 2.0 as PlasmaCore

ApplicationWindow {
    id: dialog

    width: PlasmaCore.Units.gridUnit * 15
    height: PlasmaCore.Units.gridUnit * 15

    visible: false

    property variant previewPlugins: plasmoid.configuration.previewPlugins

    title: i18n("Preview Plugins")

    flags: Qt.Dialog
    modality: Qt.WindowModal

    onVisibleChanged: {
        previewPluginsModel.checkedPlugins = plasmoid.configuration.previewPlugins;
    }

    Folder.PreviewPluginsModel {
        id: previewPluginsModel
    }

    SystemPalette {
        id: systemPalette;
        colorGroup: SystemPalette.Active
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 2*PlasmaCore.Units.smallSpacing

        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: systemPalette.base

            ScrollView {
                anchors.fill: parent

                frameVisible: true

                ListView {
                    model: previewPluginsModel

                    delegate: CheckBox {
                        Layout.leftMargin: PlasmaCore.Units.smallSpacing
                        Layout.rightMargin: PlasmaCore.Units.smallSpacing

                        text: model.display

                        checked: model.checked
                        onCheckedChanged: model.checked = checked
                    }
                }
            }
        }

        RowLayout {
            Layout.margins: PlasmaCore.Units.smallSpacing
            Layout.alignment: (Qt.application.layoutDirection == Qt.LeftToRight) ? Qt.AlignRight : Qt.AlignLeft

            layoutDirection: Qt.application.layoutDirection

            Button {
                id: okButton

                iconName: "dialog-ok"
                text: i18n("OK")

                onClicked: {
                    previewPlugins = previewPluginsModel.checkedPlugins;

                    dialog.visible = false;
                }
            }

            Button {
                id: cancelButton

                iconName: "dialog-cancel"
                text: i18n("Cancel")

                onClicked: {
                    dialog.visible = false;
                }
            }
        }
    }
}
