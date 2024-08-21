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

import org.kde.plasma.private.kcm_keyboard as KCMKeyboard

KCM.ScrollViewKCM {
    id: root

    implicitWidth: Kirigami.Units.gridUnit * 44
    implicitHeight: Kirigami.Units.gridUnit * 25

    actions: [keyBindingsPageAction]

    extraFooterTopPadding: true // re-add separator line

    KItemModels.KSortFilterProxyModel {
        id: keyboardsProxy
        sourceModel: kcm?.keyboards ?? undefined
        sortRoleName: "description"
        sortOrder: Qt.AscendingOrder
    }

    readonly property Kirigami.Action switchingPageAction: Kirigami.Action {
        icon.name: "input-keyboard-symbolic"
        text: i18nc("@action:button", "Configure Switching…")
        Accessible.name: i18nc("@action:button accessible", "Configure Switching")
        onTriggered: kcm.push("Switching.qml")
    }

    readonly property Kirigami.Action keyBindingsPageAction: Kirigami.Action {
        icon.name: "settings-configure-symbolic"
        text: i18nc("@action:button", "Key Bindings")
        Accessible.name: i18nc("@action:button accessible", "Key Bindings")
        onTriggered: kcm.push("KeyBindings.qml")
    }

    readonly property Kirigami.Action addLayoutAction: Kirigami.Action {
        icon.name: "list-add-symbolic"
        text: i18nc("@action:button Layout", "Add…")
        Accessible.name: i18nc("@action:button accessible", "Add layout")
        onTriggered: _cLayoutDialog.incubateObject(root, {}, Qt.Asynchronous)
        enabled: kcm.keyboardSettings.configureLayouts
    }

    readonly property Kirigami.Action configureLayoutsAction: Kirigami.Action {
        displayComponent: QQC2.Switch {
            text: i18nc("@label:checkbox configure layouts option", "Enable")
            Accessible.name: i18nc("@label:checkbox accessible", "Configure Layouts")
            checked: kcm.keyboardSettings.configureLayouts
            onToggled: kcm.keyboardSettings.configureLayouts = checked

            KCM.SettingStateBinding {
                configObject: kcm.keyboardSettings
                settingName: "configureLayouts"
            }
        }
    }

    header: Kirigami.FormLayout {
        id: headerFormLayout

        QQC2.ComboBox {
            id: keyboardModelComboBox
            Kirigami.FormData.label: i18nc("@label:listbox Keyboard model", "Model:")
            model: keyboardsProxy
            textRole: "description"
            valueRole: "name"
            onActivated: kcm.keyboardSettings.keyboardModel = currentValue

            Component.onCompleted: selectCurrent()

            Connections {
                target: kcm?.keyboardSettings ?? undefined

                function onKeyboardModelChanged(): void {
                    keyboardModelComboBox.selectCurrent()
                }
            }

            function selectCurrent(): void {
                currentIndex = indexOfValue(kcm.keyboardSettings.keyboardModel);
            }

            KCM.SettingStateBinding {
                configObject: kcm.keyboardSettings
                settingName: "keyboardModel"
            }
        }

        QQC2.ComboBox {
            id: numlockCombobox
            Kirigami.FormData.label: i18nc("@label:listbox", "NumLock on startup:")
            Layout.preferredWidth: keyboardModelComboBox.width
            textRole: "text"
            valueRole: "value"
            onActivated: kcm.miscSettings.numLock = currentValue

            model: [
                {
                    "text": i18nc("@item:inlistbox Turn on \"NumLock\" on startup", "Turn on"),
                    "value": KCMKeyboard.NumLockState.NumLockOn
                },
                {
                    "text": i18nc("@item:inlistbox Turn off \"NumLock\" on startup", "Turn off"),
                    "value": KCMKeyboard.NumLockState.NumLockOff
                },
                {
                    "text": i18nc("@item:inlistbox Leave \"NumLock\" at whatever it was set to before Plasma started up", "Leave unchanged"),
                    "value": KCMKeyboard.NumLockState.NumLockUnchanged
                },
            ]

            Component.onCompleted: selectCurrent()

            Connections {
                target: kcm?.miscSettings ?? undefined

                function onNumLockChanged(): void {
                    numlockCombobox.selectCurrent()
                }
            }

            function selectCurrent(): void {
                currentIndex = indexOfValue(kcm.miscSettings.numLock);
            }

            KCM.SettingStateBinding {
                configObject: kcm.miscSettings
                settingName: "numLock"
            }
        }

        RowLayout {
            Kirigami.FormData.label: i18nc("@label:checkbox", "Key repeat:")
            spacing: Kirigami.Units.smallSpacing

            QQC2.CheckBox {
                checked: kcm.miscSettings.keyboardRepeat === __internal.keyboardRepeatRepeat
                onToggled: kcm.miscSettings.keyboardRepeat = checked
                           ? __internal.keyboardRepeatRepeat
                           : __internal.keyboardRepeatNothing

                Accessible.name: i18nc("@label:checkbox accessible", "Key repeat")

                KCM.SettingStateBinding {
                    configObject: kcm.miscSettings
                    settingName: "keyboardRepeat"
                }
            }

            QQC2.Label {
                // Force the label to use its own Kirigami.Theme object
                Kirigami.Theme.inherit: false
                text: i18nc("@label:textbox Key repeat delay", "Delay:")
                enabled: kcm.miscSettings.keyboardRepeat !== __internal.keyboardRepeatNothing
            }

            QQC2.SpinBox {
                id: repeatDelaySpinbox
                Layout.preferredWidth: Kirigami.Units.gridUnit * 8
                from: 100
                to: 5000
                stepSize: 50

                value: kcm.miscSettings.repeatDelay
                onValueModified: kcm.miscSettings.repeatDelay = value

                Accessible.name: i18nc("@label:spinbox accessible", "Key repeat delay %1", textFromValue(value))

                validator: IntValidator {
                    bottom: Math.min(repeatDelaySpinbox.from, repeatDelaySpinbox.to)
                    top: Math.max(repeatDelaySpinbox.from, repeatDelaySpinbox.to)
                }

                textFromValue: (value, locale) => {
                    return i18nc("@spinbox:value Repeat delay interval", "%1 ms", value)
                }

                valueFromText: (text, locale) => {
                    return Number.fromLocaleString(locale, text.replace(i18nc("@spinbox:value Repeat delay interval", "ms"), ""))
                }

                KCM.SettingStateBinding {
                    configObject: kcm.miscSettings
                    settingName: "repeatDelay"
                    extraEnabledConditions: kcm.miscSettings.keyboardRepeat !== __internal.keyboardRepeatNothing
                }
            }

            QQC2.Label {
                // Force the label to use its own Kirigami.Theme object
                Kirigami.Theme.inherit: false
                text: i18nc("@label:textbox Key repeat rate", "Rate:")
                enabled: kcm.miscSettings.keyboardRepeat !== __internal.keyboardRepeatNothing
            }

            QQC2.SpinBox {
                id: repeatRateSpinbox
                Layout.preferredWidth: Kirigami.Units.gridUnit * 8
                from: 20
                to: 20000
                stepSize: 500

                value: Math.round(kcm.miscSettings.repeatRate * 100)
                onValueModified: kcm.miscSettings.repeatRate = value / 100

                Accessible.name: i18nc("@label:spinbox accessible", "Key repeat rate %1", textFromValue(value))

                validator: DoubleValidator {
                    bottom: Math.min(repeatRateSpinbox.from, repeatRateSpinbox.to)
                    top:  Math.max(repeatRateSpinbox.from, repeatRateSpinbox.to)
                }

                textFromValue: (value, locale) => {
                    return i18nc("@spinbox:value  Key repeat rate", "%1 repeats/s", value / 100)
                }

                valueFromText: (text, locale) => {
                    return Number.fromLocaleString(locale, text.replace(i18nc("@spinbox:value  Key repeat rate", "repeats/s"), "")) * 100
                }

                KCM.SettingStateBinding {
                    configObject: kcm.miscSettings
                    settingName: "repeatRate"
                    extraEnabledConditions: kcm.miscSettings.keyboardRepeat !== __internal.keyboardRepeatNothing
                }
            }
        }

        QQC2.TextField {
            Kirigami.FormData.label: i18nc("@label:textbox", "Test area:")
            Layout.preferredWidth: keyboardModelComboBox.width
            placeholderText: i18nc("@info:placeholder", "Type here to test settings")
        }
    }

    view: ListView {
        id: list
        model: kcm.userLayoutModel

        headerPositioning: ListView.OverlayHeader
        header: Kirigami.InlineViewHeader {
            width: list.width
            text: i18nc("@title:view", "Layouts")
            actions: [ configureLayoutsAction, addLayoutAction ]
        }

        delegate: LayoutDelegate {
            id: delegate
            onMove: (oldIndex, newIndex) => kcm.userLayoutModel.move(oldIndex, newIndex)

            isLoopingLayout: layoutLoopingCheckBox.checked && (index + 1) > layoutLoopCountSpinBox.value
            actions: [
                Kirigami.Action {
                    icon.name: "arrow-up-symbolic"
                    text: i18nc("@action:button", "Move Up")
                    Accessible.name: i18nc("@action:button accessible", "Move %1 layout up", delegate.layoutName)
                    onTriggered: kcm.userLayoutModel.move(index, index - 1)
                    enabled: index > 0
                },
                Kirigami.Action {
                    icon.name: "arrow-down-symbolic"
                    text: i18nc("@action:button", "Move Down")
                    Accessible.name: i18nc("@action:button accessible", "Move %1 layout Down", delegate.layoutName)
                    onTriggered: kcm.userLayoutModel.move(index, index + 1)
                    enabled: index < list.count - 1
                },
                Kirigami.Action {
                    separator: true
                },
                Kirigami.Action {
                    icon.name: "input-keyboard-virtual-symbolic"
                    text: i18nc("@action:button", "Preview")
                    Accessible.name: i18nc("@action:button accessible", "Preview")
                    onTriggered: kcm.requestPreview(kcm.keyboardSettings.keyboardModel, layout, variant, variantName)
                },
                Kirigami.Action {
                    separator: true
                },
                Kirigami.Action {
                    icon.name: "edit-delete-remove-symbolic"
                    text: i18nc("@action:button", "Remove")
                    Accessible.name: i18nc("@action:button accessible", "Remove layout %1", delegate.layoutName)
                    onTriggered: kcm.userLayoutModel.remove(index)
                }
            ]
        }
    }

    footer: Kirigami.FormLayout {
        twinFormLayouts: headerFormLayout

        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            Kirigami.FormData.label: i18nc("@label:checkbox", "Spare layouts:")

            QQC2.CheckBox {
                id: layoutLoopingCheckBox
                onToggled: {
                    const count = checked ? __internal.minimumAvailableLoops : __internal.noLooping;
                    kcm.keyboardSettings.layoutLoopCount = count;
                }

                checked: {
                    // If the configureLayouts is turned off or the available value is less than the minimum, then the option is disabled
                    if (!kcm.keyboardSettings.configureLayouts || __internal.minimumAvailableLoops < __internal.minimumLoopingCount) {
                        return false;
                    }

                    // If the number of layouts is not the default value
                    // or the available number of layouts is greater than the limit from X.org, then enable the option
                    return kcm.keyboardSettings.layoutLoopCount !== __internal.noLooping
                        || __internal.minimumAvailableLoops >= kcm.maxGroupCount
                }

                enabled: {
                    // If the configureLayouts is turned off, then the element is disabled
                    if (!kcm.keyboardSettings.configureLayouts) {
                        return false;
                    }

                    // If the number of layouts is within the range of acceptable values, then enable the element
                    return __internal.minimumAvailableLoops >= __internal.minimumLoopingCount
                        && __internal.minimumAvailableLoops < kcm.maxGroupCount
                }
            }

            QQC2.SpinBox {
                id: layoutLoopCountSpinBox
                Layout.minimumWidth: Kirigami.Units.gridUnit * 4

                Accessible.name: i18nc("@label:spinbox", "Number of spared main layouts")

                enabled: layoutLoopingCheckBox.checked && kcm.keyboardSettings.configureLayouts
                to: Math.max(from, __internal.minimumAvailableLoops)
                from: {
                    if (layoutLoopingCheckBox.checked) {
                        return __internal.minimumLoopingCount;
                    }

                    return __internal.noLooping;
                }

                value: {
                    if (!kcm.keyboardSettings.configureLayouts) {
                        return __internal.noLooping;
                    }

                    if (kcm.keyboardSettings.layoutLoopCount > to){
                        return to
                    }

                    return kcm.keyboardSettings.layoutLoopCount;
                }

                textFromValue: (value, locale) => value < 0 ? "" : Number(value).toLocaleString(locale, "f", 0)
                onValueModified: kcm.keyboardSettings.layoutLoopCount = value;
            }

            QQC2.Label {
                // Force the label to use its own Kirigami.Theme object
                Kirigami.Theme.inherit: false
                enabled: layoutLoopCountSpinBox.enabled
                text: i18nc("@label:textbox completes spinbox, x main layouts", "main layouts")
            }

            Kirigami.ContextualHelpButton {
                toolTipText: i18nc("@info", "You can have up to 4 main layouts, all other layouts are spare. Spare layouts are not included when cycling through the layouts, but can be accessed through the keyboard indicator applet or by setting a custom shortcut.")
            }
        }

        RowLayout {
            Kirigami.FormData.label: i18nc("@label:group Shortcut to change the keyboard layout. Keep it short", "Change layout shortcut:")
            spacing: Kirigami.Units.smallSpacing

            KQuickControls.KeySequenceItem {
                id: alternativeShortcut
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

            QQC2.Label {
                text: i18nc("@label Change layout shortcut or \"Configure Switching\"", "or")
            }

            QQC2.Button {
                action: switchingPageAction
            }
        }
    }

    Component {
        id: _cLayoutDialog

        LayoutDialog {
            visible: false

            Component.onCompleted: open()
            onAccepted: destroy()
            onRejected: destroy()
        }
    }

    QtObject {
        id: __internal

        readonly property string keyboardRepeatRepeat: "repeat"
        readonly property string keyboardRepeatNothing: "nothing"
        readonly property string keyboardRepeatAccent: "accent"

        // Default value for layoutLoopCount (e.g. we cannot use layout looping feature)
        readonly property int noLooping: -1

        // Minimum number of layouts that can be used with layout looping feature
        readonly property int minimumLoopingCount: 2

        // Minimum number of layouts currently available (X.org only allows to have 4 layouts)
        readonly property int minimumAvailableLoops: Math.min(kcm.maxGroupCount, list.count - 1)
    }
}
