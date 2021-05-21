/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras


Item {
    id: sectionDelegate

    width: parent.width
    height: childrenRect.height

    PlasmaExtras.Heading {
        id: sectionHeading
        anchors {
            left: parent.left
            leftMargin: PlasmaCore.Units.gridUnit
            right: parent.right
        }

        // important for RTL (otherwise label won't reverse)
        horizontalAlignment: Text.AlignLeft
        y: Math.round(PlasmaCore.Units.gridUnit / 4)
        level: 2
        // Force it to be uppercase or else if the first item in a section starts
        // with a lowercase letter, the header letter will be lowercase too!
        // Only applies to alphabetical characters in "All applications"
        text: section.length == 1 ? section.toUpperCase() : section
    }
    Item {
        width: parent.width
        height: Math.round(PlasmaCore.Units.gridUnit / 4)
        anchors.left: parent.left
        anchors.top: sectionHeading.bottom
    }
} // sectionDelegate
