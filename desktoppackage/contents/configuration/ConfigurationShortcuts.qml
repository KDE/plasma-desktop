/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 2.3 as QQC2
import QtQuick.Layouts 1.0
import org.kde.kquickcontrols 2.0
import org.kde.kirigami 2.14 as Kirigami
import org.kde.plasma.plasmoid 2.0
import org.kde.kcmutils as KCM

KCM.SimpleKCM {
    id: root

    title: i18n("Shortcuts")

    signal configurationChanged
    function saveConfig() {
        Plasmoid.globalShortcut = button.keySequence
    }

    ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            Layout.fillWidth: true
            text: i18nd("plasma_shell_org.kde.plasma.desktop", "This shortcut will activate the applet as though it had been clicked.")
            textFormat: Text.PlainText
            wrapMode: Text.WordWrap
        }

        KeySequenceItem {
            id: button
            keySequence: Plasmoid.globalShortcut
            modifierOnlyAllowed: true
            onCaptureFinished: {
                if (keySequence !== Plasmoid.globalShortcut) {
                    root.configurationChanged();
                }
            }
        }
    }
}
