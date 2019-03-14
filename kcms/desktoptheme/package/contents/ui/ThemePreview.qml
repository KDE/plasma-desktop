/*
   Copyright (c) 2016 David Rosca <nowrep@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
import QtQuick 2.4
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.4 as Kirigami
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: root
    property string themeName

    Item {
        id: backgroundMask
        anchors.fill: parent
        clip: true

        PlasmaCore.FrameSvgItem {
            id: background
            // Normalize margins around background.
            // Some themes like "Air" have huge transparent margins which would result in too small container area.
            // Sadly all of the breathing, shadow and border sizes are in one single margin value,
            // but for typical themes the border is the smaller part the margin and should be in the size of
            // Units.largeSpacing, to which we add another Units.largeSpacing for margin of the visual content
            // Ideally Plasma::FrameSvg exposes the transparent margins one day.
            readonly property int generalMargin: 2 * Kirigami.Units.largeSpacing
            anchors {
                fill: parent
                topMargin: -margins.top + generalMargin
                bottomMargin: -margins.bottom + generalMargin
                leftMargin: -margins.left + generalMargin
                rightMargin: -margins.right + generalMargin
            }
            imagePath: "widgets/background"
        }
    }

    RowLayout {
        id: contents
        anchors {
            fill: parent
            topMargin: background.generalMargin
            bottomMargin: background.generalMargin
            leftMargin: background.generalMargin
            rightMargin: background.generalMargin
        }

        // Icons
        ColumnLayout {
            id: icons
            Layout.fillHeight: true

            PlasmaCore.IconItem {
                id: computerIcon
                Layout.fillHeight: true
                source: "computer"
            }

            PlasmaCore.IconItem {
                id: applicationsIcon
                Layout.fillHeight: true
                source: "applications-other"
            }

            PlasmaCore.IconItem {
                id: logoutIcon
                Layout.fillHeight: true
                source: "system-log-out"
            }
        }

        Item {
            Layout.fillWidth: true
        }

        // Analog clock
        Item {
            id: clock
            Layout.fillHeight: true
            Layout.preferredWidth: height

            property int hours: 9
            property int minutes: 5

            PlasmaCore.Svg {
                id: clockSvg
                imagePath: "widgets/clock"
            }

            PlasmaCore.SvgItem {
                id: face
                anchors.centerIn: parent
                width: Math.min(parent.width, parent.height)
                height: Math.min(parent.width, parent.height)
                svg: clockSvg
                elementId: "ClockFace"
            }

            Hand {
                elementId: "HourHand"
                rotation: 180 + clock.hours * 30 + (clock.minutes/2)
                svgScale: face.width / face.naturalSize.width
            }

            Hand {
                elementId: "MinuteHand"
                rotation: 180 + clock.minutes * 6
                svgScale: face.width / face.naturalSize.width
            }

            PlasmaCore.SvgItem {
                id: center
                width: naturalSize.width * face.width / face.naturalSize.width
                height: naturalSize.height * face.width / face.naturalSize.width
                anchors.centerIn: clock
                svg: clockSvg
                elementId: "HandCenterScrew"
                z: 1000
            }

            PlasmaCore.SvgItem {
                anchors.fill: face
                svg: clockSvg
                elementId: "Glass"
                width: naturalSize.width * face.width / face.naturalSize.width
                height: naturalSize.height * face.width / face.naturalSize.width
            }
        }
    }

    Component.onCompleted: {
        kcm.applyPlasmaTheme(root, themeName);
    }
}
