/*
*   Copyright 2013 by Sebastian Kügler <sebas@kde.org>
*   Copyright 2014 by Martin Gräßlin <mgraesslin@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Library General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

Column {
    id: tooltipContentItem

    property Item toolTip

    property int preferredTextWidth: theme.mSize(theme.defaultFont).width * 40
    property int _s: units.largeSpacing / 2

    Layout.minimumWidth: childrenRect.width + _s
    Layout.minimumHeight: childrenRect.height + units.largeSpacing
    Layout.maximumWidth: childrenRect.width + _s
    Layout.maximumHeight: childrenRect.height + units.largeSpacing
    width: childrenRect.width + _s
    height: childrenRect.height + units.largeSpacing
    spacing: units.largeSpacing

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        width: childrenRect.width
        height: childrenRect.height
        spacing: units.largeSpacing
        Repeater {
            model: toolTip.windows
            PlasmaCore.WindowThumbnail {
                // hardcoded values from old implementation
                width: 200
                height: 150
                winId: modelData
            }
        }
    }

    Row {
        width: childrenRect.width + _s
        height: childrenRect.height + units.largeSpacing
        spacing: units.largeSpacing

        Item {
            id: imageContainer
            visible: toolTip != null && toolTip.icon != null
            width: tooltipIcon.width
            height: tooltipIcon.height
            x: _s
            y: _s

            PlasmaCore.IconItem {
                id: tooltipIcon
                x: _s
                width: toolTip != undefined && toolTip.icon != null ? units.iconSizes.desktop : 0
                height: width
                source: toolTip != undefined && toolTip.icon != null ? toolTip.icon : ""
            }
        }

        Column {
            id: mainColumn
            x: _s
            y: _s

            //This instance is purely for metrics
            PlasmaExtras.Heading {
                id: tooltipMaintextPlaceholder
                visible: false
                level: 3
                text: toolTip ? toolTip.mainText : ""
            }
            PlasmaExtras.Heading {
                id: tooltipMaintext
                level: 3
                width: Math.min(tooltipMaintextPlaceholder.width, preferredTextWidth)
                //width: 400
                elide: Text.ElideRight
                text: toolTip ? toolTip.mainText : ""
            }
            PlasmaComponents.Label {
                id: tooltipSubtext
                width: tooltipContentItem.preferredTextWidth
                wrapMode: Text.WordWrap
                text: toolTip ? toolTip.subText : ""
                opacity: 0.5
            }
        }
    }
}
