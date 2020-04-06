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
import org.kde.kirigami 2.4 as Kirigami
import org.kde.kquickcontrols 2.0 as KQuickControls
import org.kde.kcm 1.3 as KCM

import org.kde.notificationmanager 1.0 as NotificationManager

KCM.SimpleKCM {
    id: root
    KCM.ConfigModule.quickHelp: i18n("This module lets you manage application and system notifications.")

    // Sidebar on SourcesPage is 1/3 of the width at a minimum of 12, so assume 3 * 12 = 36 as preferred
    implicitWidth: Kirigami.Units.gridUnit * 36

    readonly property string ourServerVendor: "KDE"
    readonly property string ourServerName: "Plasma"

    readonly property NotificationManager.ServerInfo currentOwnerInfo: NotificationManager.Server.currentOwner

    readonly property bool notificationsAvailable: currentOwnerInfo.status === NotificationManager.ServerInfo.Running
        && currentOwnerInfo.vendor === ourServerVendor && currentOwnerInfo.name === ourServerName

    function openSourcesSettings() {
        // TODO would be nice to re-use the current SourcesPage instead of pushing a new one that lost all state
        // but there's no pageAt(index) method in KConfigModuleQml
        kcm.push("SourcesPage.qml");
    }

    Kirigami.FormLayout {
        Kirigami.InlineMessage {
            Kirigami.FormData.isSection: true
            Layout.fillWidth: true
            type: Kirigami.MessageType.Error
            text: i18n("Could not find a 'Notifications' widget which is required for displaying notifications.");
            visible: currentOwnerInfo.status === NotificationManager.ServerInfo.NotRunning
        }

        Kirigami.InlineMessage {
            Kirigami.FormData.isSection: true
            Layout.fillWidth: true
            type: Kirigami.MessageType.Information
            text: {
                if (currentOwnerInfo.vendor && currentOwnerInfo.name) {
                    return i18nc("Vendor and product name",
                                 "Notifications are currently provided by '%1 %2' instead of Plasma.",
                                 currentOwnerInfo.vendor, currentOwnerInfo.name);
                }

                return i18n("Notifications are currently not provided by Plasma.");
            }
            visible: root.currentOwnerInfo.status === NotificationManager.ServerInfo.Running
                && (currentOwnerInfo.vendor !== root.ourServerVendor || currentOwnerInfo.name !== root.ourServerName)
        }

        QtControls.CheckBox {
            Kirigami.FormData.label: i18n("Do Not Disturb mode:")
            text: i18nc("Do not disturb when screens are mirrored", "Enable when screens are mirrored")
            checked: kcm.dndSettings.whenScreensMirrored
            onClicked: kcm.dndSettings.whenScreensMirrored = checked

            KCM.SettingStateBinding {
                configObject: kcm.dndSettings
                settingName: "WhenScreensMirrored"
                extraEnabledConditions: root.notificationsAvailable
            }
        }

        QtControls.CheckBox {
            text: i18n("Show critical notifications")
            checked: kcm.notificationSettings.criticalInDndMode
            onClicked: kcm.notificationSettings.criticalInDndMode = checked

            KCM.SettingStateBinding {
                configObject: kcm.notificationSettings
                settingName: "CriticalInDndMode"
                extraEnabledConditions: root.notificationsAvailable
            }
        }

        RowLayout {
            enabled: root.notificationsAvailable

            QtControls.Label {
                text: i18nc("Turn do not disturb mode on/off with keyboard shortcut", "Toggle with:")
            }

            KQuickControls.KeySequenceItem {
                keySequence: kcm.toggleDoNotDisturbShortcut
                onKeySequenceChanged: kcm.toggleDoNotDisturbShortcut = keySequence
            }
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        QtControls.CheckBox {
            Kirigami.FormData.label: i18n("Critical notifications:")
            text: i18n("Always keep on top")
            checked: kcm.notificationSettings.criticalAlwaysOnTop
            onClicked: kcm.notificationSettings.criticalAlwaysOnTop = checked

            KCM.SettingStateBinding {
                configObject: kcm.notificationSettings
                settingName: "CriticalAlwaysOnTop"
                extraEnabledConditions: root.notificationsAvailable
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QtControls.CheckBox {
            Kirigami.FormData.label: i18n("Low priority notifications:")
            text: i18n("Show popup")
            checked: kcm.notificationSettings.lowPriorityPopups
            onClicked: kcm.notificationSettings.lowPriorityPopups = checked

            KCM.SettingStateBinding {
                configObject: kcm.notificationSettings
                settingName: "LowPriorityPopups"
                extraEnabledConditions: root.notificationsAvailable
            }
        }

        QtControls.CheckBox {
            text: i18n("Show in history")
            checked: kcm.notificationSettings.lowPriorityHistory
            onClicked: kcm.notificationSettings.lowPriorityHistory = checked

            KCM.SettingStateBinding {
                configObject: kcm.notificationSettings
                settingName: "LowPriorityHistory"
                extraEnabledConditions: root.notificationsAvailable
            }
        }

        QtControls.ButtonGroup {
            id: positionGroup
            buttons: [positionCloseToWidget, positionCustomPosition]
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        QtControls.RadioButton {
            id: positionCloseToWidget
            Kirigami.FormData.label: i18n("Popup:")
            text: i18nc("Popup position near notification plasmoid", "Show near notification icon") // "widget"
            checked: kcm.notificationSettings.popupPosition === NotificationManager.Settings.CloseToWidget
                // Force binding re-evaluation when user returns from position selector
                + kcm.currentIndex * 0
            onClicked: kcm.notificationSettings.popupPosition = NotificationManager.Settings.CloseToWidget

            KCM.SettingStateBinding {
                configObject: kcm.notificationSettings
                settingName: "PopupPosition"
                extraEnabledConditions: root.notificationsAvailable
            }
        }

        RowLayout {
            spacing: 0
            enabled: positionCloseToWidget.enabled

            QtControls.RadioButton {
                id: positionCustomPosition
                checked: kcm.notificationSettings.popupPosition !== NotificationManager.Settings.CloseToWidget
                    + kcm.currentIndex * 0
                activeFocusOnTab: false

                MouseArea {
                    anchors.fill: parent
                    onClicked: positionCustomButton.clicked()
                }

                KCM.SettingStateBinding {
                    configObject: kcm.notificationSettings
                    settingName: "PopupPosition"
                    extraEnabledConditions: root.notificationsAvailable
                }

            }
            QtControls.Button {
                id: positionCustomButton
                text: i18n("Choose Custom Position...")
                icon.name: "preferences-desktop-display"
                onClicked: kcm.push("PopupPositionPage.qml")
            }
        }

        TextMetrics {
            id: timeoutSpinnerMetrics
            font: timeoutSpinner.font
            text: i18np("%1 second", "%1 seconds", 888)
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        RowLayout {
            QtControls.Label {
                text: i18nc("Part of a sentence like, 'Hide popup after n seconds'", "Hide after:")
            }

            QtControls.SpinBox {
                id: timeoutSpinner
                Layout.preferredWidth: timeoutSpinnerMetrics.width + leftPadding + rightPadding
                from: 1000 // 1 second
                to: 120000 // 2 minutes
                stepSize: 1000
                value: kcm.notificationSettings.popupTimeout
                editable: true
                valueFromText: function(text, locale) {
                    return parseInt(text) * 1000;
                }
                textFromValue: function(value, locale) {
                    return i18np("%1 second", "%1 seconds", Math.round(value / 1000));
                }
                onValueModified: kcm.notificationSettings.popupTimeout = value

                KCM.SettingStateBinding {
                    configObject: kcm.notificationSettings
                    settingName: "PopupTimeout"
                    extraEnabledConditions: root.notificationsAvailable
                }
            }
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        QtControls.CheckBox {
            Kirigami.FormData.label: i18n("Application progress:")
            text: i18n("Show in task manager")
            checked: kcm.jobSettings.inTaskManager
            onClicked: kcm.jobSettings.inTaskManager = checked

            KCM.SettingStateBinding {
                configObject: kcm.jobSettings
                settingName: "InTaskManager"
            }
        }

        QtControls.CheckBox {
            id: applicationJobsEnabledCheck
            text: i18nc("Show application jobs in notification widget", "Show in notifications")
            checked: kcm.jobSettings.inNotifications
            onClicked: kcm.jobSettings.inNotifications = checked

            KCM.SettingStateBinding {
                configObject: kcm.jobSettings
                settingName: "InNotifications"
            }
        }

        RowLayout { // just for indentation
            QtControls.CheckBox {
                Layout.leftMargin: mirrored ? 0 : indicator.width
                Layout.rightMargin: mirrored ? indicator.width : 0
                text: i18nc("Keep application job popup open for entire duration of job", "Keep popup open during progress")
                checked: kcm.jobSettings.permanentPopups
                onClicked: kcm.jobSettings.permanentPopups = checked

                KCM.SettingStateBinding {
                    configObject: kcm.jobSettings
                    settingName: "PermanentPopups"
                    extraEnabledConditions: applicationJobsEnabledCheck.checked
                }
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QtControls.CheckBox {
            Kirigami.FormData.label: i18n("Notification badges:")
            text: i18n("Show in task manager")
            checked: kcm.badgeSettings.inTaskManager
            onClicked: kcm.badgeSettings.inTaskManager = checked

            KCM.SettingStateBinding {
                configObject: kcm.badgeSettings
                settingName: "InTaskManager"
            }
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }

        QtControls.Button {
            Kirigami.FormData.label: i18n("Applications:")
            text: i18n("Configure...")
            icon.name: "configure"
            enabled: root.notificationsAvailable
            onClicked: root.openSourcesSettings()
        }

        Connections {
            target: kcm
            onFirstLoadDone: {
                if (kcm.initialDesktopEntry || kcm.initialNotifyRcName) {
                        root.openSourcesSettings();
                }
            }
        }
    }
}
