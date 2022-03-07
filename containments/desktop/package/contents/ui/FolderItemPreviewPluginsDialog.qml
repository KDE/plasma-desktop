/*
    SPDX-FileCopyrightText: 2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 1.1
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.private.desktopcontainment.folder 0.1 as Folder
import org.kde.plasma.core 2.0 as PlasmaCore

ApplicationWindow {
    id: dialog

    width: PlasmaCore.Units.gridUnit * 15
    height: PlasmaCore.Units.gridUnit * 15

    visible: false

    property variant previewPlugins: Plasmoid.configuration.previewPlugins

    title: i18n("Preview Plugins")

    flags: Qt.Dialog
    modality: Qt.WindowModal

    onVisibleChanged: {
        previewPluginsModel.checkedPlugins = Plasmoid.configuration.previewPlugins;
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
