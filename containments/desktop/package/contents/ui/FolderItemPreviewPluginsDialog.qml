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

ApplicationWindow {
    id: dialog

    width: units.gridUnit * 15
    height: units.gridUnit * 15

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

    ColumnLayout {
        anchors.fill: parent

        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true

            frameVisible: true

            ListView {
                model: previewPluginsModel

                delegate: CheckBox {
                    text: model.display

                    checked: model.checked

                    onCheckedChanged: {
                        if (checked != model.checked) {
                            previewPluginsModel.setRowChecked(model.index, checked);
                        }
                    }
                }
            }
        }

        RowLayout {
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
