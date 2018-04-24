/*
   Copyright (c) 2014 Marco Martin <mart@kde.org>
   Copyright (c) 2016 David Rosca <nowrep@gmail.com>
   Copyright (c) 2018 Kai Uwe Broulik <kde@privat.broulik.de>

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
import QtQuick.Controls 2.3 as QtControls
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kcm 1.1 as KCM

KCM.GridViewKCM {
    KCM.ConfigModule.quickHelp: i18n("This module lets you configure the desktop theme.")

    view.model: kcm.desktopThemeModel
    view.currentIndex: kcm.indexOf(kcm.selectedPlugin)

    view.remove: Transition {
        ParallelAnimation {
            NumberAnimation { property: "scale"; to: 0.5; duration: Kirigami.Units.longDuration }
            NumberAnimation { property: "opacity"; to: 0.0; duration: Kirigami.Units.longDuration }
        }
    }

    view.removeDisplaced: Transition {
        SequentialAnimation {
            // wait for the "remove" animation to finish
            PauseAnimation { duration: Kirigami.Units.longDuration }
            NumberAnimation { properties: "x,y"; duration: Kirigami.Units.longDuration }
        }
    }

    view.delegate: KCM.GridDelegate {
        id: delegate

        text: model.themeName
        toolTip: model.description || model.themeName

        thumbnailAvailable: true
        thumbnail: ThemePreview {
            id: preview
            anchors.fill: parent
            themeName: model.pluginName
        }

        actions: [
            Kirigami.Action {
                iconName: "document-edit"
                tooltip: i18n("Edit Theme")
                visible: kcm.canEditThemes
                onTriggered: kcm.editTheme(model.pluginName)
            },
            Kirigami.Action {
                iconName: "edit-delete"
                tooltip: i18n("Remove Theme")
                enabled: model.isLocal
                onTriggered: kcm.removeTheme(model.pluginName)
            }
        ]

        onClicked: {
            kcm.selectedPlugin = model.pluginName;
            view.forceActiveFocus();
        }
    }

    footer: ColumnLayout {
        Kirigami.InlineMessage {
            id: infoLabel
            Layout.fillWidth: true

            showCloseButton: true

            Connections {
                target: kcm
                onShowSuccessMessage: {
                    infoLabel.type = Kirigami.MessageType.Positive;
                    infoLabel.text = message;
                    infoLabel.visible = true;
                }
                onShowErrorMessage: {
                    infoLabel.type = Kirigami.MessageType.Error;
                    infoLabel.text = message;
                    infoLabel.visible = true;
                }
            }
        }

        RowLayout {
            Layout.alignment: Qt.AlignRight

            QtControls.Button {
                text: i18n("Install from File...")
                icon.name: "document-import"
                onClicked: fileDialogLoader.active = true;
            }

            QtControls.Button {
                text: i18n("Get New Themes...")
                icon.name: "get-hot-new-stuff"
                onClicked: kcm.getNewThemes()
            }
        }
    }

    Loader {
        id: fileDialogLoader
        active: false
        sourceComponent: FileDialog {
            title: i18n("Open Theme")
            folder: shortcuts.home
            nameFilters: [ i18n("Theme Files (*.zip *.tar.gz *.tar.bz2)") ]
            Component.onCompleted: open()
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
