/*
    SPDX-FileCopyrightText: 2025 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kquickcontrols as KQuickControls
import org.kde.kitemmodels as KItemModels
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kcmutils as KCM

import org.kde.plasma.private.kcm_keyboard as KCMKeyboard

KCM.SimpleKCM {
    id: root

    KItemModels.KSortFilterProxyModel {
        id: keyboardsProxy
        sourceModel: KCMKeyboard.KeyboardModel {}
        sortRoleName: "description"
        sortOrder: Qt.AscendingOrder
    }

    leftPadding: 0
    rightPadding: 0

    ColumnLayout {
        spacing: 0

        FormCard.FormHeader {
            title: i18n("Model & Layout")
        }

        FormCard.FormCard {
            FormCard.FormComboBoxDelegate {
                id: modelCombo

                text: i18nc("@label:listbox", "Keyboard Model")
                textRole: "description"
                valueRole: "name"
                model: keyboardsProxy

                Component.onCompleted: {
                    selectCurrent()
                }

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

            FormCard.FormSwitchDelegate {
                id: enableLayouts
                text: i18nc("@option:check", "Custom Layout")
                checked: kcm.keyboardSettings.configureLayouts
                onCheckedChanged: {
                    kcm.keyboardSettings.configureLayouts = checked
                }
                KCM.SettingStateBinding {
                    configObject: kcm.keyboardSettings
                    settingName: "configureLayouts"
                }
            }

            Repeater {
                id: list
                model: kcm.userLayoutModel

                delegate: FormCard.FormTextDelegate {
                    text: layoutName
                    description: variantName
                }
            }

            FormCard.FormButtonDelegate {
                id: pickLayoutButton
                icon.name: "preferences-desktop-keyboard"
                text: i18nc("@action:button", "Change Layout...")
                onClicked: _cLayoutDialog.incubateObject(root, {}, Qt.Asynchronous)
                enabled: kcm.keyboardSettings.configureLayouts
            }
        }

        FormCard.FormHeader {
            title: i18nc("@title:group", "Key Repeat")
        }
        FormCard.FormCard {

            FormCard.FormSwitchDelegate {
                id: keyRepeatSwitch
                text: i18nc("@option:check", "Customize Repeat")
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

            FormCard.FormSpinBoxDelegate {
                id: repeatDelaySpinbox
                Layout.preferredWidth: Kirigami.Units.gridUnit * 8
                from: 100
                to: 5000
                stepSize: 50

                label: i18nc("@label:textbox Key repeat delay", "Delay:")
                value: kcm.miscSettings.repeatDelay
                onValueChanged: kcm.miscSettings.repeatDelay = value

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

            FormCard.FormSpinBoxDelegate {
                id: repeatRateSpinbox
                from: 20
                to: 20000
                stepSize: 500
                label: i18nc("@label:textbox Key repeat rate", "Rate:")



                value: Math.round(kcm.miscSettings.repeatRate * 100)
                onValueChanged: kcm.miscSettings.repeatRate = value / 100

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
    }

    Component {
        id: _cLayoutDialog

        LayoutDialog {
            visible: false
            isMobile: true

            Component.onCompleted: open()
            onAccepted: destroy()
            onRejected: destroy()
        }
    }

    KeyboardInternal {
        id: __internal
    }
}
