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
    spacing: Kirigami.Units.smallSpacing

    Kirigami.FormLayout {
        id: formLayout

        QQC2.CheckBox {
            Kirigami.FormData.label: i18nc("@label", "Color blindness correction:")
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
            Kirigami.FormData.label: i18nc("@label", "Mode:")
            currentIndex: kcm.colorblindnessCorrectionSettings.mode
            textRole: "text"
            valueRole: "value"
            model: [
                { value: 0, text: i18nc("@option", "Protanopia (red weak)") },
                { value: 1, text: i18nc("@option", "Deuteranopia (green weak)") },
                { value: 2, text: i18nc("@option", "Tritanopia (blue-yellow)") },
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

        Kirigami.Separator {
            Kirigami.FormData.isSection: true
        }
    }

    RowLayout {
        id: previewArea
        Layout.fillWidth: true
        spacing: Kirigami.Units.smallSpacing
        Layout.topMargin: Kirigami.Units.largeSpacing

        Item {
            Layout.fillWidth: true
        }

        Repeater {
            model: [
                { name: i18n("Red"), colors: ["Red", "Orange", "Yellow"] },
                { name: i18n("Green"), colors: ["Green", "LimeGreen", "Lime"] },
                { name: i18n("Blue"), colors: ["Blue", "DeepSkyBlue", "Aqua"] },
                { name: i18n("Purple"), colors: ["Purple", "Fuchsia", "Violet"] },
            ]

            delegate: Column {
                spacing: 0

                Repeater {
                    model: modelData.colors
                    delegate: Rectangle {
                        width: Kirigami.Units.gridUnit * 5
                        height: Kirigami.Units.gridUnit * 5
                        color: modelData
                    }
                }

                QQC2.Label {
                    anchors.horizontalCenter: parent.horizontalCenter
                    text: modelData.name
                }
            }
        }

        Item {
            Layout.fillWidth: true
        }
    }
}
