/***************************************************************************
 *   Copyright (C) 2012-2013 by Eike Hein <hein@kde.org>                   *
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

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Flow {
    property bool animating: false

    layoutDirection: Qt.application.layoutDirection

    property int rows: Math.floor(height / children[0].height)
    property int columns: Math.floor(width / children[0].width)

    move: Transition {
        SequentialAnimation {
            PropertyAction { target: taskList; property: "animating"; value: true }

            NumberAnimation {
                properties: "x, y"
                easing.type: Easing.OutQuad
                duration: PlasmaCore.Units.longDuration
            }

            PropertyAction { target: taskList; property: "animating"; value: false }
        }
    }
}
