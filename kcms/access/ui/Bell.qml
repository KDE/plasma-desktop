/*
    SPDX-FileCopyrightText: 2018 Tomaz Canabrava <tcanabrava@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.6
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Dialogs 6.3 as Dialogs
import org.kde.kcmutils as KCM
import org.kde.kquickcontrols 2.0 as KQuickAddons
import org.kde.kirigami 2.3 as Kirigami

Kirigami.FormLayout {

    Dialogs.FileDialog {
        id: fileDialog
        title: i18n("Please choose an audio file")
        nameFilters: [i18nc("Name filters in a file dialog. Do not translate `(*.ogg *.oga *.wav)`",
                            "ogg, oga, and wav audio files (*.ogg *.oga *.wav)")]
        onAccepted: {
            kcm.bellSettings.customBellFile = fileDialog.selectedFile
        }
    }

    QQC2.CheckBox {
        id: systemBell

        Kirigami.FormData.label: i18n("Audible bell:")
        text: i18nc("Enable the system bell", "Enable")
        checked: kcm.bellSettings.systemBell
        onToggled: kcm.bellSettings.systemBell = checked
        KCM.SettingStateBinding {
            configObject: kcm.bellSettings
            settingName: "SystemBell"
        }

        Accessible.role: Accessible.CheckBox
        Accessible.name: i18n("Enable audible bell")
        Accessible.description: i18n("Emits a sound whenever certain keys are pressed")
    }

    RowLayout {
        Kirigami.FormData.label: i18nc("Defines if the system will use a sound system bell", "Custom sound:")
        spacing: 0

        QQC2.CheckBox {
            id: customBell
            Layout.alignment: Qt.AlignVCenter

            KCM.SettingStateBinding {
                configObject: kcm.bellSettings
                settingName: "CustomBell"
                extraEnabledConditions: kcm.bellSettings.systemBell
            }

            checked: kcm.bellSettings.customBell
            onToggled: kcm.bellSettings.customBell= checked

            Accessible.role: Accessible.CheckBox
            Accessible.name: i18n("Enable custom sound for the audible bell")
        }

        QQC2.TextField {
            id: textEdit

            text: kcm.bellSettings.customBellFile

            KCM.SettingStateBinding {
                configObject: kcm.bellSettings
                settingName: "CustomBellFile"
                extraEnabledConditions: kcm.bellSettings.customBell
            }

            onEditingFinished: kcm.bellSettings.customBellFile = textEdit.text

            Accessible.role: Accessible.EditableText
            Accessible.name: i18n("Text field containing the path for an audio file")
        }
        QQC2.Button {
            icon.name: "folder"
            QQC2.ToolTip.visible: down
            QQC2.ToolTip.text: i18n("Search audio file for the system bell")
            KCM.SettingStateBinding {
                configObject: kcm.bellSettings
                settingName: "CustomBellFile"
                extraEnabledConditions: kcm.bellSettings.customBell
            }

            onClicked: fileDialog.open()

            Accessible.role: Accessible.Button
            Accessible.name: i18n("Button to search for an audio file")
            Accessible.description: i18n("Opens a new dialog to search for an audio file")
        }
    }
    Item {
        Kirigami.FormData.isSection: true
    }
    QQC2.CheckBox {
        id: visibleBell

        Kirigami.FormData.label: i18n("Visual bell:")
        text: i18nc("Enable visual bell", "Enable")

        KCM.SettingStateBinding {
            configObject: kcm.bellSettings
            settingName: "VisibleBell"
        }

        checked: kcm.bellSettings.visibleBell
        onToggled: kcm.bellSettings.visibleBell = checked

        Accessible.role: Accessible.CheckBox
        Accessible.name: i18n("Enable visual bell")
        Accessible.description: i18n("Flashes the screen whenever certain keys are pressed")
    }

    QQC2.RadioButton {
        id: invertScreen

        text: i18n("Invert screen colors")

        KCM.SettingStateBinding {
            configObject: kcm.bellSettings
            settingName: "InvertScreen"
            extraEnabledConditions: kcm.bellSettings.visibleBell
        }

        checked: kcm.bellSettings.invertScreen
        onToggled: kcm.bellSettings.invertScreen = checked

        Accessible.role: Accessible.RadioButton
        Accessible.name: text
    }
    RowLayout {
        enabled: kcm.bellSettings.visibleBell

        QQC2.RadioButton {
            id: flashScreen

            text: i18n("Flash screen")

            KCM.SettingStateBinding {
                configObject: kcm.bellSettings
                settingName: "InvertScreen"
            }

            checked: !kcm.bellSettings.invertScreen
            onToggled: kcm.bellSettings.invertScreen = !checked

            Accessible.role: Accessible.RadioButton
            Accessible.name: text
            Accessible.description: i18n("Briefly flashes the screen with the color selected using the color chooser next to this radio button")
        }
        KQuickAddons.ColorButton {
            text: i18nc("Color of the system bell","Color")
            // avoid to show text outside button
            display: QQC2.AbstractButton.IconOnly

            KCM.SettingStateBinding {
                configObject: kcm.bellSettings
                settingName: "VisibleBellColor"
            }

            color: kcm.bellSettings.visibleBellColor
            onAccepted: color => kcm.bellSettings.visibleBellColor = color

            Accessible.role: Accessible.ColorChooser
            Accessible.name: i18n("Color of the visible bell")
            Accessible.description: i18n("Color chooser for the flash screen color")
        }
    }
    QQC2.SpinBox {
        Kirigami.FormData.label: i18nc("Duration of the system bell", "Duration:")

        from: 100
        to: 2000

        KCM.SettingStateBinding {
            configObject: kcm.bellSettings
            settingName: "VisibleBellPause"
            extraEnabledConditions: kcm.bellSettings.visibleBell
        }

        value: kcm.bellSettings.visibleBellPause
        onValueModified: kcm.bellSettings.visibleBellPause = value
        textFromValue: function(value) { return value + " ms"}

        Accessible.role: Accessible.SpinBox
        Accessible.name: i18n("Duration of the visible bell")
        Accessible.description: i18n("Length of time the color will flash on the screen")
    }
}
