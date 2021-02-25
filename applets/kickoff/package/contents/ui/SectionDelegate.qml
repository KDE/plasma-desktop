/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012 Marco Martin <mart@kde.org>
    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
        level: 4
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
