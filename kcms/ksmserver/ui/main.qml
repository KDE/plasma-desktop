/*
    SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import QtQuick.Dialogs
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    implicitHeight: Kirigami.Units.gridUnit * 28
    implicitWidth: Kirigami.Units.gridUnit * 28

    headerPaddingEnabled: false // Let the InlineMessage touch the edges
    header: ColumnLayout {
        spacing : 0

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            position: Kirigami.InlineMessage.Position.Header
            type: kcm.error.length > 0 ? Kirigami.MessageType.Error : Kirigami.MessageType.Information
            visible: kcm.restartInSetupScreen || kcm.error.length > 0
            text: kcm.error.length > 0
            ? i18n("Failed to request restart to firmware setup: %1", kcm.error)
            : kcm.isUefi ? i18n("Next time the computer is restarted, it will enter the UEFI setup screen.")
            : i18n("Next time the computer is restarted, it will enter the firmware setup screen.")
            showCloseButton: true
            actions: Kirigami.Action {
                icon.name: "system-reboot"
                onTriggered: kcm.reboot();
                text: i18n("Restart Now")
            }
        }
    }

    Kirigami.FormLayout {

        Item {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18nc("@title:group", "General")
        }

        CheckBox {
            id: logoutScreenCheckbox
        Kirigami.FormData.label: i18nc("@label beginning of the logical sentence 'Ask for confirmation on shutdown, restart, and logout.'", "Ask for confirmation:")
            text: i18nc("@option:check end of the logical sentence 'Ask for confirmation on shutdown, restart, and logout.'", "On shutdown, restart, and logout")
            checked: kcm.settings.confirmLogout
            onToggled: kcm.settings.confirmLogout = checked
            KCM.SettingStateBinding {
                configObject: kcm.settings
                settingName: "confirmLogout"
            }
        }

        Item {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18nc("@title:group", "Session Restore")
        }

        ButtonGroup {
            buttons: [loginRestore, loginManual, loginEmpty]
        }
        RadioButton {
            id: loginRestore
            Kirigami.FormData.label: i18n("On login, launch apps that were open:")
            text: i18nc("@option:radio Automatic style of session restoration", "On last logout")
            checked: kcm.settings.loginMode === 0
            onToggled: kcm.settings.loginMode = 0
            KCM.SettingStateBinding {
                configObject: kcm.settings
                settingName: "loginMode"
            }
        }
        RowLayout {
            spacing: 0

            RadioButton {
                id: loginManual
                text: i18nc("@option:radio Manual style of session restoration", "When session was manually saved")
                checked: kcm.settings.loginMode === 1
                onToggled: kcm.settings.loginMode = 1
                KCM.SettingStateBinding {
                    configObject: kcm.settings
                    settingName: "loginMode"
                }
            }
            Kirigami.ContextualHelpButton {
                toolTipText: xi18nc("@info", "A <interface>Save Session</interface> button will appear in the <interface>Application Launcher</interface> menu. When you click it, Plasma will remember the apps that are open and restore them on the next login. Click it again to replace the set of remembered apps.")
            }
        }
        RadioButton {
            id: loginEmpty
            text: i18nc("@option:radio Here 'session' refers to the technical concept of session restoration, whereby the windows that were open on logout are re-opened on the next login", "Start with an empty session")
            checked: kcm.settings.loginMode === 2
            onToggled: kcm.settings.loginMode = 2
            KCM.SettingStateBinding {
                configObject: kcm.settings
                settingName: "loginMode"
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        RowLayout {
            Kirigami.FormData.label: i18n("Ignored applications:")

            spacing: 0

            TextField {
                Layout.preferredWidth: Kirigami.Units.gridUnit * 16
                text: kcm.settings.excludeApps
                enabled: !loginEmpty.checked
                // onTextEdited instead of onAccepted because otherwise the apply and
                // reset buttons won't work, since otherwise in many case no change will
                // be sent to the kconfigXt backend.
                onTextEdited: kcm.settings.excludeApps = text

                KCM.SettingStateBinding {
                    configObject: kcm.settings
                    settingName: "excludeApps"
                }
            }
            Kirigami.ContextualHelpButton {
                toolTipText: i18n("Write apps' desktop file identifier or, on X11, executable names here (separated by commas or colons, for example 'xterm:org.kde.konsole.desktop' or 'xterm,org.kde.konsole.desktop') to prevent them from autostarting along with other session-restored apps.")
            }
        }

        Item {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18nc("@title:group", "Firmware")
            visible: uefi.visible
        }

        Switch {
            id: uefi
            Kirigami.FormData.label: i18nc("@label:check part of a sentence: After next restart enter UEFI/Firmware setup screen", "After next restart:")
            text: kcm.isUefi ? i18nc("@option:check", "Enter UEFI setup screen")
                            : i18nc("@option:check", "Enter firmware setup screen")
            visible: kcm.canFirmwareSetup
            checked: kcm.restartInSetupScreen
            onToggled: kcm.restartInSetupScreen = checked
        }
    }
}
