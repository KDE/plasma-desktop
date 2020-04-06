/*
 * Copyright 2019 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QtControls
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kcm 1.3 as KCM
import org.kde.private.kcms.style 1.0 as Private

KCM.GridViewKCM {
    id: root

    KCM.ConfigModule.quickHelp: i18n("This module allows you to modify the visual appearance of applications' user interface elements.")

    view.model: kcm.model
    view.currentIndex: kcm.model.selectedStyleIndex

    KCM.SettingStateBinding {
        configObject: kcm.styleSettings
        settingName: "widgetStyle"
    }

    function openGtkStyleSettings() {
        kcm.push("GtkStylePage.qml");
    }

    Component.onCompleted: {
        // The widget thumbnails are a bit more elaborate and need more room, especially when translated
        view.implicitCellWidth = Kirigami.Units.gridUnit * 20;
        view.implicitCellHeight = Kirigami.Units.gridUnit * 15;
    }

    // putting the InlineMessage as header item causes it to show up initially despite visible false
    header: ColumnLayout {
        Kirigami.InlineMessage {
            id: infoLabel
            Layout.fillWidth: true

            showCloseButton: true
            visible: false

            Connections {
                target: kcm
                onShowErrorMessage: {
                    infoLabel.type = Kirigami.MessageType.Error;
                    infoLabel.text = message;
                    infoLabel.visible = true;
                }
            }
        }
    }

    view.delegate: KCM.GridDelegate {
        id: delegate

        text: model.display
        toolTip: model.description

        thumbnailAvailable: thumbnailItem.valid
        thumbnail: Item {
            anchors.fill: parent
            clip: thumbnailItem.implicitWidth * thumbnailItem.scale > width

            Private.PreviewItem {
                id: thumbnailItem
                anchors.centerIn: parent
                // Fit the widget's width in the grid view
                // Round up to the nearest 0.05, so that 0.95 gets scaled up to 1.0
                // to avoid blurriness in case the widget is only slightly bigger
                scale: (Math.ceil(Math.min(1, parent.width / implicitWidth) * 20) / 20).toFixed(2)
                width: Math.max(parent.width, implicitWidth)
                // Scale the height back up as the background color comes from the style
                // and we don't want to leave an ugly gap above/below it
                height: parent.height / scale
                smooth: true
                styleName: model.styleName
            }

            Connections {
                target: kcm
                onStyleReconfigured: {
                    if (styleName === model.styleName) {
                        thumbnailItem.reload();
                    }
                }
            }
        }

        actions: [
            Kirigami.Action {
                iconName: "document-edit"
                tooltip: i18n("Configure Style...")
                enabled: model.configurable
                onTriggered: kcm.configure(model.display, model.styleName, delegate)
            }
        ]
        onClicked: {
            kcm.model.selectedStyle = model.styleName;
            view.forceActiveFocus();
        }
    }

    footer: ColumnLayout {
        RowLayout {
            Layout.alignment: Qt.AlignLeft

            QtControls.Button {
                id: effectSettingsButton
                text: i18n("Configure Icons and Toolbars")
                icon.name: "configure-toolbars" // proper icon?
                checkable: true
                checked: effectSettingsPopupLoader.item && effectSettingsPopupLoader.item.opened
                onClicked: {
                    effectSettingsPopupLoader.active = true;
                    effectSettingsPopupLoader.item.open();
                }
            }

            QtControls.Button {
                id: gtkSettingsButton
                visible: kcm.gtkConfigKdedModuleLoaded()
                text: i18n("Configure GNOME/GTK Application Style...")
                icon.name: "configure"
                onClicked: root.openGtkStyleSettings()
            }
        }
    }

    Loader {
        id: effectSettingsPopupLoader
        active: false
        sourceComponent: EffectSettingsPopup {
            parent: effectSettingsButton
            y: -height
        }
    }
}
