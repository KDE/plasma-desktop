/*
    Copyright 2020  Devin Lin <espidev@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library.  If not, see <http://www.gnu.org/licenses/>.
*/

import QtQuick 2.12
import QtQuick.Layouts 1.3
import QtQuick.Shapes 1.12

import org.kde.kirigami 2.12 as Kirigami

Item {
    width: progressCircle.width
    height: progressCircle.height
    Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
    
    property alias colorTimer: colorChangeBackTimer
    
    Timer {
        id: colorChangeBackTimer
        interval: 500
        onTriggered: {
            iconColorAnimation.to = Kirigami.Theme.textColor
            iconColorAnimation.start();
            circleColorAnimation.to = Kirigami.Theme.highlightColor
            circleColorAnimation.start();
        }
    }
    
    Connections {
        target: fingerprintModel
        function onScanSuccess() {
            iconColorAnimation.to = Kirigami.Theme.highlightColor
            iconColorAnimation.start();
            colorChangeBackTimer.restart();
        }
        function onScanFailure() {
            iconColorAnimation.to = Kirigami.Theme.negativeTextColor
            iconColorAnimation.start();
            colorChangeBackTimer.restart();
        }
        function onScanComplete() {
            iconColorAnimation.to = Kirigami.Theme.positiveTextColor
            iconColorAnimation.start();
        }
    }
    
    Kirigami.Icon {
        id: fingerprintEnrollFeedback
        source: "fingerprint"
        implicitHeight: Kirigami.Units.iconSizes.huge
        implicitWidth: implicitHeight
        anchors.centerIn: parent
    
        ColorAnimation on color {
            id: iconColorAnimation
            easing.type: Easing.InOutQuad
            duration: 150
        }
    }
    
    Shape {
        id: progressCircle
        anchors.horizontalCenter: fingerprintEnrollFeedback.horizontalCenter
        anchors.verticalCenter: fingerprintEnrollFeedback.verticalCenter
        implicitWidth: Kirigami.Units.iconSizes.huge + Kirigami.Units.gridUnit
        implicitHeight: Kirigami.Units.iconSizes.huge + Kirigami.Units.gridUnit
        layer.enabled: true
        layer.samples: 40
        anchors.centerIn: parent
        
        property int rawAngle: fingerprintModel.enrollProgress * 360
        property int renderedAngle: 0
        NumberAnimation on renderedAngle {
            id: elapsedAngleAnimation
            easing.type: Easing.InOutQuad
            duration: 500
        }
        onRawAngleChanged: {
            elapsedAngleAnimation.to = rawAngle;
            elapsedAngleAnimation.start();
        }
        
        ShapePath {
            strokeColor: "lightgrey"
            fillColor: "transparent"
            strokeWidth: 3
            capStyle: ShapePath.FlatCap
            PathAngleArc {
                centerX: progressCircle.implicitWidth / 2; centerY: progressCircle.implicitHeight / 2;
                radiusX: (progressCircle.implicitWidth - Kirigami.Units.gridUnit) / 2; radiusY: radiusX;
                startAngle: 0
                sweepAngle: 360
            }
        }
        ShapePath {
            strokeColor: Kirigami.Theme.highlightColor
            fillColor: "transparent"
            strokeWidth: 3
            capStyle: ShapePath.RoundCap
            
            ColorAnimation on strokeColor {
                id: circleColorAnimation
                easing.type: Easing.InOutQuad
                duration: 200
            }
            
            PathAngleArc {
                centerX: progressCircle.implicitWidth / 2; centerY: progressCircle.implicitHeight / 2;
                radiusX: (progressCircle.implicitWidth - Kirigami.Units.gridUnit) / 2; radiusY: radiusX;
                startAngle: -90
                sweepAngle: progressCircle.renderedAngle
            }
        }
    }
}
