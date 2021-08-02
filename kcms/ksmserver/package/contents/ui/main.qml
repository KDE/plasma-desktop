/*
    SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.10
import QtQuick.Controls 2.10
import QtQuick.Layouts 1.11
import org.kde.kirigami 2.13 as Kirigami
import QtQuick.Dialogs 1.3
import org.kde.desktopsession.private 1.0 
import org.kde.kcm 1.3 as KCM

KCM.SimpleKCM {
    id: root

    implicitHeight: Kirigami.Units.gridUnit * 28
    implicitWidth: Kirigami.Units.gridUnit * 28

    header: Kirigami.InlineMessage {
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

    Kirigami.FormLayout {
        CheckBox {
            Kirigami.FormData.label: i18n("General:")
            text: i18n("Confirm logout")
            checked: Settings.confirmLogout
            onToggled: Settings.confirmLogout = checked
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "confirmLogout"
            }
        }
        CheckBox {
            text: i18n("Offer shutdown options")
            checked: Settings.offerShutdown
            onToggled: Settings.offerShutdown = checked
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "offerShutdown"
            }
        }
        Item {
            Kirigami.FormData.isSection: true
        }

        ButtonGroup {
            buttons: [leaveEnd, leaveRestart, leaveOff]
        }
        RadioButton {
            id: leaveEnd
            Kirigami.FormData.label: i18n("Default leave option:")
            text: i18n("End current session")
            checked: Settings.shutdownType === 0
            onToggled: Settings.shutdownType = 0
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "shutdownType"
            }
        }
        RadioButton {
            id: leaveRestart
            text: i18n("Restart computer")
            checked: Settings.shutdownType === 1
            onToggled: Settings.shutdownType = 1
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "shutdownType"
            }
        }
        RadioButton {
            id: leaveOff
            text: i18n("Turn off computer")
            checked: Settings.shutdownType === 2
            onToggled: Settings.shutdownType = 2
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "shutdownType"
            }
        }
        Item {
            Kirigami.FormData.isSection: true
        }
        ButtonGroup {
            buttons: [loginRestore, loginManual, loginEmpty]
        }
        RadioButton {
            id: loginRestore
            Kirigami.FormData.label: i18n("When logging in:")
            text: i18n("Restore previous saved session")
            checked: Settings.loginMode === 0
            onToggled: Settings.loginMode = 0
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "loginMode"
            }
        }
        RadioButton {
            id: loginManual
            text: i18n("Restore manually saved session")
            checked: Settings.loginMode === 1
            onToggled: Settings.loginMode = 1
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "loginMode"
            }
        }
        RadioButton {
            id: loginEmpty
            text: i18n("Start with an empty session")
            checked: Settings.loginMode === 2
            onToggled: Settings.loginMode = 2
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "loginMode"
            }
        }
        Item {
            Kirigami.FormData.isSection: true
        }
        TextField {
            id: notRestoredApplications
            Kirigami.FormData.label: i18n("Don't restore these applications:")
            text: Settings.excludeApps
            // onTextEdited instead of onAccepted because otherwise the apply and
            // reset buttons won't work, since otherwise in many case no change will
            // be sent to the kconfigXt backend.
            onTextEdited: Settings.excludeApps = text
            ToolTip.text: i18n("Here you can enter a colon or comma separated list of applications that should not be saved in sessions, and therefore will not be started when restoring a session. For example 'xterm:konsole' or 'xterm,konsole'.")
            ToolTip.visible: notRestoredApplications.hovered
            ToolTip.delay: Kirigami.Units.toolTipDelay
            KCM.SettingStateBinding {
                configObject: Settings
                settingName: "excludeApps"
            }
        }
        Item {
            Kirigami.FormData.isSection: true
            visible: kcm.canFirmwareSetup
        }
        CheckBox {
            id: uefi
            text: i18n("Enter firmware setup screen on next restart")
            visible: kcm.canFirmwareSetup
            checked: kcm.restartInSetupScreen
            onToggled: kcm.restartInSetupScreen = checked
        }
    }
}
