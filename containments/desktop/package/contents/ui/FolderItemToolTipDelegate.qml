/***************************************************************************
 *   Copyright (C) 2014 by Eike Hein <hein@kde.org>                        *
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
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: tooltipContentItem

    property Item toolTip

    Layout.minimumWidth: width
    Layout.minimumHeight: height
    Layout.maximumWidth: width
    Layout.maximumHeight: height
    width: childrenRect.width + units.largeSpacing * 2
    height: childrenRect.height + units.largeSpacing * 2

    Row {
        x: units.largeSpacing
        y: x

        spacing: units.largeSpacing

        PlasmaCore.IconItem {
            id: icon

            anchors.verticalCenter: parent.verticalCenter

            width: toolTip ? units.iconSizes.medium : 0
            height: toolTip ? units.iconSizes.medium : 0

            source: toolTip ? toolTip.icon : null
        }

        Column {
            spacing: units.smallSpacing

            PlasmaExtras.Heading {
                level: 3
                elide: Text.ElideRight
                text: toolTip ? toolTip.mainText : ""
            }

            PlasmaComponents.Label {
                text: toolTip ? toolTip.subText : ""
                opacity: 0.5
            }
        }
    }
}
