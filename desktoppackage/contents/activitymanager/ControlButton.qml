/*   vim:set foldmethod=marker:
 *
 *   Copyright (C) 2014 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.2
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.SvgItem {
    id: root

    signal clicked

    property alias icon: innerIcon.elementId

    width  : units.iconSizes.smallMedium
    height : units.iconSizes.smallMedium
    /*flat   : false*/

    svg: PlasmaCore.Svg { imagePath: "widgets/actionbutton" }
    elementId : mouseArea.pressed ? "pressed" : "normal"

    PlasmaCore.SvgItem {
        id: innerIcon

        anchors.centerIn: parent

        // TODO: Ugly X
        width  : units.iconSizes.small - (root.icon == "close" ? 4 : 0)
        height : width

        svg: PlasmaCore.Svg { imagePath: "widgets/configuration-icons" }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        onClicked: root.clicked()
    }
}


