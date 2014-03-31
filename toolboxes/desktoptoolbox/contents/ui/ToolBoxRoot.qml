/*
 *   Copyright 2011 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid 2.0


Item {
    id: main

    anchors.fill: parent

    signal minimumWidthChanged
    signal minimumHeightChanged
    signal maximumWidthChanged
    signal maximumHeightChanged
    signal preferredWidthChanged
    signal preferredHeightChanged

    property int iconSize: units.iconSizes.small
    property variant availScreenRect: Plasmoid.availableScreenRegion(plasmoid.screen)[0]
    property int iconWidth: units.iconSizes.smallMedium
    property int iconHeight: iconWidth

    onWidthChanged: {
        placeToolBoxTimer.restart();
    }
    onHeightChanged: {
        placeToolBoxTimer.restart();
    }

    MouseArea {
        id: toolBoxDismisser
        anchors.fill: parent
        visible: toolBoxItem.showing
        onPressed: toolBoxItem.showing = false
    }

    PlasmaCore.Svg {
        id: toolBoxSvg
        imagePath: "widgets/toolbox"
        property int rightBorder: elementSize("right").width
        property int topBorder: elementSize("top").height
        property int bottomBorder: elementSize("bottom").height
        property int leftBorder: elementSize("left").width
    }

    ToolBoxButton {
        id: toolBoxButton
        visible: false
        Component.onCompleted: {
            placeToolBox(plasmoid.configuration.ToolBoxButtonState);
            toolBoxButton.visible = true
        }
    }

    Timer {
        id: placeToolBoxTimer
        interval: 100
        repeat: false
        running: false
        onTriggered: {
            placeToolBox(plasmoid.configuration.ToolBoxButtonState);
        }
    }

    PlasmaExtras.ConditionalLoader {
        id: toolBoxItem
        property alias showing: toolBoxItem.when
        state: item ? item.state : "collapsed"
        property int margin: 22
        width: item ? item.width : 0
        height: item ? item.height : 0
        x: {
            var maxX = main.width - toolBoxItem.width - margin
            if (toolBoxButton.x > maxX) {
                return maxX;
            } else {
                return Math.max(toolBoxButton.x, margin);
            }
        }
        y: {
            var maxY = main.height - item.childrenRect.height - margin
            if (toolBoxButton.y > maxY) {
                return maxY;
            } else {
                return Math.max(toolBoxButton.y, margin);
            }
        }
        anchors.margins: 16

        source: Component {
            ToolBoxItem {
                showing: toolBoxItem.showing
            }
        }
    }

    function placeToolBox(ts) {
        var tx = Plasmoid.configuration.ToolBoxButtonX
        var ty = Plasmoid.configuration.ToolBoxButtonY

        switch (ts) {
        case "top":
            ty = 0;
            break;
        case "left":
            tx = 0;
            break;
        case "right":
            tx = main.width - toolBoxButton.width;
            break;
        case "bottom":
            ty = main.height - toolBoxButton.height;
            break;
        case "topleft":
            tx = 0;
            ty = 0;
            break;
        case "topright":
            tx = main.width - toolBoxButton.height;
            ty = 0;
            break;
        case "bottomleft":
            tx = 0;
            ty = main.height - toolBoxButton.height;
            break;
        case "bottomright":
        default:
            tx = main.width - toolBoxButton.height;
            ty = main.height - toolBoxButton.height;
            break;
        }
        //print("XXXY Setting toolbox to: " + ts + " " + tx + "x" + ty + " screen: " + main.width+ "x" + main.height+"");
        toolBoxButton.x = tx;
        toolBoxButton.y = ty;

    }
}
