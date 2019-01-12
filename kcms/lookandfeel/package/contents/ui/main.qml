/*
   Copyright (c) 2018 Marco Martin <mart@kde.org>

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

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Window 2.2
import QtQuick.Controls 2.3 as QtControls
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kconfig 1.0 // for KAuthorized
import org.kde.kcm 1.1 as KCM

KCM.GridViewKCM {
    KCM.ConfigModule.quickHelp: i18n("This module lets you choose the Look and Feel theme.")

    view.model: kcm.lookAndFeelModel
    view.currentIndex: kcm.selectedPluginIndex
    view.delegate: KCM.GridDelegate {
        id: delegate

        text: model.display
        toolTip: model.description

        thumbnailAvailable: model.screenshot
        thumbnail: Image {
            anchors.fill: parent
            source: model.screenshot || ""
        }
        actions: [
            Kirigami.Action {
                visible: model.fullScreenPreview != ""
                iconName: "media-playback-start"
                tooltip: i18n("Preview Theme")
                onTriggered: {
                    previewWindow.url = model.fullScreenPreview;
                    previewWindow.showFullScreen();
                }
            }
        ]
        onClicked: {
            kcm.selectedPlugin = model.pluginName;
            view.forceActiveFocus();
            resetCheckbox.checked = false;
        }
    }

    footer: ColumnLayout {
        Kirigami.InlineMessage {
            Layout.fillWidth: true
            type: Kirigami.MessageType.Warning
            text: i18n("Your desktop layout will be lost and reset to the default layout provided by the selected theme.")
            visible: resetCheckbox.checked
        }

        RowLayout {
            Layout.fillWidth: true

            QtControls.CheckBox {
                id: resetCheckbox
                checked: kcm.resetDefaultLayout
                text: i18n("Use desktop layout from theme")
                onCheckedChanged: kcm.resetDefaultLayout = checked;
            }
            Item {
                Layout.fillWidth: true
            }
            QtControls.Button {
                text: i18n("Get New Look and Feel Themes...")
                icon.name: "get-hot-new-stuff"
                onClicked: kcm.getNewStuff(this);
                visible: KAuthorized.authorize("ghns")
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
                icon.name: "window-close"
                onClicked: previewWindow.visible = false;
            }
            Shortcut {
                onActivated: previewWindow.visible = false;
                sequence: "Esc"
            }
        }
    }
}
