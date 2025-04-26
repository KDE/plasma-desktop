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

    property bool unsavedChanges: false

    function saveConfig() {
        Plasmoid.globalShortcut = button.keySequence
        unsavedChanges = false
    }

    Kirigami.FormLayout {

        KeySequenceItem {
            id: button
            Kirigami.FormData.label: i18nc("@action:button set keyboard shortcut for", "Activate widget as if clicked:")
            keySequence: Plasmoid.globalShortcut
            modifierOnlyAllowed: true
            onKeySequenceModified: root.unsavedChanges = keySequence !== Plasmoid.globalShortcut
        }
    }
}
