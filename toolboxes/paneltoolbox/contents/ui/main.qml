/*
 *   Copyright 2011 Sebastian Kügler <sebas@kde.org>
 *   Copyright 2013 Marco Martin <mart@kde.org>
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
import org.kde.plasma.plasmoid 2.0

Item {
    id: main

    width: isVertical ? units.iconSizes.medium : units.iconSizes.smallMedium + units.smallSpacing * 2
    height: isVertical ? units.iconSizes.smallMedium + units.smallSpacing * 2 : units.iconSizes.medium
    property bool isVertical: plasmoid.formFactor == 3
    visible: !plasmoid.immutable

    anchors {
        left: undefined
        top: undefined
        right: isVertical ? undefined : parent.right
        bottom: isVertical ? parent.bottom : undefined
        verticalCenter: isVertical ? undefined : parent.verticalCenter
        horizontalCenter: isVertical ? parent.horizontalCenter : undefined
    }

    PlasmaCore.Svg {
        id: toolBoxSvg
        imagePath: "widgets/line"
    }

    PlasmaCore.SvgItem {
        id: toolBoxButton
        svg: toolBoxSvg
        width: {
            if (isVertical) {
                parent.width
            } else {
                naturalSize.width
            }
        }
        height: {
            if (isVertical) {
                naturalSize.height
            } else {
                parent.height
            }
        }
        elementId: {
            if (isVertical) {
                return "horizontal-line"
            } else {
                return "vertical-line"
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors {
            fill: parent
            topMargin: isVertical ? 5 : 0
            leftMargin: isVertical ? 0 : 5
        }
        hoverEnabled: true
        onClicked: {
            main.Plasmoid.action("configure").trigger()
        }
        PlasmaCore.IconItem {
            anchors {
                fill: parent
                margins: units.smallSpacing
            }
            source: "plasma"
            enabled: mouseArea.containsMouse || main.Plasmoid.userConfiguring
        }
    }
}
