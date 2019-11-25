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
import org.kde.private.kcms.launchfeedback 1.0 as Private

SimpleKCM {
    id: root

    ConfigModule.quickHelp: i18n("Launch Feedback")

    Kirigami.FormLayout {
        id: formLayout

        readonly property bool cursorImmutable: kcm.launchFeedbackSettings.isImmutable("busyCursor") || kcm.launchFeedbackSettings.isImmutable("blinking") || kcm.launchFeedbackSettings.isImmutable("bouncing")

        function setCursorSettings(feedback) {
            kcm.launchFeedbackSettings.busyCursor = feedback !== Private.KCM.None
            kcm.launchFeedbackSettings.blinking = feedback === Private.KCM.Blinking
            kcm.launchFeedbackSettings.bouncing = feedback === Private.KCM.Bouncing
        }

        QtControls.RadioButton {
            id: busyCursorDisabled

            enabled: !formLayout.cursorImmutable
            Kirigami.FormData.label: i18n("Cursor:")
            text: i18n("No Feedback")
            checked: !kcm.launchFeedbackSettings.busyCursor && !kcm.launchFeedbackSettings.blinking && !kcm.launchFeedbackSettings.bouncing
            onToggled: formLayout.setCursorSettings(Private.KCM.None)
        }

        QtControls.RadioButton {
            id: busyCursorStatic

            enabled: !formLayout.cursorImmutable
            text: i18n("Static")
            checked: kcm.launchFeedbackSettings.busyCursor && !kcm.launchFeedbackSettings.blinking && !kcm.launchFeedbackSettings.bouncing
            onToggled: formLayout.setCursorSettings(Private.KCM.Static)
        }

        QtControls.RadioButton {
            id: busyCursorBlinking

            enabled: !formLayout.cursorImmutable
            text: i18n("Blinking")
            checked: kcm.launchFeedbackSettings.busyCursor && kcm.launchFeedbackSettings.blinking && !kcm.launchFeedbackSettings.bouncing
            onToggled: formLayout.setCursorSettings(Private.KCM.Blinking)
        }

        QtControls.RadioButton {
            id: busyCursorBouncing

            enabled: !formLayout.cursorImmutable
            text: i18n("Bouncing")
            checked: kcm.launchFeedbackSettings.busyCursor && !kcm.launchFeedbackSettings.blinking && kcm.launchFeedbackSettings.bouncing
            onToggled: formLayout.setCursorSettings(Private.KCM.Bouncing)
        }

        QtControls.CheckBox {
            id: taskManagerNotification

            enabled: !kcm.launchFeedbackSettings.isImmutable("taskbarButton")
            Kirigami.FormData.label: i18n("Task Manager:")

            text: i18n("Enable animation")

            checked: kcm.launchFeedbackSettings.taskbarButton
            onToggled: kcm.launchFeedbackSettings.taskbarButton = checked
        }

        QtControls.SpinBox {
            id: notificationTimeout

            Kirigami.FormData.label: i18n("Stop animation after:")

            stepSize: 1
            editable: true

            enabled: taskManagerNotification.checked && !kcm.launchFeedbackSettings.isImmutable("cursorTimeout")

            value: kcm.launchFeedbackSettings.cursorTimeout
            onValueModified: {
                kcm.launchFeedbackSettings.cursorTimeout = value
                kcm.launchFeedbackSettings.taskbarTimeout = value
            }

            textFromValue: function(value, locale) { return i18np("%1 sec", "%1 secs", value)}
            valueFromText: function(text, locale) { return parseInt(text) }
        }
    }
}
