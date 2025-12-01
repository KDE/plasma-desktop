/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kquickcontrols
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid
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
            patterns: ShortcutPattern.Modifier | ShortcutPattern.ModifierAndKey
            onKeySequenceModified: root.unsavedChanges = keySequence !== Plasmoid.globalShortcut
        }
    }
}
