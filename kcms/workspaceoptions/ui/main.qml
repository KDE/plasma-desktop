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

KCM.SimpleKCM {
    implicitWidth: Kirigami.Units.gridUnit * 40

    headerPaddingEnabled: false // Let the InlineMessage touch the edges
    header: Kirigami.InlineMessage {
        id: primarySelectionRebootMessage
        position: Kirigami.InlineMessage.Position.Header
        type: Kirigami.MessageType.Information
        text: i18n("The system must be restarted before changes to the middle-click paste setting can take effect.")
        visible: false
        showCloseButton: true
        actions: [
            Kirigami.Action {
                icon.name: "system-reboot"
                text: i18n("Restart")
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

    Kirigami.FormLayout {

        // Visual behavior settings
        RowLayout {
            Kirigami.FormData.label: i18nc("Part of the sentence 'Allow Plasma to show panel and widget tooltips'", "Allow Plasma to show:")
            Kirigami.FormData.buddyFor: tooltipDisablerCheckbox

            spacing: 0

            QQC2.CheckBox {
                id: tooltipDisablerCheckbox
                text: i18nc("Part of the sentence 'Allow Plasma to show panel and widget tooltips'", "Panel and widget tooltips")
                checked: kcm.plasmaSettings.delay > 0
                onCheckedChanged: kcm.plasmaSettings.delay = (checked ? 0.7 : -1)

                KCM.SettingStateBinding {
                    configObject: kcm.plasmaSettings
                    settingName: "delay"
                }
            }
            Kirigami.ContextualHelpButton {
                toolTipText: i18n("Allows all Plasma panel and desktop widgets to show descriptive tooltips when hovered with the pointer. This setting has no effect on the small tooltips displayed when hovering over individual buttons and other user interface elements.")
            }
        }

        RowLayout {
            spacing: 0

            QQC2.CheckBox {
                text: i18nc("Part of the sentence 'Allow Plasma to show OSD popups for status changes'", "OSD popups for status changes")
                checked: kcm.plasmaSettings.osdEnabled
                onCheckedChanged: kcm.plasmaSettings.osdEnabled = checked

                KCM.SettingStateBinding {
                    configObject: kcm.plasmaSettings
                    settingName: "osdEnabled"
                }
            }
            Kirigami.ContextualHelpButton {
                toolTipText: i18n("Allows all Plasma widgets to show on-screen display (OSD) popups for changes like volume and brightness level, or audio device switching.")
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        RowLayout {
            Kirigami.FormData.label: i18nc("@label", "Scrolling:")
            spacing: 0

            QQC2.CheckBox {
                text: i18nc("@option:check", "Prefer smooth scrolling")
                checked: kcm.globalsSettings.smoothScroll
                onToggled: kcm.globalsSettings.smoothScroll = checked

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "smoothScroll"
                }
            }
            Kirigami.ContextualHelpButton {
                toolTipText: i18nc("@info:tooltip", "This setting enables or disables animated transitions when scrolling with a mouse wheel or the keyboard. Some applications may not honor this setting because they either do not support smooth scrolling, or have their own setting for enabling and disabling it.")
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        // Click behavior settings

        QQC2.ButtonGroup { id: singleClickGroup }

        ColumnLayout {
            Kirigami.FormData.label: i18nc("part of a sentence: 'Clicking files or folders [opens them/selects them]'", "Clicking files or folders:")
            Kirigami.FormData.buddyFor: doubleClick

            spacing: 0

            QQC2.RadioButton {
                id: doubleClick
                text: i18nc("part of a sentence: 'Clicking files or folders selects them'", "Selects them")
                Accessible.description: doubleClickHelperText.text
                checked: !kcm.globalsSettings.singleClick
                onToggled: kcm.globalsSettings.singleClick = false
                QQC2.ButtonGroup.group: singleClickGroup

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "singleClick"
                    extraEnabledConditions: singleClick.enabled
                }
            }
            QQC2.Label {
                id: doubleClickHelperText
                Layout.fillWidth: true
                leftPadding: Application.layoutDirection === Qt.LeftToRight ?
                    doubleClick.indicator.width + doubleClick.spacing : doubleClickHelperText.padding
                rightPadding: Application.layoutDirection === Qt.RightToLeft ?
                    doubleClick.indicator.width + doubleClick.spacing : doubleClickHelperText.padding
                text: i18n("Open by double-clicking instead")
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0

            QQC2.RadioButton {
                id: singleClick
                text: i18nc("part of a sentence: 'Clicking files or folders opens them'", "Opens them")
                Accessible.description: singleClickHelperText.text
                checked: kcm.globalsSettings.singleClick
                onToggled: kcm.globalsSettings.singleClick = true
                QQC2.ButtonGroup.group: singleClickGroup

                KCM.SettingStateBinding {
                    configObject: kcm.globalsSettings
                    settingName: "singleClick"
                }
            }
            QQC2.Label {
                id: singleClickHelperText
                Layout.fillWidth: true
                leftPadding: Application.layoutDirection === Qt.LeftToRight ?
                    singleClick.indicator.width + singleClick.spacing : singleClickHelperText.padding
                rightPadding: Application.layoutDirection === Qt.RightToLeft ?
                    singleClick.indicator.width + singleClick.spacing : singleClickHelperText.padding
                text: i18n("Select by clicking on item's selection marker")
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
            }
        }

        Item {
            Kirigami.FormData.isSection: false
        }

        // scroll handle settings

        QQC2.ButtonGroup { id: scrollHandleBehaviorGroup }

        QQC2.RadioButton {
            Kirigami.FormData.label: i18n("Clicking in scrollbar track:")
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

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0

            QQC2.RadioButton {
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
            QQC2.Label {
                id: scrollbarLeftClickNavigatesByPageHelperText
                Layout.fillWidth: true
                leftPadding: Application.layoutDirection === Qt.LeftToRight ?
                    scrollbarLeftClickNavigatesByPage.indicator.width + scrollbarLeftClickNavigatesByPage.spacing : scrollbarLeftClickNavigatesByPageHelperText.padding
                rightPadding: Application.layoutDirection === Qt.RightToLeft ?
                    scrollbarLeftClickNavigatesByPage.indicator.width + scrollbarLeftClickNavigatesByPage.spacing : scrollbarLeftClickNavigatesByPageHelperText.padding
                text: i18n("Middle-click to scroll to clicked location")
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
            }
        }

        Item {
            Kirigami.FormData.isSection: true
            visible: primarySelectionRadio.visible
        }

        QQC2.CheckBox {
            id: primarySelectionRadio
            Kirigami.FormData.label: i18nc("@label for checkbox, part of a complete sentence: 'Middle-click pastes selected text'", "Middle-click:")
            visible: KWindowSystem.isPlatformWayland
            text: i18nc("@option:check part of a complete sentence: 'Middle click pastes selected text'", "Pastes selected text")
            checked: kcm.kwinSettings.primarySelection
            onToggled: kcm.kwinSettings.primarySelection = checked

            KCM.SettingStateBinding {
                configObject: kcm.kwinSettings
                settingName: "primarySelection"
            }
        }

        Item {
            Kirigami.FormData.isSection: true
        }

        QQC2.ButtonGroup { id: tabletModeBehaviorGroup }

        RowLayout {
            Kirigami.FormData.label: i18n("Touch Mode:")
            QQC2.RadioButton {
                text: KWindowSystem.isPlatformWayland ? i18nc("As in: 'Touch Mode is automatically enabled as needed'", "Automatically enable as needed") : i18nc("As in: 'Touch Mode is never enabled'", "Never enabled")
                Accessible.description: touchModeAlwaysOffRadioButtonHelperText.text
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
            Kirigami.ContextualHelpButton {
                visible: KWindowSystem.isPlatformWayland
                toolTipText: i18n("Touch Mode will be automatically activated whenever the system detects a touchscreen but no mouse or touchpad. For example: when a transformable laptop's keyboard is flipped around or detached.")
            }
        }

        QQC2.RadioButton {
            text: i18nc("As in: 'Touch Mode is always enabled'", "Always enabled")
            Accessible.description: touchModeAlwaysOffRadioButtonHelperText.text
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

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 0

            QQC2.RadioButton {
                id: touchModeAlwaysOffRadioButton
                visible: KWindowSystem.isPlatformWayland
                text: i18nc("As in: 'Touch Mode is never enabled'", "Never enabled")
                Accessible.description: touchModeAlwaysOffRadioButtonHelperText.text
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
            QQC2.Label {
                id: touchModeAlwaysOffRadioButtonHelperText
                Layout.fillWidth: true
                Layout.preferredWidth: Kirigami.Units.gridUnit * 20
                leftPadding: Application.layoutDirection === Qt.LeftToRight ?
                    touchModeAlwaysOffRadioButton.indicator.width + touchModeAlwaysOffRadioButton.spacing : touchModeAlwaysOffRadioButton.padding
                rightPadding: Application.layoutDirection === Qt.RightToLeft ?
                    touchModeAlwaysOffRadioButton.indicator.width + touchModeAlwaysOffRadioButton.spacing : touchModeAlwaysOffRadioButtonHelperText.padding
                text: i18n("In Touch Mode, many elements of the user interface will become larger to more easily accommodate touch interaction.")
                textFormat: Text.PlainText
                elide: Text.ElideRight
                font: Kirigami.Theme.smallFont
                wrapMode: Text.WordWrap
            }
        }

        // There is no label for what middle-clicking does when using the
        // "click to zoom the handle" behavior because Qt doesn't invert the
        // middle-click functionality when using this; see
        // https://bugreports.qt.io/browse/QTBUG-80728

        Item {
            Kirigami.FormData.isSection: true
        }

        RowLayout {
            Kirigami.FormData.label: i18n("Double-click interval:")
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
                toolTipText: i18n("Two clicks within this duration are considered a double-click. Some applications may not honor this setting.")
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
        Item {
            Kirigami.FormData.isSection: false
        }

        // Drag&Drop behavior settings

        QQC2.ButtonGroup { id: dndToMoveGroup }

        QQC2.RadioButton {
            id: dndToMoveDisabler
            Kirigami.FormData.label: i18n("Drag and drop behavior:")
            text: i18n("Always ask what to do")
            Accessible.description: dndToMoveDisablerHelperText.text
            enabled: !kcm.globalsSettings.isImmutable("dndToMove")
            checked: kcm.globalsSettings.dndToMove === 0
            onToggled: kcm.globalsSettings.dndToMove = 0
            QQC2.ButtonGroup.group: dndToMoveGroup
        }

        QQC2.Label {
            id: dndToMoveDisablerHelperText
            Layout.fillWidth: true
            leftPadding: Application.layoutDirection === Qt.LeftToRight ?
                dndToMoveDisabler.indicator.width + dndToMoveDisabler.spacing : dndToMoveDisablerHelperText.padding
            rightPadding: Application.layoutDirection === Qt.RightToLeft ?
                dndToMoveDisabler.indicator.width + dndToMoveDisabler.spacing : dndToMoveDisablerHelperText.padding
            text: i18n("Hold shift to move the file, Hold Ctrl to copy file, Hold Alt to create a symlink, otherwise show a dialog")
            textFormat: Text.PlainText
            elide: Text.ElideRight
            font: Kirigami.Theme.smallFont
        }

        QQC2.RadioButton {
            id: dndToMoveEnabler
            text: i18n("Move files if on the same device")
            Accessible.description: dndToMoveEnablerHelperText.text
            enabled: !kcm.globalsSettings.isImmutable("dndToMove")
            checked: kcm.globalsSettings.dndToMove === 1
            onToggled: kcm.globalsSettings.dndToMove = 1
            QQC2.ButtonGroup.group: dndToMoveGroup
        }

        QQC2.Label {
            id: dndToMoveEnablerHelperText
            Layout.fillWidth: true
            leftPadding: Application.layoutDirection === Qt.LeftToRight ?
                dndToMoveEnabler.indicator.width + dndToMoveEnabler.spacing : dndToMoveEnablerHelperText.padding
            rightPadding: Application.layoutDirection === Qt.RightToLeft ?
                dndToMoveEnabler.indicator.width + dndToMoveEnabler.spacing : dndToMoveEnablerHelperText.padding
            text: i18n("Hold Shift when dropping to show drop options")
            textFormat: Text.PlainText
            elide: Text.ElideRight
            font: Kirigami.Theme.smallFont
        }
    }
}
