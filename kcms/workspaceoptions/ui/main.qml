/*
    SPDX-FileCopyrightText: 2018 Furkan Tokac <furkantokac34@gmail.com>
    SPDX-FileCopyrightText: 2019 Nate Graham <nate@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM
import org.kde.kwindowsystem

import org.kde.plasma.workspaceoptions.kcm

KCM.SimpleKCM {
    implicitWidth: Kirigami.Units.gridUnit * 40

    headerPaddingEnabled: false // Let the InlineMessage touch the edges
    header: Kirigami.InlineMessage {
        id: primarySelectionRebootMessage
        position: Kirigami.InlineMessage.Position.Header
        type: Kirigami.MessageType.Information
        text: i18nc("@info:status inlinemessage", "The system must be restarted before changes to the middle-click paste setting can take effect.")
        visible: false
        showCloseButton: true
        actions: [
            Kirigami.Action {
                icon.name: "system-reboot"
                text: i18nc("@action:button", "Restart")
                onTriggered: kcm.requestReboot();
            }
        ]
        Connections {
            target: kcm
            function onPrimarySelectionOptionSaved() {
                primarySelectionRebootMessage.visible = true;
            }
        }
    }

    Kirigami.Form {
        Layout.fillWidth: true
        Kirigami.FormGroup {
            title: i18nc("@title:group", "Plasma")
            // Visual behavior settings
            Kirigami.FormEntry {
                title: i18nc("Part of the sentence 'Allow Plasma to show panel and widget tooltips'", "Allow Plasma to show:")
                contentItem: QQC2.CheckBox {
                    id: tooltipDisablerCheckbox
                    text: i18nc("Part of the sentence 'Allow Plasma to show panel and widget tooltips'", "Panel and widget tooltips")
                    checked: kcm.plasmaSettings.delay > 0
                    onCheckedChanged: kcm.plasmaSettings.delay = (checked ? 0.7 : -1)

                    KCM.SettingStateBinding {
                        configObject: kcm.plasmaSettings
                        settingName: "delay"
                    }
                }

                trailingItems: Kirigami.ContextualHelpButton {
                    toolTipText: i18nc("@info:whatsthis contextualhelpbutton tooltip", "Allows all Plasma panel and desktop widgets to show descriptive tooltips when hovered with the pointer. This setting has no effect on the small tooltips displayed when hovering over individual buttons and other user interface elements.")
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: osdDisablerCheckbox
                    text: i18nc("Part of the sentence 'Allow Plasma to show OSD popups for status changes'", "OSD popups for status changes")
                    checked: kcm.plasmaSettings.osdEnabled
                    onCheckedChanged: kcm.plasmaSettings.osdEnabled = checked

                    KCM.SettingStateBinding {
                        configObject: kcm.plasmaSettings
                        settingName: "osdEnabled"
                    }
                }
                trailingItems: Kirigami.ContextualHelpButton {
                    Layout.alignment: Qt.AlignRight
                    toolTipText: i18nc("@info:whatsthis contextualhelpbutton tooltip", "Allows all Plasma widgets to show on-screen display (OSD) popups for changes like volume and brightness level, or audio device switching.")
                }
            }
        }


        Kirigami.FormGroup {
            title: i18nc("@title:group", "Scrolling")

            QQC2.ButtonGroup { id: scrollHandleBehaviorGroup }

            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    Kirigami.FormData.label: i18nc("@title:group prefix radiobutton group", "Clicking in scrollbar track:")
                    text: i18nc("@radio part of a complete sentence: 'Clicking in scrollbar track scrolls to the clicked location'", "Scrolls to the clicked location")
                    checked: !kcm.globalsSettings.scrollbarLeftClickNavigatesByPage
                    onToggled: kcm.globalsSettings.scrollbarLeftClickNavigatesByPage = false
                    QQC2.ButtonGroup.group: scrollHandleBehaviorGroup

                    KCM.SettingStateBinding {
                        configObject: kcm.globalsSettings
                        settingName: "scrollbarLeftClickNavigatesByPage"
                        extraEnabledConditions: scrollbarLeftClickNavigatesByPage.enabled
                    }
                }
            }

            Kirigami.FormEntry {
                subtitle: i18nc("@info:usagetip", "Middle-click to scroll to clicked location")
                contentItem: QQC2.RadioButton {
                    id: scrollbarLeftClickNavigatesByPage
                    text: i18nc("@radio part of a complete sentence: 'Clicking in scrollbar track scrolls one page up or down'", "Scrolls one page up or down")
                    Accessible.description: scrollbarLeftClickNavigatesByPageHelperText.text
                    checked: kcm.globalsSettings.scrollbarLeftClickNavigatesByPage
                    onToggled: kcm.globalsSettings.scrollbarLeftClickNavigatesByPage = true
                    QQC2.ButtonGroup.group: scrollHandleBehaviorGroup

                    KCM.SettingStateBinding {
                        configObject: kcm.globalsSettings
                        settingName: "scrollbarLeftClickNavigatesByPage"
                    }
                }
            }

            Kirigami.FormSeparator {}

            Kirigami.FormEntry {
                contentItem: QQC2.CheckBox {
                    id: smoothScrollingCheckbox
                    text: i18nc("@option:check", "Prefer smooth scrolling")
                    checked: kcm.globalsSettings.smoothScroll
                    onToggled: kcm.globalsSettings.smoothScroll = checked

                    KCM.SettingStateBinding {
                        configObject: kcm.globalsSettings
                        settingName: "smoothScroll"
                    }
                }
                trailingItems: Kirigami.ContextualHelpButton {
                    Layout.alignment: Qt.AlignRight
                    toolTipText: i18nc("@info:tooltip", "This setting enables or disables animated transitions when scrolling with a mouse wheel or the keyboard. Some applications may not honor this setting because they either do not support smooth scrolling, or have their own setting for enabling and disabling it.")
                }
            }
        }

        Kirigami.FormGroup {
            title: i18nc("@title:group", "Clicking")

            Kirigami.FormEntry {
                title: i18nc("part of a sentence: 'Clicking files or folders [opens them/selects them]'", "Clicking files or folders:")
                subtitle: doubleClick.Accessible.description
                contentItem: QQC2.RadioButton {
                    id: doubleClick
                    text: i18nc("part of a sentence: 'Clicking files or folders selects them'", "Selects them")
                    Accessible.description: i18nc("@info:usagetip", "Open by double-clicking instead")
                    checked: !kcm.globalsSettings.singleClick
                    onToggled: kcm.globalsSettings.singleClick = false
                    QQC2.ButtonGroup.group: singleClickGroup

                    KCM.SettingStateBinding {
                        configObject: kcm.globalsSettings
                        settingName: "singleClick"
                        extraEnabledConditions: singleClick.enabled
                    }
                }
            }

            Kirigami.FormEntry {
                subtitle: singleClick.Accessible.description
                contentItem: QQC2.RadioButton {
                    id: singleClick
                    text: i18nc("part of a sentence: 'Clicking files or folders opens them'", "Opens them")
                    Accessible.description: i18nc("@info:usagetip", "Select by clicking on item's selection marker")
                    checked: kcm.globalsSettings.singleClick
                    onToggled: kcm.globalsSettings.singleClick = true
                    QQC2.ButtonGroup.group: singleClickGroup

                    KCM.SettingStateBinding {
                        configObject: kcm.globalsSettings
                        settingName: "singleClick"
                    }
                }
            }

            Kirigami.FormSeparator {
                visible: KWindowSystem.isPlatformWayland
            }

            Kirigami.FormEntry {
                title: i18nc("@label for checkbox, part of a complete sentence: 'Middle-click pastes selected text'", "Middle-click:")
                visible: KWindowSystem.isPlatformWayland
                contentItem: QQC2.CheckBox {
                    id: primarySelectionRadio
                    text: i18nc("@option:check part of a complete sentence: 'Middle click pastes selected text'", "Pastes selected text")
                    checked: kcm.kwinSettings.primarySelection
                    onToggled: kcm.kwinSettings.primarySelection = checked

                    KCM.SettingStateBinding {
                        configObject: kcm.kwinSettings
                        settingName: "primarySelection"
                    }
                }
            }
            // There is no label for what middle-clicking does when using the
            // "click to zoom the handle" behavior because Qt doesn't invert the
            // middle-click functionality when using this; see
            // https://bugreports.qt.io/browse/QTBUG-80728

            Kirigami.FormSeparator {}

            Kirigami.FormEntry {
                title: i18nc("@label:spinbox", "Double-click interval:")
                contentItem: ColumnLayout {
                    Kirigami.FormData.buddyFor: spinbox
                    spacing: Kirigami.Units.smallSpacing

                    RowLayout {
                        spacing: Kirigami.Units.smallSpacing

                        QQC2.SpinBox {
                            id: spinbox
                            from: 100
                            to: 2000
                            stepSize: 100

                            validator: IntValidator {
                                bottom: Math.min(spinbox.from, spinbox.to)
                                top: Math.max(spinbox.from, spinbox.to)
                            }

                            textFromValue: (value, locale) => {
                                return i18ncp("short for millisecond(s)", "%1 ms", "%1 ms", value)
                            }

                            valueFromText: (text, locale) => {
                                return Number.fromLocaleString(locale, text.replace(i18nc("short for millisecond(s)", "ms"), ""))
                            }

                            onValueModified: kcm.globalsSettings.doubleClickInterval = value

                            value: kcm.globalsSettings.doubleClickInterval

                            KCM.SettingStateBinding {
                                configObject: kcm.globalsSettings
                                settingName: "doubleClickInterval"
                            }
                        }

                        Kirigami.ContextualHelpButton {
                            Layout.alignment: Qt.AlignRight
                            toolTipText: i18nc("@info:whatsthis contextualhelpbutton tooltip", "Two clicks within this duration are considered a double-click. Some applications may not honor this setting.")
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: Kirigami.Units.smallSpacing

                        QQC2.Label {
                            Layout.fillWidth: true
                            Layout.preferredWidth: Kirigami.Units.gridUnit * 16
                            text: i18nc("Test double-click speed", "Double-click the folder to test. If it does not change its appearance, increase double-click interval.")
                            textFormat: Text.PlainText
                            wrapMode: Text.WordWrap
                            font: Kirigami.Theme.smallFont
                        }

                        QQC2.Frame {
                            Layout.preferredWidth: Kirigami.Units.iconSizes.large
                            Layout.preferredHeight: Kirigami.Units.iconSizes.large
                            Kirigami.Theme.colorSet: Kirigami.Theme.View

                            Kirigami.Icon {
                                id: doubleClickTestIcon
                                property bool checked: false
                                anchors.fill: parent
                                source: checked ? "folder-open" : "folder"
                            }

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true // for onExited to properly work.
                                onPressed: {
                                    if (testDoubleClickTimer.running) {
                                        doubleClickTestIcon.checked = !doubleClickTestIcon.checked
                                        testDoubleClickTimer.stop()
                                    } else {
                                        testDoubleClickTimer.start()
                                    }
                                }
                                onExited: testDoubleClickTimer.stop()
                            }

                            Timer {
                                id: testDoubleClickTimer
                                interval: kcm.globalsSettings.doubleClickInterval
                            }
                        }
                    }
                }
            }
        }


        Kirigami.FormGroup {
            title: i18nc("@title:group prefix radiobutton group", "Drag and Drop")

            QQC2.ButtonGroup { id: dndBehaviorGroup }

            Kirigami.FormEntry {
                title: i18nc("@title:group prefix radiobutton group", "When dragging files or folders:")
                subtitle: dndBehaviorAsk.Accessible.description
                contentItem: QQC2.RadioButton {
                    id: dndBehaviorAsk
                    text: i18nc("@option:radio When dragging...", "Always ask what to do")
                    Accessible.description: xi18nc("@info", "Hold <shortcut>Shift</shortcut> to move, <shortcut>Ctrl</shortcut> to copy, and <shortcut>Shift+Ctrl</shortcut> to create a symlink. Otherwise show a dialog.")
                    enabled: !kcm.globalsSettings.isImmutable("dndBehavior")
                    checked: kcm.globalsSettings.dndBehavior === WorkspaceOptionsGlobalsSettings.AlwaysAsk
                    onToggled: kcm.globalsSettings.dndBehavior = WorkspaceOptionsGlobalsSettings.AlwaysAsk
                    QQC2.ButtonGroup.group: dndBehaviorGroup
                }
            }

            Kirigami.FormEntry {
                subtitle: dndBehaviorMove.Accessible.description
                contentItem: QQC2.RadioButton {
                    id: dndBehaviorMove
                    text: i18nc("@option:radio When dragging…", "Move if on the same device")
                    Accessible.description: xi18nc("@info", "Hold <shortcut>Shift</shortcut> when dropping to show other options.")
                    enabled: !kcm.globalsSettings.isImmutable("dndBehavior")
                    checked: kcm.globalsSettings.dndBehavior === WorkspaceOptionsGlobalsSettings.MoveIfSameDevice
                    onToggled: kcm.globalsSettings.dndBehavior = WorkspaceOptionsGlobalsSettings.MoveIfSameDevice
                    QQC2.ButtonGroup.group: dndBehaviorGroup
                }
            }
        }



        Kirigami.FormGroup {
            title: i18nc("@title:group", "Touch")

            QQC2.ButtonGroup { id: tabletModeBehaviorGroup }

            Kirigami.FormEntry {
                title: i18nc("@title:group prefix radiobutton group", "Enable Touch Mode:")
                contentItem: QQC2.RadioButton {
                    id: touchEnabledRadio
                    text: KWindowSystem.isPlatformWayland ? i18nc("As in: 'Touch Mode is automatically enabled as needed'", "Automatically enable as needed") : i18nc("As in: 'Touch Mode is never enabled'", "Never enabled")
                    Accessible.description: touchModeAlwaysOffRadioButton.Accessible.description
                    checked: kcm.kwinSettings.tabletMode === "auto"
                    onToggled: {
                        if (checked) {
                            kcm.kwinSettings.tabletMode = "auto"
                        }
                    }
                    QQC2.ButtonGroup.group: tabletModeBehaviorGroup

                    KCM.SettingStateBinding {
                        configObject: kcm.kwinSettings
                        settingName: "tabletMode"
                    }
                }
                trailingItems: Kirigami.ContextualHelpButton {
                    Layout.alignment: Qt.AlignRight
                    visible: KWindowSystem.isPlatformWayland
                    toolTipText: i18nc("@info:whatsthis contextualhelpbutton tooltip", "Touch Mode will be automatically activated whenever the system detects a touchscreen but no mouse or touchpad. For example: when a transformable laptop's keyboard is flipped around or detached.")
                }
            }

            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    text: i18nc("As in: 'Touch Mode is always enabled'", "Always active")
                    Accessible.description: touchModeAlwaysOffRadioButton.Accessible.description
                    checked: kcm.kwinSettings.tabletMode === "on"
                    onToggled: {
                        if (checked) {
                            kcm.kwinSettings.tabletMode = "on"
                        }
                    }
                    QQC2.ButtonGroup.group: tabletModeBehaviorGroup

                    KCM.SettingStateBinding {
                        configObject: kcm.kwinSettings
                        settingName: "tabletMode"
                    }
                }
            }

            Kirigami.FormEntry {
                subtitle: touchModeAlwaysOffRadioButton.Accessible.description
                visible: KWindowSystem.isPlatformWayland
                contentItem: QQC2.RadioButton {
                    id: touchModeAlwaysOffRadioButton
                    text: i18nc("As in: 'Touch Mode is never enabled'", "Disabled")
                    Accessible.description: i18nc("@info:usagetip", "In Touch Mode, many elements of the user interface will become larger to more easily accommodate touch interaction.")
                    checked: kcm.kwinSettings.tabletMode === "off"
                    onToggled: {
                        if (checked) {
                            kcm.kwinSettings.tabletMode = "off"
                        }
                    }
                    QQC2.ButtonGroup.group: tabletModeBehaviorGroup

                    KCM.SettingStateBinding {
                        configObject: kcm.kwinSettings
                        settingName: "tabletMode"
                    }
                }
            }
        }
    }
}
