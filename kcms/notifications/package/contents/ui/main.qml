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
import QtQuick.Window 2.2
//import QtQuick.Dialogs 1.0 as QtDialogs
import QtQuick.Controls 2.3 as QtControls
import org.kde.kirigami 2.4 as Kirigami
//import org.kde.kconfig 1.0 // for KAuthorized
import org.kde.kcm 1.2 as KCM

KCM.SimpleKCM {
    KCM.ConfigModule.quickHelp: i18n("This module lets you manage application and system notifications.")
    KCM.ConfigModule.buttons: KCM.ConfigModule.Help/* | KCM.ConfigModule.Default*/ | KCM.ConfigModule.Apply

    implicitHeight: 550 // HACK FIXME

    Kirigami.FormLayout {
        RowLayout {
            Kirigami.FormData.label: i18n("Do not disturb mode:")

            QtControls.CheckBox {
                id: dndTimeCheck
                text: i18nc("Enable between hh:mm and hh:mm", "Automatically enable")
            }

            QtControls.Button {
                text: i18nc("Set times for automatic do not disturb mode", "Set Times...")
                icon.name: "preferences-system-time"
                onClicked: kcm.push("DndTimePage.qml")
                enabled: dndTimeCheck.checked
            }
        }

        QtControls.CheckBox {
            text: i18nc("Apps can enable do not disturb mode", "Applications can enable")
            checked: true
        }

        RowLayout {
            QtControls.Label {
                text: i18n("Keyboard Shortcut:")
            }

            // TODO keysequence thing
            QtControls.Button {
                icon.name: "configure"
                text: i18n("Meta+N")
            }

            QtControls.Button {
                icon.name: "edit-clear"
            }
        }

        ColumnLayout {
            visible: activitiesDndRepeater.count > 1

            QtControls.Label {
                text: i18n("Automatically enable in the following activities:")
            }

            Repeater {
                id: activitiesDndRepeater
                model: kcm.activitiesModel

                QtControls.CheckBox {
                    id: activityCheck

                    text: model.name
                    icon.name: model.iconSource || "preferences-activities"

                    // FIXME make icons work in QQC2 desktop style CheckBox
                    contentItem: RowLayout {
                        Kirigami.Icon {
                            Layout.leftMargin: indicator.width + Kirigami.Units.smallSpacing
                            source: activityCheck.icon.name
                            Layout.preferredWidth: Kirigami.Units.iconSizes.small
                            Layout.preferredHeight: Kirigami.Units.iconSizes.small
                        }

                        QtControls.Label {
                            text: activityCheck.text
                        }
                    }
                }
            }
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        QtControls.CheckBox {
            Kirigami.FormData.label: i18n("Critical notifications:")
            text: i18n("Show in do not disturb mode")
            checked: true
        }

        QtControls.CheckBox {
            text: i18n("Keep always on top")
            checked: true
        }

        QtControls.CheckBox {
            Kirigami.FormData.label: i18n("Low priority notifications:")
            text: i18n("Show popup")
        }

        QtControls.ButtonGroup {
            id: positionGroup
            buttons: [positionCloseToPanel, positionCustomPosition]
        }

        QtControls.RadioButton {
            id: positionCloseToPanel
            Kirigami.FormData.label: i18n("Popup position:")
            text: i18n("Near the notification icon") // "widget"
            checked: true
        }

        RowLayout {
            QtControls.RadioButton {
                id: positionCustomPosition
                text: i18n("Custom Position")
            }
            QtControls.Button {
                text: i18n("Choose...")
                onClicked: kcm.push("PopupPositionPage.qml")
                enabled: positionCustomPosition.checked
            }
        }

        QtControls.ComboBox {
            Layout.fillWidth: false
            Kirigami.FormData.label: i18n("Hide popup after:")
            textRole: "label"
            model: [
                {label: i18n("5 seconds"), value: 5},
                {label: i18n("7 seconds"), value: 7},
                {label: i18n("10 seconds"), value: 10},
                {label: i18n("15 seconds"), value: 15},
                {label: i18n("30 seconds"), value: 30},
                {label: i18n("1 minute"), value: 60}
            ]
            // FIXME proper sizing, especially for the popup
            Layout.maximumWidth: Kirigami.Units.gridUnit * 6
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        QtControls.CheckBox {
            Kirigami.FormData.label: i18n("Application progress:")
            text: i18n("Show in task manager")
            checked: true
        }

        QtControls.CheckBox {
            id: applicationJobsEnabledCheck
            text: i18n("Show popup")
            checked: true
        }

        RowLayout { // HACK just for indentation
            QtControls.CheckBox {
                Layout.leftMargin: indicator.width
                text: i18n("Hide when running")
                enabled: applicationJobsEnabledCheck.checked
            }
        }

        QtControls.CheckBox {
            Kirigami.FormData.label: i18n("Notification badges:")
            text: i18n("Show in task manager")
            checked: true
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        QtControls.Button {
            text: i18n("Application Settings")
            icon.name: "preferences-desktop-notification"
            onClicked: kcm.push("SourcesPage.qml")
        }
    }
}
