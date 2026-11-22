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

    contentItem: Kirigami.Form {
        Kirigami.FormGroup {
            title: i18nc("@title:group", "Behavior")
            Kirigami.FormEntry {
                title: i18nc("@label:listbox", "Switching layout affects:")
                contentItem: QQC2.ComboBox {
                    id: switchingPolicyCombobox
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
            }
            Kirigami.FormEntry {
                title: i18nc("@option:check prefix, completes …Show OSD popup", "On layout change:")
                contentItem: QQC2.CheckBox {
                    text: i18nc("@option:check, completes On layout change…", "Show OSD popup")
                    checked: kcm.workspaceOptions.osdKbdLayoutChangedEnabled
                    onToggled: kcm.workspaceOptions.osdKbdLayoutChangedEnabled = checked

                    KCM.SettingStateBinding {
                        configObject: kcm.workspaceOptions
                        settingName: "osdKbdLayoutChangedEnabled"
                    }
                }
            }
        }
        Kirigami.FormGroup {
            title: i18nc("@title:group", "Shortcuts")
            Kirigami.FormEntry {
                title: i18nc("@option:textbox", "Change layout:")
                contentItem: KQuickControls.KeySequenceItem {
                    id: alternativeShortcut
                    patterns: KQuickControls.ShortcutPattern.Modifier | KQuickControls.ShortcutPattern.ModifierAndKey

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
            }
            Kirigami.FormEntry {
                title: i18nc("@option:textbox", "Switch to last used layout:")
                contentItem: KQuickControls.KeySequenceItem {
                    id: lastUsedShortcut
                    patterns: KQuickControls.ShortcutPattern.Modifier | KQuickControls.ShortcutPattern.ModifierAndKey

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
            }
        }
        Kirigami.FormGroup {
            title: i18nc("@title:group", "Legacy Shortcuts")
            Kirigami.FormEntry {
                title: i18nc("@option:textbox", "Change layout:")
                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

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
                        icon.name: Application.layoutDirection === Qt.LeftToRight ? "edit-clear-locationbar-rtl-symbolic" :
                        "edit-clear-locationbar-ltr-symbolic"
                        onClicked: kcm.xkbOptionsModel.clearXkbGroup(__internal.mainShortcutGroupName);
                    }
                }
                trailingItems: [
                    Kirigami.ContextualHelpButton {
                        toolTipText: xi18nc("@info", "Legacy shortcuts allow finer control of modifier-only shortcuts.<nl/><nl/>Setting this shortcut will take you to the Key bindings page.")
                    }
                ]
            }
            Kirigami.FormEntry {
                title: i18nc("@option:textbox", "Alternative symbols:")
                contentItem: RowLayout {
                    spacing: Kirigami.Units.smallSpacing

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
                        icon.name: Application.layoutDirection === Qt.LeftToRight ? "edit-clear-locationbar-rtl-symbolic" :
                        "edit-clear-locationbar-ltr-symbolic"
                        onClicked: kcm.xkbOptionsModel.clearXkbGroup(__internal.lvl3ShortcutGroupName)
                    }
                }
                trailingItems: [
                    Kirigami.ContextualHelpButton {
                        toolTipText: xi18nc("@info", "Modifier keys can be used to type additional characters, symbols, and diacritical marks.<nl/><nl/>Setting this shortcut will take you to the Key bindings page.")
                    }
                ]
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
