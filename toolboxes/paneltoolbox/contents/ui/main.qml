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

import QtQuick 2.8
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.plasmoid 2.0

Item {
    id: main

    width: isVertical ? units.iconSizes.medium : units.iconSizes.smallMedium + units.smallSpacing * 2
    height: isVertical ? units.iconSizes.smallMedium + units.smallSpacing * 2 : units.iconSizes.medium
    property bool isVertical: plasmoid.formFactor === 3

    z: 999

    states: [
        State {
            when: plasmoid.editMode
            PropertyChanges {
                target: main
                visible: true
            }
            PropertyChanges {
                target: main
                opacity: mouseArea.containsMouse || plasmoid.userConfiguring ? 1 : 0.5
            }
        },
        State {
            when: !plasmoid.editMode
            PropertyChanges {
                target: main
                visible: false
            }
            PropertyChanges {
                target: main
                opacity: 0
            }
        }
    ]
    Behavior on opacity {
        OpacityAnimator {
            duration: units.longDuration;
            easing.type: Easing.InOutQuad;
        }
    }

    LayoutMirroring.enabled: (Qt.application.layoutDirection === Qt.RightToLeft)
    anchors {
        left: undefined
        top: undefined
        right: isVertical || !parent ? undefined : parent.right
        bottom: isVertical && parent ? parent.bottom : undefined
        verticalCenter: isVertical || !parent ? undefined : parent.verticalCenter
        horizontalCenter: isVertical && parent ? parent.horizontalCenter : undefined
    }

    PlasmaCore.SvgItem {
        id: toolBoxIcon
        svg: PlasmaCore.Svg {
            id: iconSvg
            imagePath: "widgets/configuration-icons"
        }
        elementId: "configure"

        anchors.centerIn: mouseArea
        width: units.iconSizes.small
        height: width
    }

    Connections {
        target: plasmoid
        function onUserConfiguringChanged() {
            if (plasmoid.userConfiguring) {
                plasmoid.editMode = true;
                toolTipArea.hideToolTip();
            }
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: enabled
        enabled: plasmoid.editMode || plasmoid.userConfiguring
        onClicked: {
            main.Plasmoid.action("configure").trigger()
        }

        PlasmaCore.ToolTipArea {
            id: toolTipArea
            anchors.fill: parent
            mainText: i18nd("plasma_toolbox_org.kde.paneltoolbox", "Configure Panel...")
            icon: "configure"
            enabled: mouseArea.containsMouse
        }
    }
}
