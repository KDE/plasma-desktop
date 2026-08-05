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

Kirigami.Form {
    id: pageLayout

    component FullScreenZoomSettingsModel : ObjectModel {
        id: objectModel
        property bool visible: kcm.zoomMagnifierSettings.zoom
        Kirigami.FormEntry {
            visible: objectModel.visible
            title: i18nc("@label:spinbox", "Show pixel grid at zoom level:")
            contentItem: QQC2.SpinBox {
                id: zoomPixelGridZoomSpinBox

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
        }

        Kirigami.FormEntry {
            visible: objectModel.visible
            title: i18nc("@label:listbox", "Pointer tracking:")
            contentItem: QQC2.ComboBox {
                delegate: QQC2.ItemDelegate {
                    id: delegate

                    required property string title
                    required property string description

                    width: parent?.width ?? 0

                    text: delegate.title

                    contentItem: Kirigami.TitleSubtitle {
                        title: delegate.title
                        subtitle: delegate.description
                        font: delegate.font
                        selected: delegate.highlighted || delegate.down
                        wrapMode: Text.Wrap
                    }
                }
                model: [
                    {
                        title: i18nc("@item:inlistbox", "Proportional"),
                        description: i18nc("@item:inlistbox", "Zoom area moves in sync with pointer"),
                        settingIndex: 0
                    },
                    {
                        title: i18nc("@item:inlistbox", "Centered"),
                        description: i18nc("@item:inlistbox", "Pointer stays centered on-screen, except near screen edges"),
                        settingIndex: 1
                    },
                    {
                        title: i18nc("@item:inlistbox", "Centered (Strict)"),
                        description: i18nc("@item:inlistbox", "Pointer stays centered on-screen, even near screen edges"),
                        settingIndex: 4
                    },
                    {
                        title: i18nc("@item:inlistbox", "Push"),
                        description: i18nc("@item:inlistbox", "Pointer pushes zoom area at screen edges"),
                        settingIndex: 2
                    },
                    {
                        title: i18nc("@item:inlistbox", "Disabled"),
                        description: i18nc("@item:inlistbox", "Zoom area doesn't follow pointer"),
                        settingIndex: 3
                    }
                ]
                textRole: "title"
                currentIndex: model.findIndex(m => m.settingIndex === kcm.zoomMagnifierSettings.zoomMouseTracking)
                onActivated: index => kcm.zoomMagnifierSettings.zoomMouseTracking = model[index].settingIndex

                Layout.preferredWidth: Kirigami.Units.gridUnit * 15

                KCM.SettingStateBinding {
                    configObject: kcm.zoomMagnifierSettings
                    settingName: "ZoomMouseTracking"
                }
            }
        }

        Kirigami.FormEntry {
            visible: objectModel.visible
            contentItem: QQC2.CheckBox {
                text: i18nc("@option:check", "Sharpen screen content while zoomed in")
                checked: kcm.zoomMagnifierSettings.zoomUsePatternUpscaler
                onCheckedChanged: kcm.zoomMagnifierSettings.zoomUsePatternUpscaler = checked

                KCM.SettingStateBinding {
                    configObject: kcm.zoomMagnifierSettings
                    settingName: "ZoomUsePatternUpscaler"
                }
            }
        }

        Kirigami.FormEntry {
            visible: objectModel.visible && KWindowSystem.isPlatformX11
            contentItem: QQC2.CheckBox {
                text: i18nc("@option:check", "Enable focus tracking")
                checked: kcm.zoomMagnifierSettings.zoomEnableFocusTracking
                onCheckedChanged: kcm.zoomMagnifierSettings.zoomEnableFocusTracking = checked

                KCM.SettingStateBinding {
                    configObject: kcm.zoomMagnifierSettings
                    settingName: "ZoomEnableFocusTracking"
                }
            }
        }

        Kirigami.FormEntry {
            visible: objectModel.visible
            contentItem: QQC2.CheckBox {
                text: i18nc("@option:check", "Enable text caret tracking")
                checked: kcm.zoomMagnifierSettings.zoomEnableTextCaretTracking
                onCheckedChanged: kcm.zoomMagnifierSettings.zoomEnableTextCaretTracking = checked

                KCM.SettingStateBinding {
                    configObject: kcm.zoomMagnifierSettings
                    settingName: "ZoomEnableTextCaretTracking"
                }
            }
        }
    }

    component MagnifierZoomSettingsModel : ObjectModel {
        id: objectModel
        property bool visible: kcm.zoomMagnifierSettings.magnifier
        Kirigami.FormEntry {
            visible: objectModel.visible
            title: i18nc("@label:spinbox", "Width:")
            contentItem: QQC2.SpinBox {
                id: magnifierWidthSpinBox

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
        }

        Kirigami.FormEntry {
            visible: objectModel.visible
            title: i18nc("@label:spinbox", "Height:")
            contentItem: QQC2.SpinBox {
                id: magnifierHeightSpinBox

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

    Kirigami.FormGroup {
        Kirigami.FormEntry {
            title: i18nc("@label, followed by 'full screen' or 'magnify region'", "Zoom:")
            contentItem: QQC2.RadioButton {
                id: zoomRadioButton
                QQC2.ButtonGroup.group: effectGroup

                text: i18nc("@option:check, enable zoom effect, following 'Zoom:'", "Full screen")
                checked: kcm.zoomMagnifierSettings.zoom
                onToggled: { kcm.zoomMagnifierSettings.zoom = true; kcm.zoomMagnifierSettings.magnifier = false; }

                QQC2.ButtonGroup { id: effectGroup }

                KCM.SettingHighlighter { highlight: !kcm.zoomMagnifierSettings.zoom }
            }
        }

        Kirigami.FormEntry {
            contentItem: QQC2.RadioButton {
                id: magnifierRadioButton
                QQC2.ButtonGroup.group: effectGroup

                text: i18nc("@option:check, enable magnify effect, following 'Zoom:'", "Magnify region")
                checked: kcm.zoomMagnifierSettings.magnifier
                onToggled: { kcm.zoomMagnifierSettings.zoom = false; kcm.zoomMagnifierSettings.magnifier = true; }

                KCM.SettingHighlighter { highlight: !kcm.zoomMagnifierSettings.zoom }
            }
        }

        Kirigami.FormEntry {
            contentItem: QQC2.RadioButton {
                id: noneRadioButton
                QQC2.ButtonGroup.group: effectGroup

                text: i18nc("@option:check, disable zoom/magnify effect", "Disabled")
                checked: !(kcm.zoomMagnifierSettings.zoom || kcm.zoomMagnifierSettings.magnifier)
                onToggled: { kcm.zoomMagnifierSettings.zoom = false; kcm.zoomMagnifierSettings.magnifier = false; }

                KCM.SettingHighlighter { highlight: !kcm.zoomMagnifierSettings.zoom }
            }
        }

        Kirigami.FormSeparator { visible: kcm.zoomMagnifierSettings.zoom || kcm.zoomMagnifierSettings.magnifier}

        Repeater {
            model: FullScreenZoomSettingsModel {}
        }

        Repeater {
            model: MagnifierZoomSettingsModel {}
        }

        Kirigami.FormSeparator {}

        Kirigami.FormEntry {
            title: i18nc("@label:spinbox", "Zoom factor:")
            contentItem: QQC2.SpinBox {
                id: sharedZoomFactorSpinBox

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
        }
    }

    Kirigami.FormGroup {
        Kirigami.FormEntry {
            title: i18nc("@label", "Scroll gesture modifier keys:")
            subtitle: i18nc("@label Hint for scroll gestures", "Scroll while modifier keys are pressed to zoom")
            trailingItems: [
                Kirigami.ContextualHelpButton {
                    visible: kcm.isPlatformX11
                    toolTipText: i18nc("@info:tooltip, indicates feature unavailable on X11", "Zoom scroll gestures are only available on Wayland.")
                }
            ]
            contentItem: KQuickControls.KeySequenceItem {
                id: zoomPointerAxisGestureModifiersBox
                enabled: !kcm.isPlatformX11 && (kcm.zoomMagnifierSettings.zoom || kcm.zoomMagnifierSettings.magnifier)
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
        Kirigami.FormSeparator {}
        Kirigami.FormAction {
            enabled: action.enabled
            subtitle: i18nc("@label Hint for zoom/magnify effect usage", "Use shortcuts to control zoom")
            action: QQC2.Action {
                enabled: (kcm.zoomMagnifierSettings.zoom || kcm.zoomMagnifierSettings.magnifier)
                text: i18nc("@action:button", "Configure Shortcuts…")
                icon.name: "preferences-desktop-keyboard-shortcut"
                onTriggered: kcm.configureZoomMagnifyShortcuts()
            }
        }
    }
}
