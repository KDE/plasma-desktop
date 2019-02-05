/*
   Copyright (c) 2017 Eike Hein <hein@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.2 as QtControls
import org.kde.kirigami 2.3 as Kirigami
import org.kde.kcm 1.1

SimpleKCM {
    id: root

    ConfigModule.quickHelp: i18n("Launch Feedback")
    ConfigModule.buttons: ConfigModule.Help | ConfigModule.Defaults | ConfigModule.Apply

    function applyBusyCursorCurrentIndex() {
        if (kcm.busyCursorCurrentIndex == 0) {
            busyCursorDisabled.checked = true;
        } else if (kcm.busyCursorCurrentIndex == 1) {
            busyCursorStatic.checked = true;
        } else if (kcm.busyCursorCurrentIndex == 2) {
            busyCursorBlinking.checked = true;
        } else if (kcm.busyCursorCurrentIndex == 3) {
            busyCursorBouncing.checked = true;
        }
    }

    Kirigami.FormLayout {
        id: formLayout

        Connections {
            target: kcm

            onBusyCursorCurrentIndexChanged: applyBusyCursorCurrentIndex()

            onTaskManagerNotificationChanged: taskManagerNotification.checked = kcm.taskManagerNotification

            onNotificationTimeoutChanged: notificationTimeout.value = kcm.notificationTimeout
        }

        ColumnLayout {
            Kirigami.FormData.label: i18n("Cursor:")

            QtControls.RadioButton {
                id: busyCursorDisabled

                text: i18n("No Feedback")

                onToggled: kcm.busyCursorCurrentIndex = 0;
            }

            QtControls.RadioButton {
                id: busyCursorStatic

                text: i18n("Static")

                onToggled: kcm.busyCursorCurrentIndex = 1;
            }

            QtControls.RadioButton {
                id: busyCursorBlinking

                text: i18n("Blinking")

                onToggled: kcm.busyCursorCurrentIndex = 2;
            }

            QtControls.RadioButton {
                id:  busyCursorBouncing

                text: i18n("Bouncing")

                onToggled: kcm.busyCursorCurrentIndex = 3;
            }
        }

        QtControls.CheckBox {
            id: taskManagerNotification

            Kirigami.FormData.label: i18n("Task Manager:")

            text: i18n("Enable animation")

            checked: kcm.taskManagerNotification
            onToggled: kcm.taskManagerNotification = checked
        }

        QtControls.SpinBox {
            id: notificationTimeout

            Kirigami.FormData.label: i18n("Stop animation after:")

            stepSize: 1

            enabled: taskManagerNotification.checked

            value: kcm.notificationTimeout
            onValueChanged: kcm.notificationTimeout = value

            textFromValue: function(value, locale) { return i18np("%1 sec", "%1 secs", value)}
        }
    }

    Component.onCompleted: applyBusyCursorCurrentIndex()
}

