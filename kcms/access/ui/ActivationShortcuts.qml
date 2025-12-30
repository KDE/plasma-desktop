/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami
import org.kde.plasma.access.kcm

Kirigami.FormLayout {

    QQC2.CheckBox {
        id: activationShortcuts
        Kirigami.FormData.label: i18nc("@option:check", "Activation shortcuts:")
        text: i18nc("Enable activation shortcuts", "Enable")

        KCMUtils.SettingStateBinding {
            configObject: kcm.activationGesturesSettings
            settingName: "Gestures"
        }

        checked: kcm.activationGesturesSettings.gestures
        onToggled: kcm.activationGesturesSettings.gestures = checked
    }
    QQC2.Label {
        leftPadding: activationShortcuts.indicator.width
        text: i18nc("@label", "Press Shift 5 times to enable Sticky Keys")
        textFormat: Text.PlainText
        elide: Text.ElideRight
        font: Kirigami.Theme.smallFont
    }
    QQC2.Label {
        leftPadding: activationShortcuts.indicator.width
        text: i18nc("@label", "Hold Shift for 8 seconds to enable Slow Keys")
        textFormat: Text.PlainText
        elide: Text.ElideRight
        font: Kirigami.Theme.smallFont
    }

    RowLayout {
        spacing: Kirigami.Units.smallSpacing
        QQC2.CheckBox {
            text: i18nc("@option:check", "Disable sticky and slow keys after:")

            KCMUtils.SettingStateBinding {
                configObject: kcm.activationGesturesSettings
                settingName: "AccessXTimeout"
            }

            checked: kcm.activationGesturesSettings.accessXTimeout
            onToggled: kcm.activationGesturesSettings.accessXTimeout = checked

        }
        QQC2.SpinBox {
            id: spinbox

            KCMUtils.SettingStateBinding {
                configObject: kcm.activationGesturesSettings
                settingName: "AccessXTimeoutDelay"
                extraEnabledConditions: kcm.activationGesturesSettings.accessXTimeout
            }

            from: 1
            to: 30

            textFromValue: (value, locale) => {
                return i18ncp("@label:valuesuffix in spinbox %1 is timeout delay", "%1 min", "%1 min", value)
            }


            validator: IntValidatorWithSuffix {
                bottom: spinbox.from
                top: spinbox.to
            }
            valueFromText: (text, locale) => {
                return Number.fromLocaleString(locale, text.replace(i18ncp("short for minute(s)", "min", "min"), ""))
            }

            value: kcm.activationGesturesSettings.accessXTimeoutDelay
            onValueChanged: kcm.activationGesturesSettings.accessXTimeoutDelay = value
        }
    }

    QQC2.CheckBox {
        Kirigami.FormData.label: i18nc("@title:group for checkbox group", "When a shortcut is used:")
        text: i18nc("@option:check when shortcut used", "Display a confirmation dialog")

        KCMUtils.SettingStateBinding {
            configObject: kcm.activationGesturesSettings
            settingName: "GestureConfirmation"
        }

        checked: kcm.activationGesturesSettings.gestureConfirmation
        onToggled: kcm.activationGesturesSettings.gestureConfirmation = checked
    }
    QQC2.CheckBox {
        text: i18nc("@option:check when shortcut used", "Ring the system bell")

        KCMUtils.SettingStateBinding {
            configObject: kcm.activationGesturesSettings
            settingName: "AccessXBeep"
        }

        checked: kcm.activationGesturesSettings.accessXBeep
        onToggled: kcm.activationGesturesSettings.accessXBeep = checked
    }
    QQC2.CheckBox {
        text: i18nc("@option:check when shortcut used", "Show a notification")

        KCMUtils.SettingStateBinding {
            configObject: kcm.activationGesturesSettings
            settingName: "KeyboardNotifyAccess"
        }

        checked: kcm.activationGesturesSettings.keyboardNotifyAccess
        onToggled: kcm.activationGesturesSettings.keyboardNotifyAccess = checked
    }
    QQC2.Button {
        text: i18nc("@action:button opens Notifications kcm", "Configure Notifications…")
        icon.name: "preferences-desktop-notification"

        onClicked: kcm.configureKNotify()
    }
}
