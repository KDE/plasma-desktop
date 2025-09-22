/*
    SPDX-FileCopyrightText: 2023 Fushan Wen <qydwhotmail@gmail.com>
    SPDX-FileCopyrightText: 2024 Thomas Duckworth <tduck973564@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kcmutils as KCM
import org.kde.kirigami as Kirigami

ColumnLayout {
    spacing: Kirigami.Units.gridUnit

    QQC2.Label {
        Layout.fillWidth: true

        text: i18nc("@info", "If you have trouble with certain colors on the screen, these filters can change them into other colors.")
        textFormat: Text.PlainText
        wrapMode: Text.WordWrap
        horizontalAlignment: Text.AlignHCenter
    }

    Kirigami.FormLayout {
        id: formLayout

        QQC2.CheckBox {
            text: i18nc("@option check, Enable color blindness correction effect", "Enable")

            KCM.SettingStateBinding {
                configObject: kcm.colorblindnessCorrectionSettings
                settingName: "ColorblindnessCorrection"
            }

            checked: kcm.colorblindnessCorrectionSettings.colorblindnessCorrection
            onToggled: kcm.colorblindnessCorrectionSettings.colorblindnessCorrection = checked
        }

        QQC2.ComboBox {
            id: colorComboBox
            Kirigami.FormData.label: i18nc("@label:listbox Difficulty seeing any of the following colors on the screen", "Problematic colors:")
            currentIndex: kcm.colorblindnessCorrectionSettings.mode
            textRole: "text"
            valueRole: "value"
            model: [
                { value: 0, text: i18nc("@option", "Red & purple (Protanopia)") },
                { value: 1, text: i18nc("@option", "Green & purple (Deuteranopia)") },
                { value: 2, text: i18nc("@option", "Yellow, green & purple (Tritanopia)") },
                { value: 3, text: i18nc("@option", "All (grayscale mode)") },
            ]

            Layout.preferredWidth: Kirigami.Units.gridUnit * 15

            KCM.SettingStateBinding {
                configObject: kcm.colorblindnessCorrectionSettings
                settingName: "Mode"
                extraEnabledConditions: kcm.colorblindnessCorrectionSettings.colorblindnessCorrection
            }

            onActivated: kcm.colorblindnessCorrectionSettings.mode = currentValue
        }

        ColumnLayout {
            Kirigami.FormData.label: i18nc("@label", "Intensity:")
            Kirigami.FormData.buddyFor: intensitySlider
            spacing: Kirigami.Units.smallSpacing

            QQC2.Slider {
                id: intensitySlider
                Layout.preferredWidth: Kirigami.Units.gridUnit * 15

                KCM.SettingStateBinding {
                    configObject: kcm.colorblindnessCorrectionSettings
                    settingName: "Intensity"
                    extraEnabledConditions: kcm.colorblindnessCorrectionSettings.colorblindnessCorrection
                }

                from: 0.05 // 0.0 just rolls over to 1? hacky
                to: 1.0
                value: kcm.colorblindnessCorrectionSettings.intensity
                onMoved: kcm.colorblindnessCorrectionSettings.intensity = value
            }
            RowLayout {
                spacing: 0

                QQC2.Label {
                    text: i18nc("@label Mild color blindness correction intensity", "Mild")
                    textFormat: Text.PlainText
                }
                Item {
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: i18nc("@label Intense color blindness correction intensity", "Intense")
                    textFormat: Text.PlainText
                }
            }
        }
    }

    Kirigami.Separator {
        Layout.fillWidth: true
    }

    ColumnLayout {
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            Layout.fillWidth: true

            text: i18nc("@info", "Adjusted colors:")
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
            horizontalAlignment: Text.AlignHCenter
        }

        RowLayout {
            id: previewArea
            Layout.fillWidth: true
            spacing: Kirigami.Units.smallSpacing

            Item {
                Layout.fillWidth: true
            }

            Repeater {
                model: [
                    { name: i18n("Reds:"), colors: ["Red", "Orange", "Yellow"] },
                    { name: i18n("Greens:"), colors: ["Green", "LimeGreen", "Lime"] },
                    { name: i18n("Blues:"), colors: ["Blue", "DeepSkyBlue", "Aqua"] },
                    { name: i18n("Purples:"), colors: ["Purple", "Fuchsia", "Violet"] },
                ]

                delegate: Column {
                    spacing: Kirigami.Units.smallSpacing

                    QQC2.Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: modelData.name
                    }

                    Repeater {
                        model: modelData.colors
                        delegate: Rectangle {
                            width: Kirigami.Units.gridUnit * 4
                            height: Kirigami.Units.gridUnit * 4
                            color: modelData
                        }
                    }
                }
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }
}
