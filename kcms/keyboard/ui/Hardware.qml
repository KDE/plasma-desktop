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

Kirigami.FormLayout {

    enum NumLockState {
        NumLockOn,
        NumLockOff,
        NumLockUnchanged
    }

    KItemModels.KSortFilterProxyModel {
        id: keyboardsProxy
        sourceModel: kcm?.keyboards ?? undefined

        sortRoleName: "description"
        sortOrder: Qt.AscendingOrder
    }

    QQC2.ComboBox {
        id: keyboardModelComboBox
        Kirigami.FormData.label: i18nc("@title:group", "Keyboard model:")
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
            let index = indexOfValue(kcm.keyboardSettings.keyboardModel)
            currentIndex = index
        }

        KCM.SettingStateBinding {
            configObject: kcm.keyboardSettings
            settingName: "keyboardModel"
        }
    }

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
    }

    ColumnLayout {
        id: numlockLayout
        spacing: Kirigami.Units.smallSpacing
        Kirigami.FormData.label: i18nc("@title:group", "NumLock on Plasma Startup:")

        Repeater {
            model: [
                {
                    "text": i18nc("@option:radio", "Turn On"),
                    "key": Hardware.NumLockState.NumLockOn
                },
                {
                    "text": i18nc("@option:radio", "Turn Off"),
                    "key": Hardware.NumLockState.NumLockOff
                },
                {
                    "text": i18nc("@option:radio", "Leave unchanged"),
                    "key": Hardware.NumLockState.NumLockUnchanged
                },
            ]

            delegate: QQC2.RadioButton {
                text: modelData.text
                checked: kcm.miscSettings.numLock == modelData.key
                onToggled: kcm.miscSettings.numLock = modelData.key

                KCM.SettingStateBinding {
                    configObject: kcm.miscSettings
                    settingName: "numLock"
                }
            }

            onItemAdded: (index, item) => {
                if (index === 0) {
                    // Set buddy once, for an item in the first row.
                    // No, it doesn't work as a binding on children list.
                    numlockLayout.Kirigami.FormData.buddyFor = item;
                }
            }
        }
    }

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
    }

    ColumnLayout {
        id: keyLayout
        spacing: Kirigami.Units.smallSpacing
        Kirigami.FormData.label: i18nc("@title:group", "When key is held:")

        Repeater {
            model: [
                {
                    "text": i18nc("@option:radio", "Repeat the key"),
                    "key": __internal.keyboardRepeatRepeat,
                    "visible": true,
                },
                {
                    "text": i18nc("@option:radio", "Do nothing"),
                    "key": __internal.keyboardRepeatNothing,
                    "visible": true,
                },
                {
                    "text": i18nc("@option:radio", "Show accented and similar characters"),
                    "key": __internal.keyboardRepeatAccent,
                    "visible": kcm?.hasAccentSupport() ?? false,
                },
            ]

            delegate: QQC2.RadioButton {
                text: modelData.text
                visible: modelData.visible
                checked: kcm.miscSettings.keyboardRepeat === modelData.key
                onToggled: kcm.miscSettings.keyboardRepeat = modelData.key

                KCM.SettingStateBinding {
                    configObject: kcm.miscSettings
                    settingName: "keyboardRepeat"
                }
            }

            onItemAdded: (index, item) => {
                if (index === 0) {
                    // Set buddy once, for an item in the first row.
                    // No, it doesn't work as a binding on children list.
                    keyLayout.Kirigami.FormData.buddyFor = item;
                }
            }
        }
    }

    Kirigami.Separator {
        Kirigami.FormData.isSection: true
    }

    RowLayout {
        spacing: Kirigami.Units.smallSpacing
        visible: kcm.miscSettings.keyboardRepeat !== __internal.keyboardRepeatNothing
        Kirigami.FormData.label: i18nc("@label:slider", "Delay:")

        QQC2.Slider {
            Layout.preferredWidth: Kirigami.Units.gridUnit * 15

            from: 100
            to: 5000
            value: kcm.miscSettings.repeatDelay
            onMoved: kcm.miscSettings.repeatDelay = value

            KCM.SettingStateBinding {
                configObject: kcm.miscSettings
                settingName: "repeatDelay"
            }
        }

        QQC2.SpinBox {
            Layout.preferredWidth: Kirigami.Units.gridUnit * 6
            from: 0
            to: 5000
            stepSize: 50

            value: kcm.miscSettings.repeatDelay
            onValueModified: kcm.miscSettings.repeatDelay = value

            KCM.SettingStateBinding {
                configObject: kcm.miscSettings
                settingName: "repeatDelay"
            }
        }
    }

    RowLayout {
        spacing: Kirigami.Units.smallSpacing
        visible: kcm.miscSettings.keyboardRepeat !== __internal.keyboardRepeatNothing
        Kirigami.FormData.label: i18nc("@label:slider", "Rate:")

        QQC2.Slider {
            Layout.preferredWidth: Kirigami.Units.gridUnit * 15

            from: 20
            to: 20000
            value: kcm.miscSettings.repeatRate * 100
            onMoved: kcm.miscSettings.repeatRate = value / 100

            KCM.SettingStateBinding {
                configObject: kcm.miscSettings
                settingName: "repeatRate"
            }
        }

        QQC2.SpinBox {
            Layout.preferredWidth: Kirigami.Units.gridUnit * 6

            from: 20
            to: 20000
            stepSize: 500
            value: Math.round(kcm.miscSettings.repeatRate * 100)
            onValueModified: kcm.miscSettings.repeatRate = value / 100

            KCM.SettingStateBinding {
                configObject: kcm.miscSettings
                settingName: "repeatRate"
            }

            validator: DoubleValidator {
                bottom: Math.min(parent.from, parent.to)
                top:  Math.max(parent.from, parent.to)
            }

            textFromValue: function(value, locale) {
                return Number(value / 100).toLocaleString(locale, 'f', 2)
            }

            valueFromText: function(text, locale) {
                return Math.round(Number.fromLocaleString(locale, text) * 100)
            }
        }
    }

    QQC2.TextField {
        visible: kcm.miscSettings.keyboardRepeat !== __internal.keyboardRepeatNothing
        Layout.fillWidth: true
        Kirigami.FormData.label: i18nc("@label:textbox", "Test area:")
        placeholderText: i18nc("@info:placeholder", "Type here to test settings")
    }

    QtObject {
        id: __internal

        readonly property string keyboardRepeatRepeat: "repeat"
        readonly property string keyboardRepeatNothing: "nothing"
        readonly property string keyboardRepeatAccent: "accent"
    }
}
