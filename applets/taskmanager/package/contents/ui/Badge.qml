/***************************************************************************
 *   Copyright (C) 2018 Kai Uwe Broulik <kde@privat.broulik.de>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.4

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Rectangle {
    id: badgeRect

    property alias text: label.text
    property alias textColor: label.color
    property int number: 0

    implicitWidth: Math.max(height, Math.round(label.contentWidth + radius / 2)) // Add some padding around.
    color: PlasmaCore.Theme.highlightColor
    radius: height / 2

    PlasmaComponents.Label {
        id: label
        anchors.centerIn: parent
        width: height
        height: Math.min(PlasmaCore.Units.gridUnit * 2, Math.round(parent.height))
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        fontSizeMode: Text.VerticalFit
        font.pointSize: 1024
        minimumPointSize: 5
        color: PlasmaCore.Theme.highlightedTextColor
        text: {
            if (badgeRect.number < 0) {
                return i18nc("Invalid number of new messages, overlay, keep short", "—");
            } else if (badgeRect.number > 9999) {
                return i18nc("Over 9999 new messages, overlay, keep short", "9,999+");
            } else {
                return badgeRect.number.toLocaleString(Qt.locale(), 'f', 0);
            }
        }
    }
}
