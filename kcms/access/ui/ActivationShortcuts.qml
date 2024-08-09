/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kcmutils as KCMUtils
import org.kde.kirigami as Kirigami

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

        Accessible.role: Accessible.CheckBox
        Accessible.name: i18n("Enable activation gestures.")
        Accessible.description: i18n("Press Shift + Num Lock to enable Mouse Navigation. Press Shift 5 times to enable Sticky Keys. Hold Shift for 8 seconds to enable Slow Keys")
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

            Accessible.role: Accessible.CheckBox
            Accessible.name: i18n("Disable the accessibility features enabled by activation shortcuts after a specified time")
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

            validator: IntValidator {
                bottom: Math.min(spinbox.from, spinbox.to)
                top: Math.max(spinbox.from, spinbox.to)
            }

            textFromValue: (value, locale) => {
                return i18np("%1 min", "%1 min", value)
            }

            valueFromText: (text, locale) => {
                return Number.fromLocaleString(locale, text.replace(i18ncp("short for minute(s)", "min", "min"), ""))
            }

            value: kcm.activationGesturesSettings.accessXTimeoutDelay
            onValueChanged: kcm.activationGesturesSettings.accessXTimeoutDelay = value

            Accessible.role: Accessible.SpinBox
            Accessible.name: i18n("The time until the enabled accessibility settings are automatically disabled.")
        }
    }

    QQC2.CheckBox {
        Kirigami.FormData.label: i18n("When a shortcut is used:")
        text: i18n("Display a confirmation dialog")

        KCMUtils.SettingStateBinding {
            configObject: kcm.activationGesturesSettings
            settingName: "GestureConfirmation"
        }

        checked: kcm.activationGesturesSettings.gestureConfirmation
        onToggled: kcm.activationGesturesSettings.gestureConfirmation = checked

        Accessible.role: Accessible.CheckBox
        Accessible.name: i18n("Display a confirmation dialog whenever a gesture is activated")
    }
    QQC2.CheckBox {
        text: i18n("Ring the system bell")

        KCMUtils.SettingStateBinding {
            configObject: kcm.activationGesturesSettings
            settingName: "AccessXBeep"
        }

        checked: kcm.activationGesturesSettings.accessXBeep
        onToggled: kcm.activationGesturesSettings.accessXBeep = checked

        Accessible.role: Accessible.CheckBox
        Accessible.name: i18n("Ring the system bell whenever a gesture is activated")
    }
    QQC2.CheckBox {
        text: i18n("Show a notification")

        KCMUtils.SettingStateBinding {
            configObject: kcm.activationGesturesSettings
            settingName: "KeyboardNotifyAccess"
        }

        checked: kcm.activationGesturesSettings.keyboardNotifyAccess
        onToggled: kcm.activationGesturesSettings.keyboardNotifyAccess = checked

        Accessible.role: Accessible.CheckBox
        Accessible.name: i18n("Show a notification whenever a gesture is activated")
    }
    QQC2.Button {
        text: i18n("Configure Notifications…")
        icon.name: "preferences-desktop-notification"

        onClicked: kcm.configureKNotify()

        Accessible.role: Accessible.Button
        Accessible.name: text
        Accessible.description: i18n("Opens a new dialog to configure notifications for most accessibility features.")
    }
}
