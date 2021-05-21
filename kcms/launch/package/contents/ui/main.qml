/*
    SPDX-FileCopyrightText: 2017 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.2 as QtControls
import org.kde.kirigami 2.3 as Kirigami
import org.kde.kcm 1.3 as KCM
import org.kde.private.kcms.launchfeedback 1.0 as Private

KCM.SimpleKCM {
    id: root

    KCM.ConfigModule.quickHelp: i18n("Launch Feedback")

    Kirigami.FormLayout {
        id: formLayout

        readonly property bool cursorImmutable: kcm.launchFeedbackSettings.isImmutable("busyCursor") || kcm.launchFeedbackSettings.isImmutable("blinking") || kcm.launchFeedbackSettings.isImmutable("bouncing")

        QtControls.ButtonGroup {
            id: busyCursorGroup
            onCheckedButtonChanged: kcm.launchFeedbackSettings.setSelectedBusyCursor(checkedButton.settingName)
        }

        QtControls.RadioButton {
            id: busyCursorDisabled
            readonly property string settingName: "busyCursorDisabled"

            Kirigami.FormData.label: i18n("Cursor:")
            text: i18n("No Feedback")
            checked: kcm.launchFeedbackSettings.busyCursorDisabled
            QtControls.ButtonGroup.group: busyCursorGroup

            KCM.SettingStateBinding {
                configObject: kcm.launchFeedbackSettings
                settingName: parent.settingName
                extraEnabledConditions: !formLayout.cursorImmutable
            }
        }

        QtControls.RadioButton {
            id: busyCursorStatic
            readonly property string settingName: "busyCursorStatic"

            text: i18n("Static")
            checked: kcm.launchFeedbackSettings.busyCursorStatic
            QtControls.ButtonGroup.group: busyCursorGroup

            KCM.SettingStateBinding {
                configObject: kcm.launchFeedbackSettings
                settingName: parent.settingName
                extraEnabledConditions: !formLayout.cursorImmutable
            }
        }

        QtControls.RadioButton {
            id: busyCursorBlinking
            readonly property string settingName: "busyCursorBlinking"

            text: i18n("Blinking")
            checked: kcm.launchFeedbackSettings.busyCursorBlinking
            QtControls.ButtonGroup.group: busyCursorGroup

            KCM.SettingStateBinding {
                configObject: kcm.launchFeedbackSettings
                settingName: parent.settingName
                extraEnabledConditions: !formLayout.cursorImmutable
            }
        }

        QtControls.RadioButton {
            id: busyCursorBouncing
            readonly property string settingName: "busyCursorBouncing"

            text: i18n("Bouncing")
            checked: kcm.launchFeedbackSettings.busyCursorBouncing
            QtControls.ButtonGroup.group: busyCursorGroup

            KCM.SettingStateBinding {
                configObject: kcm.launchFeedbackSettings
                settingName: parent.settingName
                extraEnabledConditions: !formLayout.cursorImmutable
            }
        }

        QtControls.CheckBox {
            id: taskManagerNotification

            Kirigami.FormData.label: i18n("Task Manager:")

            text: i18n("Enable animation")

            checked: kcm.launchFeedbackSettings.taskbarButton
            onToggled: kcm.launchFeedbackSettings.taskbarButton = checked

            KCM.SettingStateBinding {
                configObject: kcm.launchFeedbackSettings
                settingName: "taskbarButton"
            }
        }

        QtControls.SpinBox {
            id: notificationTimeout

            Kirigami.FormData.label: i18n("Stop animation after:")

            // Don't resize if the suffix changes length
            Layout.minimumWidth: Kirigami.Units.gridUnit * 7

            stepSize: 1
            editable: true

            value: kcm.launchFeedbackSettings.cursorTimeout
            onValueModified: {
                kcm.launchFeedbackSettings.cursorTimeout = value
                kcm.launchFeedbackSettings.taskbarTimeout = value
            }

            textFromValue: function(value, locale) { return i18np("%1 second", "%1 seconds", value)}
            valueFromText: function(text, locale) { return parseInt(text) }

            KCM.SettingStateBinding {
                configObject: kcm.launchFeedbackSettings
                settingName: "cursorTimeout"
                extraEnabledConditions: taskManagerNotification.checked
            }
        }
    }
}
