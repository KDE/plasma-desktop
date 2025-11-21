/*
 *  SPDX-FileCopyrightText: 2022 Nate Graham <nate@kde.org>
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

    property bool cfg_showText: Plasmoid.configuration.showText
    property bool cfg_showIcon: Plasmoid.configuration.showIcon

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
    }
}
