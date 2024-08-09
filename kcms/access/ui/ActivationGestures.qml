/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.6
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as QQC2
import org.kde.kcmutils as KCM
import org.kde.kirigami 2.3 as Kirigami

Kirigami.FormLayout {

    QQC2.ButtonGroup { id: gesturesGroup }
    QQC2.CheckBox {
        id: activationGestures
        Kirigami.FormData.label: i18n("Activation gestures:")
        text: i18nc("Enable activation gestures", "Enable")

        KCM.SettingStateBinding {
            configObject: kcm.activationGesturesSettings
            settingName: "Gestures"
        }

        checked: kcm.activationGesturesSettings.gestures
        onToggled: kcm.activationGesturesSettings.gestures = checked
    }
    QQC2.Label {
        QQC2.ButtonGroup.group: gesturesGroup
        leftPadding: activationGestures.indicator.width
        text: i18n("Press Shift + Num Lock to enable Mouse Navigation")
        textFormat: Text.PlainText
        elide: Text.ElideRight
        font: Kirigami.Theme.smallFont
    }
    QQC2.Label {
        QQC2.ButtonGroup.group: gesturesGroup
        leftPadding: activationGestures.indicator.width
        text: i18n("Press Shift 5 times to enable Sticky Keys")
        textFormat: Text.PlainText
        elide: Text.ElideRight
        font: Kirigami.Theme.smallFont
    }
    QQC2.Label {
        QQC2.ButtonGroup.group: gesturesGroup
        leftPadding: activationGestures.indicator.width
        text: i18n("Hold Shift for 8 seconds to enable Slow Keys")
        textFormat: Text.PlainText
        elide: Text.ElideRight
        font: Kirigami.Theme.smallFont
    }

    QQC2.CheckBox {
        text: i18n("Disable after inactivity:")

        KCM.SettingStateBinding {
            configObject: kcm.activationGesturesSettings
            settingName: "AccessXTimeout"
        }

        checked: kcm.activationGesturesSettings.accessXTimeout
        onToggled: kcm.activationGesturesSettings.accessXTimeout = checked

    }
    RowLayout {
        QQC2.SpinBox {
            id: spinbox

            Layout.leftMargin: Kirigami.Units.smallSpacing * 5
            KCM.SettingStateBinding {
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
                return Number.fromLocaleString(locale, text.replace(i18n("min"), ""))
            }

            value: kcm.activationGesturesSettings.accessXTimeoutDelay
            onValueChanged: kcm.activationGesturesSettings.accessXTimeoutDelay = value
        }
    }

    QQC2.CheckBox {
        Kirigami.FormData.label: i18n("When a gesture is used:")
        text: i18n("Display a confirmation dialog")

        KCM.SettingStateBinding {
            configObject: kcm.activationGesturesSettings
            settingName: "GestureConfirmation"
        }

        checked: kcm.activationGesturesSettings.gestureConfirmation
        onToggled: kcm.activationGesturesSettings.gestureConfirmation = checked
    }
    QQC2.CheckBox {
        text: i18n("Ring the System Bell")

        KCM.SettingStateBinding {
            configObject: kcm.activationGesturesSettings
            settingName: "AccessXBeep"
        }

        checked: kcm.activationGesturesSettings.accessXBeep
        onToggled: kcm.activationGesturesSettings.accessXBeep = checked
    }
    QQC2.CheckBox {
        text: i18n("Show a notification")

        KCM.SettingStateBinding {
            configObject: kcm.activationGesturesSettings
            settingName: "KeyboardNotifyAccess"
        }

        checked: kcm.activationGesturesSettings.keyboardNotifyAccess
        onToggled: kcm.activationGesturesSettings.keyboardNotifyAccess = checked
    }
    QQC2.Button {
        text: i18n("Configure Notifications…")
        icon.name: "preferences-desktop-notification"

        onClicked: kcm.configureKNotify()
    }
}
