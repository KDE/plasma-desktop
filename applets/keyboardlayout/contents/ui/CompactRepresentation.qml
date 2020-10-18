/*
 * SPDX-FileCopyrightText: 2014 Daniel Vrátil <dvratil@redhat.com>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.workspace.keyboardlayout 1.0

PlasmaComponents3.AbstractButton {
    id: root
    readonly property bool hasMoreThanOneKeyboardLayout: keyboardLayout.layouts.length > 1

    Layout.minimumWidth: PlasmaCore.Units.gridUnit
    Layout.minimumHeight: PlasmaCore.Units.gridUnit

    //text: "🇦🇶" // This code can be used to test emoji flags.
    text: keyboardLayout.currentLayoutDisplayName.toUpperCase()

    // This makes the max text size large enough to match whatever size the user tries to set
    font.pointSize: 128

    contentItem: PlasmaComponents3.Label {
        text: root.text
        color: PlasmaCore.ColorScope.textColor
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        // This prevents the actual text size from being too large
        fontSizeMode: Text.Fit
    }

    onClicked: keyboardLayout.nextLayout()

    KeyboardLayout {
        id: keyboardLayout
        function nextLayout() {
            var layouts = keyboardLayout.layouts;
            var index = (layouts.indexOf(keyboardLayout.currentLayout)+1) % layouts.length;
            keyboardLayout.currentLayout = layouts[index];
        }
    }
}
