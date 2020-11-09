/*
 * Copyright 2018 Tomaz Canabrava <tcanabrava@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.6
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as QQC2
import org.kde.kcm 1.1 as KCM
import org.kde.kirigami 2.3 as Kirigami

Kirigami.FormLayout {
    QQC2.CheckBox {
        id: mouseKeys

        Kirigami.FormData.label:  i18n("Use number pad to move cursor:")
        text: i18n("Enable")

        enabled: !kcm.mouseSettings.isImmutable("MouseKeys")

        checked: kcm.mouseSettings.mouseKeys
        onToggled: kcm.mouseSettings.mouseKeys = checked
    }
    QQC2.CheckBox {
        Kirigami.FormData.label: i18n("When a gesture is used:")
        text: i18n("Display a confirmation dialog")

        enabled: !kcm.keyboardSettings.isImmutable("GestureConfirmation")

        checked: kcm.keyboardSettings.gestureConfirmation
        onToggled: kcm.keyboardSettings.gestureConfirmation = checked
    }
    QQC2.CheckBox {
        text: i18n("Ring the System Bell")

        enabled: !kcm.keyboardSettings.isImmutable("Gestures")

        checked: kcm.keyboardSettings.gestures
        onToggled: kcm.keyboardSettings.gestures = checked
    }
    QQC2.CheckBox {
        text: i18n("Show a notification")

        enabled: !kcm.keyboardSettings.isImmutable("KeyboardNotifyAccess")

        checked: kcm.keyboardSettings.keyboardNotifyAccess
        onToggled: kcm.keyboardSettings.keyboardNotifyAccess = checked
    }

    Item {
        Kirigami.FormData.isSection: true
    }

    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Acceleration delay:")

        enabled: !kcm.mouseSettings.isImmutable("AccelerationDelay")

        value: kcm.mouseSettings.accelerationDelay
        onValueChanged: kcm.mouseSettings.accelerationDelay = value
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Repeat interval:")

        enabled: !kcm.mouseSettings.isImmutable("RepetitionInterval")

        value: kcm.mouseSettings.repetitionInterval
        onValueChanged: kcm.mouseSettings.repetitionInterval = value
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Acceleration time:")

        enabled: !kcm.mouseSettings.isImmutable("AccelerationTime")

        value: kcm.mouseSettings.accelerationTime
        onValueChanged: kcm.mouseSettings.accelerationTime = value
    }
    QQC2.SpinBox {
        Kirigami.FormData.label:  i18n("Maximum speed:")

        enabled: !kcm.mouseSettings.isImmutable("MaxSpeed")

        value: kcm.mouseSettings.maxSpeed
        onValueChanged: kcm.mouseSettings.maxSpeed = value
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18n("Acceleration profile:")

        enabled: !kcm.mouseSettings.isImmutable("ProfileCurve")

        value: kcm.mouseSettings.profileCurve
        onValueChanged: kcm.mouseSettings.profileCurve = value
    }
}
