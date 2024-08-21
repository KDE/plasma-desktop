/*
    SPDX-FileCopyrightText: 2024 Evgeny Chesnokov <echesnokov@astralinux.ru>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kquickcontrols as KQuickControls
import org.kde.kitemmodels as KItemModels
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.AbstractKCM {
    title: i18nc("@title", "Configure Switching")

    contentItem: Kirigami.FormLayout {

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18nc("@title:group", "Behavior")
        }

        QQC2.ComboBox {
            id: switchingPolicyCombobox
            Kirigami.FormData.label: i18nc("@label:listbox", "Switching layout affects:")
            textRole: "text"
            valueRole: "value"
            onActivated: kcm.keyboardSettings.switchMode = currentValue

            model: [
                {
                    "text": i18nc("@item:inlistbox", "All windows"),
                    "value": "Global",
                },
                {
                    "text": i18nc("@item:inlistbox", "All windows on current desktop"),
                    "value": "Desktop",
                },
                {
                    "text": i18nc("@item:inlistbox", "All windows of current application"),
                    "value": "WinClass",
                },
                {
                    "text": i18nc("@item:inlistbox", "Current window only"),
                    "value": "Window",
                },
            ]

            Component.onCompleted: selectCurrent()

            Connections {
                target: kcm?.keyboardSettings ?? undefined

                function onSwitchModeChanged(): void {
                    switchingPolicyCombobox.selectCurrent()
                }
            }

            function selectCurrent(): void {
                currentIndex = indexOfValue(kcm.keyboardSettings.switchMode);
            }

            KCM.SettingStateBinding {
                configObject: kcm.keyboardSettings
                settingName: "switchMode"
            }
        }


        QQC2.CheckBox {
            text: i18nc("@label:checkbox", "Show notification on layout change")
            checked: kcm.workspaceOptions.osdKbdLayoutChangedEnabled
            onToggled: kcm.workspaceOptions.osdKbdLayoutChangedEnabled = checked

            KCM.SettingStateBinding {
                configObject: kcm.workspaceOptions
                settingName: "osdKbdLayoutChangedEnabled"
            }
        }

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
            Kirigami.FormData.label: i18nc("@title:group", "Shortcuts")
        }

        KQuickControls.KeySequenceItem {
            id: alternativeShortcut
            Kirigami.FormData.label: i18nc("@option:textbox", "Change layout:")
            modifierlessAllowed: false
            modifierOnlyAllowed: true

            keySequence: kcm.shortcutHelper.alternativeShortcut
            onKeySequenceModified: kcm.shortcutHelper.alternativeShortcut = keySequence

            Binding {
                target: alternativeShortcut
                property: "keySequence"
                value: kcm.shortcutHelper.alternativeShortcut
            }

            KCM.SettingHighlighter {
                target: alternativeShortcut
                highlight: alternativeShortcut.keySequence !== kcm.shortcutHelper.defaultAlternativeShortcut()
            }
        }

        KQuickControls.KeySequenceItem {
            id: lastUsedShortcut
            Kirigami.FormData.label: i18nc("@option:textbox", "Switch to last used layout:")
            modifierlessAllowed: false
            modifierOnlyAllowed: true

            keySequence: kcm.shortcutHelper.lastUsedShortcut
            onCaptureFinished: kcm.shortcutHelper.lastUsedShortcut = keySequence

            Binding {
                target: lastUsedShortcut
                property: "keySequence"
                value: kcm.shortcutHelper.lastUsedShortcut
            }

            KCM.SettingHighlighter {
                target: lastUsedShortcut
                highlight: lastUsedShortcut.keySequence !== kcm.shortcutHelper.defaultLastUsedShortcut()
            }
        }

        ColumnLayout {
            spacing: Kirigami.Units.smallSpacing
            Kirigami.FormData.label: i18nc("@title:group", "Legacy Shortcuts")
            Kirigami.FormData.isSection: true

            Kirigami.ContextualHelpButton {
                Layout.alignment: Qt.AlignRight
                Layout.topMargin: -height
                // Make sure the button doesn't take size of its parent
                Layout.maximumHeight: -1
                toolTipText: xi18nc("@info", "Legacy X11 shortcuts allow finer control of modifier-only shortcuts and have slightly better support on X11 in edge cases. They can also be used on Wayland.<nl/><nl/>Setting these shortcuts will take you to the Key bindings page.")
            }

            Kirigami.Separator {
                Layout.fillWidth: true
            }
        }

        // Since "Main shortcut" has specific behavior (we push "KeyBindings" page) we can`t use standard KeySequenceItem
        // But this component looks exactly like KeySequenceItem
        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            Kirigami.FormData.label: i18nc("@option:textbox", "Change layout:")

            // TODO: SettingStateBinding and default value
            QQC2.Button {
                id: mainShortcutButton
                icon.name: "configure-symbolic"
                text: kcm.xkbOptionsModel.getShortcutName(__internal.mainShortcutGroupName)

                onClicked: {
                    if (!kcm.keyboardSettings.resetOldXkbOptions) {
                        kcm.keyboardSettings.resetOldXkbOptions = true;
                    }

                    kcm.push("KeyBindings.qml");
                    kcm.xkbOptionsModel.navigateToGroup(__internal.mainShortcutGroupName);
                }
            }

            QQC2.Button {
                icon.name: Qt.application.layoutDirection === Qt.LeftToRight ? "edit-clear-locationbar-rtl-symbolic" :
                                                                               "edit-clear-locationbar-ltr-symbolic"
                onClicked: kcm.xkbOptionsModel.clearXkbGroup(__internal.mainShortcutGroupName);
            }
        }

        // Since "3rd level shortcuts" has specific behavior (we push "KeyBindings" page) we can`t use standard KeySequenceItem
        // But this component looks exactly like KeySequenceItem
        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            Kirigami.FormData.label: i18nc("@option:textbox", "Alternative symbols:")

            // TODO: SettingStateBinding and default value
            QQC2.Button {
                id: lvl3lShortcutButton
                icon.name: "configure-symbolic"
                text: kcm.xkbOptionsModel.getShortcutName(__internal.lvl3ShortcutGroupName)

                onClicked: {
                    if (!kcm.keyboardSettings.resetOldXkbOptions) {
                        kcm.keyboardSettings.resetOldXkbOptions = true;
                    }


                    kcm.push("KeyBindings.qml");
                    kcm.xkbOptionsModel.navigateToGroup(__internal.lvl3ShortcutGroupName);
                }
            }

            QQC2.Button {
                icon.name: Qt.application.layoutDirection === Qt.LeftToRight ? "edit-clear-locationbar-rtl-symbolic" :
                                                                               "edit-clear-locationbar-ltr-symbolic"
                onClicked: kcm.xkbOptionsModel.clearXkbGroup(__internal.lvl3ShortcutGroupName)
            }

            Kirigami.ContextualHelpButton {
                toolTipText: i18nc("@info", "Modifier keys can be used to type additional characters, symbols, and diacritical marks.")
            }
        }
    }

    // Update main & 3dLvl shortcuts text
    Connections {
        target: kcm.xkbOptionsModel

        function onDataChanged(): void {
            mainShortcutButton.text = kcm.xkbOptionsModel.getShortcutName(__internal.mainShortcutGroupName);
            lvl3lShortcutButton.text = kcm.xkbOptionsModel.getShortcutName(__internal.lvl3ShortcutGroupName);
        }

        function onModelReset(): void {
            mainShortcutButton.text = kcm.xkbOptionsModel.getShortcutName(__internal.mainShortcutGroupName);
            lvl3lShortcutButton.text = kcm.xkbOptionsModel.getShortcutName(__internal.lvl3ShortcutGroupName);
        }
    }

    QtObject {
        id: __internal

        readonly property string mainShortcutGroupName: "grp"
        readonly property string lvl3ShortcutGroupName: "lv3"
    }
}
