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
                ? i18nc("@info:status inlinemessage", "Failed to request restart to firmware setup: %1", kcm.error)
                : kcm.isUefi ? i18nc("@info:status inlinemessage", "Next time the computer is restarted, it will enter UEFI firmware settings.")
                             : i18nc("@info:status inlinemessage", "Next time the computer is restarted, it will enter the firmware setup screen.")
            showCloseButton: true
            actions: Kirigami.Action {
                icon.name: "system-reboot"
                onTriggered: kcm.reboot();
                text: i18nc("@action:button", "Restart Now")
            }
        }
    }

    Kirigami.Form {
        Kirigami.FormGroup {
            Kirigami.FormEntry {
                title: i18nc("@label beginning of the logical sentence 'Ask for confirmation on shutdown, restart, and logout.'", "Ask for confirmation:")
                contentItem: CheckBox {
                    id: logoutScreenCheckbox
                    text: i18nc("@option:check end of the logical sentence 'Ask for confirmation on shutdown, restart, and logout.'", "On shutdown, restart, and logout")
                    checked: kcm.settings.confirmLogout
                    onToggled: kcm.settings.confirmLogout = checked
                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "confirmLogout"
                    }
                }
            }

            Kirigami.FormEntry {
                visible: kcm.canFirmwareSetup
                title: i18nc("@label:check part of a sentence: After next restart enter UEFI/Firmware setup screen", "After next restart:")
                contentItem: Switch {
                    id: uefi
                    text: kcm.isUefi ? i18nc("@option:check", "Enter UEFI firmware settings")
                                    : i18nc("@option:check", "Enter firmware setup screen")
                    checked: kcm.restartInSetupScreen
                    onToggled: kcm.restartInSetupScreen = checked
                }
            }
        }

        Kirigami.FormGroup {
            title: i18nc("@title:group", "Session Restore")

            ButtonGroup {
                buttons: [loginRestore, loginManual, loginEmpty]
            }

            Kirigami.FormEntry {
                title: i18nc("@title:group prefix for radiobuttons", "On login:")
                contentItem: RadioButton {
                    id: loginRestore
                    text: i18nc("@option:radio Automatic style of session restoration", "Restore apps from previous session")
                    checked: kcm.settings.loginMode === 0
                    onToggled: kcm.settings.loginMode = 0
                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "loginMode"
                    }
                }

                trailingItems: [
                    Kirigami.ContextualHelpButton {
                        toolTipText: i18nc("@info", "When you log back in, Plasma will automatically reopen applications and windows that were running when you last logged out.")
                    }
                ]
            }

            Kirigami.FormEntry {
                contentItem: RadioButton {
                    id: loginManual
                    text: i18nc("@option:radio Manual style of session restoration", "Restore manually saved session")
                    checked: kcm.settings.loginMode === 1
                    onToggled: kcm.settings.loginMode = 1
                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "loginMode"
                    }
                }

                trailingItems: [
                    Kirigami.ContextualHelpButton {
                        toolTipText: xi18nc("@info", "Adds a <interface>Save Session</interface> button to the <interface>Application Launcher</interface> menu. When you click it, Plasma will remember the open apps and restore them on the next login. Click it again to replace the set of remembered apps.")

                    }
                ]
            }

            Kirigami.FormEntry {
                contentItem: RadioButton {
                    id: loginEmpty
                    text: i18nc("@option:radio", "Start with an empty session")
                    checked: kcm.settings.loginMode === 2
                    onToggled: kcm.settings.loginMode = 2
                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "loginMode"
                    }
                }
            }

            Kirigami.FormEntry {
                contentItem: Button {
                    icon.name: "view-visible-off"
                    text: i18nc("@action:button", "Configure Ignored Applications…")
                    enabled: !loginEmpty.checked
                    onClicked: {
                        let dialog = ignoredAppsComponent.createObject(root) as Kirigami.Dialog;
                        dialog.open();
                    }
                    KCM.SettingStateBinding {
                        configObject: kcm.settings
                        settingName: "excludeApps"
                    }
                }

                trailingItems: [
                    Kirigami.ContextualHelpButton {
                        toolTipText: i18nc("@info", "Prevent specified applications from autostarting with other session-restored apps. Use each application's desktop file identifier (e.g. org.kde.kate.desktop) or, for XWayland apps, its executable name (e.g. xterm).")
                    }
                ]
            }

            // Adapted from Spell Check KCM
            Component {
                id: ignoredAppsComponent
                Kirigami.Dialog {
                    id: dialog
                    title: i18nc("@title:dialog", "Ignored Applications")

                    width: Kirigami.Units.gridUnit * 24

                    readonly property var excludedApps: kcm.settings.excludeApps
                        .split(/[:,]/)
                        .map(app => app.trim())
                        .filter(app => app.length > 0)

                    onRejected: dialog.close()

                    footerLeadingComponent: Kirigami.ActionTextField {
                        id: newAppInput
                        placeholderText: i18nc("@info:placeholder", "Enter application (e.g. org.kde.kate.desktop or xterm)")
                        Layout.fillWidth: true
                        rightActions: [
                            Kirigami.Action {
                                id: addAction
                                enabled: newAppInput.text.trim().length > 0
                                text: i18nc("@action:button", "Ignore this Application")
                                Accessible.description: newAppInput.text.trim().length > 0
                                    ? i18nc("@info:whatsthis", "Add %1 to the list of ignored applications", newAppInput.text.trim())
                                    : i18nc("@info:whatsthis", "Add an application to the list of ignored applications")
                                tooltip: text
                                icon.name: "list-add"
                                onTriggered: {
                                    const appName = newAppInput.text.trim();
                                    if (appName.length > 0 && !dialog.excludedApps.includes(appName)) {
                                        kcm.settings.excludeApps = dialog.excludedApps.concat(appName).join(":");
                                        appsListView.positionViewAtEnd();
                                    }
                                    newAppInput.clear();
                                }
                            }
                        ]
                        onAccepted: {
                            if (addAction.enabled) {
                                addAction.triggered();
                            }
                        }
                    }

                    ListView {
                        id: appsListView
                        model: dialog.excludedApps
                        delegate: ItemDelegate {
                            id: delegate
                            required property string modelData

                            width: ListView.view.width
                            text: delegate.modelData

                            contentItem: Kirigami.TitleSubtitleWithActions {
                                title: delegate.text
                                actions: [
                                    Kirigami.Action {
                                        text: i18nc("@action:button", "Remove")
                                        Accessible.description: i18nc("@info:whatsthis", "Remove %1 from the list of ignored applications", delegate.modelData)
                                        tooltip: text
                                        icon.name: "list-remove"
                                        displayHint: Kirigami.DisplayHint.IconOnly
                                        onTriggered: {
                                            kcm.settings.excludeApps = dialog.excludedApps
                                                .filter(value => value !== delegate.modelData)
                                                .join(":");
                                        }
                                    }
                                ]
                            }
                        }
                    }
                }
            }
        }
    }
}
