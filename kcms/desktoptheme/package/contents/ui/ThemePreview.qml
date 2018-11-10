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
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: root
    property string themeName

    PlasmaCore.FrameSvgItem {
        id: background
        anchors.fill: parent
        // Air theme have huge transparent margins in widgets/background svg, so use
        // widgets/panel-background until Plasma::FrameSvg exposes the transparent margins
        imagePath: themeName == "air" ? "widgets/panel-background" : "widgets/background"
    }

    RowLayout {
        id: contents
        anchors {
            fill: parent
            topMargin: root.height / 8
            bottomMargin: root.height / 8
            leftMargin: root.width / 10
            rightMargin: root.width / 10
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
