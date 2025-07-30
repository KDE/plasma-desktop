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

    Kirigami.FormLayout {
        anchors.right: parent.right
        anchors.left: parent.left

        QQC2.CheckBox {
            id: showTextCheckbox

            enabled: Plasmoid.formFactor === PlasmaCore.Types.Horizontal

            text: i18n("Show active application's name on Panel button")

            // We do manual state handling rather than making cfg_showText an
            // alias property because we want to force the checkbox to look
            // unchecked when it's disabled, regardless of its actual underlying
            // state.
            checked: enabled ? root.cfg_showText : false
            onToggled: root.cfg_showText = checked
        }

        QQC2.Label {
            Layout.fillWidth: true
            // Arbitrary maximum length to make it wrap earlier, because long
            // unwrapped text is ugly and harder to read.
            Layout.maximumWidth: Kirigami.Units.gridUnit * 25

            visible: !showTextCheckbox.enabled

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
