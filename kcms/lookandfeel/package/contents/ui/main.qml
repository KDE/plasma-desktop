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
import org.kde.newstuff 1.62 as NewStuff
import org.kde.kconfig 1.0 // for KAuthorized
import org.kde.kcm 1.3 as KCM

KCM.GridViewKCM {
    id: root
    KCM.ConfigModule.quickHelp: i18n("This module lets you choose the global look and feel.")

    view.model: kcm.lookAndFeelModel
    view.currentIndex: kcm.pluginIndex(kcm.lookAndFeelSettings.lookAndFeelPackage)

    KCM.SettingStateBinding {
        configObject: kcm.lookAndFeelSettings
        settingName: "lookAndFeelPackage"
    }

    view.delegate: KCM.GridDelegate {
        id: delegate

        text: model.display
        toolTip: model.description

        thumbnailAvailable: model.screenshot
        thumbnail: Image {
            anchors.fill: parent
            source: model.screenshot || ""
            sourceSize: Qt.size(delegate.GridView.view.cellWidth * Screen.devicePixelRatio,
                                delegate.GridView.view.cellHeight * Screen.devicePixelRatio)
        }
        actions: [
            Kirigami.Action {
                visible: model.fullScreenPreview !== ""
                iconName: "view-preview"
                tooltip: i18n("Preview Theme")
                onTriggered: {
                    previewWindow.url = model.fullScreenPreview;
                    previewWindow.showFullScreen();
                }
            }
        ]
        onClicked: {
            kcm.lookAndFeelSettings.lookAndFeelPackage = model.pluginName;
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
            NewStuff.Button {
                text: i18n("Get New Global Themes...")
                configFile: "lookandfeel.knsrc"
                viewMode: NewStuff.Page.ViewMode.Preview
                onChangedEntriesChanged: kcm.reloadModel();
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
            onClicked: previewWindow.close()
            QtControls.ToolButton {
                anchors {
                    top: parent.top
                    right: parent.right
                }
                icon.name: "window-close"
                onClicked: previewWindow.close()
            }
            Shortcut {
                onActivated: previewWindow.close()
                sequence: "Esc"
            }
        }
    }
}
