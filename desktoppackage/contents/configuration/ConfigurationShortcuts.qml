/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 2.3 as QQC2
import QtQuick.Layouts 1.0
import org.kde.kquickcontrols 2.0
import org.kde.kirigami 2.14 as Kirigami

Kirigami.ScrollablePage {
    id: root

    title: i18n("Shortcuts")

    signal configurationChanged
    function saveConfig() {
        plasmoid.globalShortcut = button.keySequence
    }

    Kirigami.FormLayout {

        KeySequenceItem {
            Kirigami.FormData.label: i18nd("plasma_shell_org.kde.plasma.desktop", "Use this shortcut to activate the applet:")
            id: button
            keySequence: plasmoid.globalShortcut
            onCaptureFinished: {
                if (keySequence !== plasmoid.globalShortcut) {
                    root.configurationChanged();
                }
            }
        }
    }
}
