/*
 *  SPDX-FileCopyrightText: 2025 Shubham Arora <contact@shubhamarora.dev>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.13

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    property bool cfg_openOnHover: Plasmoid.configuration.openOnHover
    property int cfg_hoverOpenDelay: Plasmoid.configuration.hoverOpenDelay

    Kirigami.FormLayout {
        anchors.right: parent.right
        anchors.left: parent.left

        QQC2.CheckBox {
            id: openOnHoverCheckbox

            text: i18n("Open Window List popup on hover")

            checked: root.cfg_openOnHover
            onToggled: root.cfg_openOnHover = checked
        }

        QQC2.SpinBox {
            id: hoverDelaySpinBox

            Kirigami.FormData.label: i18nc("Hover open delay", "Delay:")

            from: 50
            to: 5000
            stepSize: 50

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

