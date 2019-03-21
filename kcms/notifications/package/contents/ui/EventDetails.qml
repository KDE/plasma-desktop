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

import QtQuick 2.9
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.3 as QtControls

import org.kde.kirigami 2.7 as Kirigami

ColumnLayout {
    id: detailsLayout

    property alias model: actionsRepeater.model

    property Component actionSettingsSound: RowLayout {
        QtControls.Button {
            icon.name: "media-playback-start"
        }
        QtControls.TextField {
            //text: "Oxygen-Sys-Trash-Emptied"//model.sound
            //textFormat: Text.PlainText
            //elide: Text.ElideMiddle
            //enabled: false
        }
        QtControls.Button {
            icon.name: "folder-open"
        }
    }

    property Component actionSettingsLogfile: RowLayout {
        QtControls.TextField {

        }
        QtControls.Button {
            icon.name: "folder-open"
        }
    }

    property Component actionSettingsExecute: RowLayout {
        QtControls.TextField {

        }
        QtControls.Button {
            icon.name: "folder-open"
        }
    }

    Repeater {
        id: actionsRepeater
        model: eventsColumn.actions

        RowLayout {
            Layout.fillWidth: true

            QtControls.CheckBox {
                id: actionCheck
                Layout.fillWidth: true
                text: modelData.label
                icon.name: modelData.icon
                checked: eventDelegate.isActionEnabled(modelData.key)
                onClicked: eventDelegate.setActionEnabled(modelData.key, checked)
                enabled: modelData.key !== "Popup" || showPopupsCheck.checked

                contentItem: RowLayout {
                    Item {
                        width: actionCheck.indicator.width
                    }

                    Kirigami.Icon {
                        source: actionCheck.icon.name
                        Layout.preferredWidth: Kirigami.Units.iconSizes.small
                        Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    }

                    QtControls.Label {
                        Layout.fillWidth: true
                        text: actionCheck.text
                        elide: Text.ElideRight
                    }
                }
            }

            Loader {
                sourceComponent: detailsLayout["actionSettings" + modelData.key]
            }
        }
    }
}
