/*
    SPDX-FileCopyrightText: 2025 Oliver Beard <olib141@outlook.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami
import org.kde.kquickcontrols as KQuickControls
import org.kde.kwindowsystem

Kirigami.FormLayout {
    id: pageLayout

    QQC2.RadioButton {
        id: zoomRadioButton
        QQC2.ButtonGroup.group: effectGroup
        Kirigami.FormData.label: i18nc("@label, followed by 'full screen' or 'magnify region'", "Zoom:")

        text: i18nc("@option:check, enable zoom effect, following 'Zoom:'", "Full screen")
        checked: kcm.zoomMagnifierSettings.zoom
        onToggled: { kcm.zoomMagnifierSettings.zoom = true; kcm.zoomMagnifierSettings.magnifier = false; }

        QQC2.ButtonGroup { id: effectGroup }

        KCM.SettingHighlighter { highlight: !kcm.zoomMagnifierSettings.zoom }
    }

    RowLayout {
        Layout.fillWidth: true

        spacing: 0

        Item {
            // Provide indentation for child FormLayout
            implicitWidth: Application.layoutDirection === Qt.RightToLeft ? magnifierRadioButton.contentItem.rightPadding : magnifierRadioButton.contentItem.leftPadding
        }

        Kirigami.FormLayout {
            id: zoomLayout

            twinFormLayouts: magnifierLayout
            wideMode: pageLayout.wideMode

            enabled: kcm.zoomMagnifierSettings.zoom

            QQC2.SpinBox {
                id: zoomPixelGridZoomSpinBox
                Kirigami.FormData.label: i18nc("@label:spinbox", "Show pixel grid at zoom level:")

                from: toInt(0)
                to: toInt(100)
                stepSize: toInt(1)

                validator: IntValidator {
                    bottom: Math.min(zoomPixelGridZoomSpinBox.from, zoomPixelGridZoomSpinBox.to)
                    top: Math.max(zoomPixelGridZoomSpinBox.from, zoomPixelGridZoomSpinBox.to)
                }

                textFromValue: (value, locale) => fromInt(value).toLocaleString(locale, 'f', 2)
                valueFromText: (text, locale) => Math.round(toInt(Number.fromLocaleString(locale, text)))

                value: toInt(kcm.zoomMagnifierSettings.zoomPixelGridZoom)
                onValueModified: kcm.zoomMagnifierSettings.zoomPixelGridZoom = fromInt(value)

                function toInt(value: double) : int {
                    return value * 100;
                }

                function fromInt(value: int) : double {
                    return value / 100;
                }

                KCM.SettingStateBinding {
                    configObject: kcm.zoomMagnifierSettings
                    settingName: "ZoomPixelGridZoom"
                }
            }

            QQC2.ComboBox {
                Kirigami.FormData.label: i18nc("@label:listbox", "Mouse pointer:")

                model: [i18nc("@item:inlistbox", "Scale"), i18nc("@item:inlistbox", "Keep"), i18nc("@item:inlistbox", "Hide")]
                currentIndex: kcm.zoomMagnifierSettings.zoomMousePointer
                onActivated: (index) => kcm.zoomMagnifierSettings.zoomMousePointer = index

                KCM.SettingStateBinding {
                    configObject: kcm.zoomMagnifierSettings
                    settingName: "ZoomMousePointer"
                }
            }

            QQC2.ComboBox {
                Kirigami.FormData.label: i18nc("@label:listbox", "Mouse tracking:")

                model: [i18nc("@item:inlistbox", "Proportional"), i18nc("@item:inlistbox", "Centered"), i18nc("@item:inlistbox", "Push"), i18nc("@item:inlistbox", "Disabled")]
                currentIndex: kcm.zoomMagnifierSettings.zoomMouseTracking
                onActivated: (index) => kcm.zoomMagnifierSettings.zoomMouseTracking = index

                KCM.SettingStateBinding {
                    configObject: kcm.zoomMagnifierSettings
                    settingName: "ZoomMouseTracking"
                }
            }

            QQC2.CheckBox {
                text: i18nc("@option:check", "Enable focus tracking")
                checked: kcm.zoomMagnifierSettings.zoomEnableFocusTracking
                onCheckedChanged: kcm.zoomMagnifierSettings.zoomEnableFocusTracking = checked
                visible: KWindowSystem.isPlatformX11

                KCM.SettingStateBinding {
                    configObject: kcm.zoomMagnifierSettings
                    settingName: "ZoomEnableFocusTracking"
                }
            }

            QQC2.CheckBox {
                text: i18nc("@option:check", "Enable text caret tracking")
                checked: kcm.zoomMagnifierSettings.zoomEnableTextCaretTracking
                onCheckedChanged: kcm.zoomMagnifierSettings.zoomEnableTextCaretTracking = checked

                KCM.SettingStateBinding {
                    configObject: kcm.zoomMagnifierSettings
                    settingName: "ZoomEnableTextCaretTracking"
                }
            }

            QQC2.Label {
                enabled: !kcm.isPlatformX11 && zoomPointerAxisGestureModifiersBox.keySequence != ""
                text: i18nc("@label Hint for scroll gestures", "Scroll while modifier keys are pressed to zoom")
                textFormat: Text.PlainText
                wrapMode: Text.Wrap
                font: Kirigami.Theme.smallFont
            }
        }
    }

    QQC2.RadioButton {
        id: magnifierRadioButton
        QQC2.ButtonGroup.group: effectGroup

        text: i18nc("@option:check, enable magnify effect, following 'Zoom:'", "Magnify region")
        checked: kcm.zoomMagnifierSettings.magnifier
        onToggled: { kcm.zoomMagnifierSettings.zoom = false; kcm.zoomMagnifierSettings.magnifier = true; }

        KCM.SettingHighlighter { highlight: !kcm.zoomMagnifierSettings.zoom }
    }

    RowLayout {
        Layout.fillWidth: true

        spacing: 0

        Item {
            // Provide indentation for child FormLayout
            implicitWidth: Application.layoutDirection === Qt.RightToLeft ? magnifierRadioButton.contentItem.rightPadding : magnifierRadioButton.contentItem.leftPadding
        }

        Kirigami.FormLayout {
            id: magnifierLayout

            twinFormLayouts: zoomLayout
            wideMode: pageLayout.wideMode

            enabled: kcm.zoomMagnifierSettings.magnifier

            QQC2.SpinBox {
                id: magnifierWidthSpinBox
                Kirigami.FormData.label: i18nc("@label:spinbox", "Width:")

                from: 100
                to: 2000
                stepSize: 10

                validator: IntValidator {
                    bottom: Math.min(magnifierWidthSpinBox.from, magnifierWidthSpinBox.to)
                    top: Math.max(magnifierWidthSpinBox.from, magnifierWidthSpinBox.to)
                }

                textFromValue: (value, locale) => { return i18ncp("short for pixel(s)", "%1 px", "%1 px", value); }
                valueFromText: (text, locale) => { return Number.fromLocaleString(locale, text.replace(i18nc("short for pixel(s)", "px"), "")); }

                value: kcm.zoomMagnifierSettings.magnifierWidth
                onValueModified: kcm.zoomMagnifierSettings.magnifierWidth = value

                KCM.SettingStateBinding {
                    configObject: kcm.zoomMagnifierSettings
                    settingName: "MagnifierWidth"
                }
            }

            QQC2.SpinBox {
                id: magnifierHeightSpinBox
                Kirigami.FormData.label: i18nc("@label:spinbox", "Height:")

                from: 100
                to: 2000
                stepSize: 10

                validator: IntValidator {
                    bottom: Math.min(magnifierHeightSpinBox.from, magnifierHeightSpinBox.to)
                    top: Math.max(magnifierHeightSpinBox.from, magnifierHeightSpinBox.to)
                }

                textFromValue: (value, locale) => { return i18ncp("short for pixel(s)", "%1 px", "%1 px", value); }
                valueFromText: (text, locale) => { return Number.fromLocaleString(locale, text.replace(i18nc("short for pixel(s)", "px"), "")); }

                value: kcm.zoomMagnifierSettings.magnifierHeight
                onValueModified: kcm.zoomMagnifierSettings.magnifierHeight = value

                KCM.SettingStateBinding {
                    configObject: kcm.zoomMagnifierSettings
                    settingName: "MagnifierHeight"
                }
            }
        }
    }

    QQC2.RadioButton {
        id: noneRadioButton
        QQC2.ButtonGroup.group: effectGroup

        text: i18nc("@option:check, disable zoom/magnify effect", "Disabled")
        checked: !(kcm.zoomMagnifierSettings.zoom || kcm.zoomMagnifierSettings.magnifier)
        onToggled: { kcm.zoomMagnifierSettings.zoom = false; kcm.zoomMagnifierSettings.magnifier = false; }

        KCM.SettingHighlighter { highlight: !kcm.zoomMagnifierSettings.zoom }
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    QQC2.SpinBox {
        id: sharedZoomFactorSpinBox
        Kirigami.FormData.label: i18nc("@label:spinbox", "Zoom factor:")

        from: toInt(1.05)
        to: toInt(4)
        stepSize: toInt(0.05)

        validator: IntValidator {
            bottom: Math.min(sharedZoomFactorSpinBox.from, sharedZoomFactorSpinBox.to)
            top: Math.max(sharedZoomFactorSpinBox.from, sharedZoomFactorSpinBox.to)
        }

        textFromValue: (value, locale) => fromInt(value).toLocaleString(locale, 'f', 2)
        valueFromText: (text, locale) => Math.round(toInt(Number.fromLocaleString(locale, text)))

        value: toInt(kcm.zoomMagnifierSettings.sharedZoomFactor)
        onValueModified: kcm.zoomMagnifierSettings.sharedZoomFactor = fromInt(value)

        function toInt(value: double) : int {
            return value * 20;
        }

        function fromInt(value: int) : double {
            return value / 20;
        }

        KCM.SettingStateBinding {
            configObject: kcm.zoomMagnifierSettings
            settingName: "SharedZoomFactor"
            extraEnabledConditions: kcm.zoomMagnifierSettings.zoom || kcm.zoomMagnifierSettings.magnifier
        }
    }

    RowLayout {
        Kirigami.FormData.label: i18nc("@label", "Scroll gesture modifier keys:")

        spacing: Kirigami.Units.smallSpacing

        Item {
            // For some reason, here setting enabled directly on
            // KeySequenceItem does not work, so we wrap it
            enabled: !kcm.isPlatformX11 && (kcm.zoomMagnifierSettings.zoom || kcm.zoomMagnifierSettings.magnifier)

            implicitWidth: zoomPointerAxisGestureModifiersBox.implicitWidth
            implicitHeight: zoomPointerAxisGestureModifiersBox.implicitHeight

            KQuickControls.KeySequenceItem {
                id: zoomPointerAxisGestureModifiersBox

                keySequence: kcm.zoomMagnifierSettings.zoomPointerAxisGestureModifiers
                onKeySequenceModified: kcm.zoomMagnifierSettings.zoomPointerAxisGestureModifiers = keySequence
                patterns: KQuickControls.ShortcutPattern.Modifier
                multiKeyShortcutsAllowed: false

                Connections {
                    target: kcm.zoomMagnifierSettings

                    // We have to do this because keySequence binding is broken
                    function onZoomPointerAxisGestureModifiersChanged() {
                        zoomPointerAxisGestureModifiersBox.keySequence = kcm.zoomMagnifierSettings.zoomPointerAxisGestureModifiers;
                    }
                }

                KCM.SettingStateBinding {
                    configObject: kcm.zoomMagnifierSettings
                    settingName: "ZoomPointerAxisGestureModifiers"
                }
            }
        }

        Kirigami.ContextualHelpButton {
            visible: kcm.isPlatformX11
            toolTipText: i18nc("@info:tooltip, indicates feature unavailable on X11", "Zoom scroll gestures are only available on Wayland.")
        }
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    QQC2.Button {
        text: i18nc("@action:button", "Configure Shortcuts…")
        icon.name: "preferences-desktop-keyboard-shortcut"

        enabled: (kcm.zoomMagnifierSettings.zoom || kcm.zoomMagnifierSettings.magnifier)

        onClicked: kcm.configureZoomMagnifyShortcuts()
    }

    QQC2.Label {
        enabled: (kcm.zoomMagnifierSettings.zoom || kcm.zoomMagnifierSettings.magnifier)
        text: i18nc("@label Hint for zoom/magnify effect usage", "Use shortcuts to control zoom")
        textFormat: Text.PlainText
        wrapMode: Text.Wrap
        font: Kirigami.Theme.smallFont
    }
}
