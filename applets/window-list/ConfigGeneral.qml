/*
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.plasma.plasmoid
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    property bool cfg_showText: Plasmoid.configuration.showText
    property bool cfg_showIcon: Plasmoid.configuration.showIcon
    property bool cfg_openOnHover: Plasmoid.configuration.openOnHover
    property int cfg_hoverOpenDelay: Plasmoid.configuration.hoverOpenDelay

    Kirigami.FormLayout {
        anchors.right: parent.right
        anchors.left: parent.left

        QQC2.ComboBox {
            id: displayModeComboBox

            Kirigami.FormData.label: i18nc("@label:listbox", "For active application, show:")

            enabled: Plasmoid.formFactor === PlasmaCore.Types.Horizontal

            model: [
                i18nc("@item:inlistbox what to show for active application in horizontal panel", "Icon and name"),
                i18nc("@item:inlistbox what to show for active application in horizontal panel", "Icon"),
                i18nc("@item:inlistbox what to show for active application in horizontal panel", "Name")
            ]

            property var mode: 0

            onModeChanged: {
                if (mode === 0) {
                    root.cfg_showIcon = true
                    root.cfg_showText = true
                } else if (mode === 1) {
                    root.cfg_showIcon = true
                    root.cfg_showText = false
                } else if (mode === 2) {
                    root.cfg_showIcon = false
                    root.cfg_showText = true
                }
            }

            onCurrentIndexChanged: mode = currentIndex

            Component.onCompleted: {
                if (root.cfg_showIcon && root.cfg_showText) {
                    mode = 0
                } else if (root.cfg_showIcon && !root.cfg_showText) {
                    mode = 1
                } else if (!root.cfg_showIcon && root.cfg_showText) {
                    mode = 2
                } else {
                    // Invalid state, fallback to icon only
                    mode = 1
                    root.cfg_showIcon = true
                    root.cfg_showText = false
                }
                currentIndex = mode
            }
        }

        QQC2.Label {
            Layout.fillWidth: true
            // Arbitrary maximum length to make it wrap earlier, because long
            // unwrapped text is ugly and harder to read.
            Layout.maximumWidth: Kirigami.Units.gridUnit * 25

            visible: !displayModeComboBox.enabled
        
            text: Plasmoid.formFactor === PlasmaCore.Types.Vertical ?
                // On a vertical panel
                i18n("Only icons can be shown when the Panel is vertical.") :
                // On the desktop
                i18n("Not applicable when the widget is on the Desktop.")
            textFormat: Text.PlainText
            wrapMode: Text.Wrap
            font: Kirigami.Theme.smallFont
        }

        Item {
            Kirigami.FormData.isSection: true
        }


        RowLayout {
            spacing: Kirigami.Units.smallSpacing
            Kirigami.FormData.label: i18nc("@label:checkbox", "Open on hover:")

             QQC2.CheckBox {
                id: openOnHoverCheckbox

                text: i18nc("@option:check open-on-hover is enabled, with the following delay:", "Enabled, with delay:")

                checked: root.cfg_openOnHover
                onToggled: root.cfg_openOnHover = checked
            }

            QQC2.SpinBox {
                id: hoverDelaySpinBox

                from: 0
                to: 1000
                stepSize: 50

                enabled: root.cfg_openOnHover

                value: root.cfg_hoverOpenDelay

                onValueChanged: root.cfg_hoverOpenDelay = value

                textFromValue: function(value, locale) {
                    return i18np("%1 ms", "%1 ms", value)
                }

                validator: IntValidator {
                    bottom: hoverDelaySpinBox.from
                    top: hoverDelaySpinBox.to
                }

                valueFromText: (text, locale) => {
                    return Number.fromLocaleString(locale, text.replace(i18ncp("short for millisecond(s)", "ms", "ms"), ""))
                }

                Accessible.name: i18nc("@label:spinbox accessible", "Hover open delay %1", textFromValue(value))

                KCM.SettingStateBinding {
                    configObject: Plasmoid.configuration
                    settingName: "hoverOpenDelay"
                    extraEnabledConditions: root.cfg_openOnHover
                }
            }
        }
    }
}
