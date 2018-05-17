import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: clock
    property date time
    property bool showSecondsHand: false

    property int _hours
    property int _minutes
    property int _seconds

    onTimeChanged: {
        _hours = time.getHours();
        _minutes = time.getMinutes();
        _seconds = time.getSeconds();
    }

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
        anchors.topMargin: 3
        elementId: "HourHandShdow"
        rotation: 180 + _hours * 30 + (_minutes/2)
        svgScale: face.width / face.naturalSize.width

    }
    Hand {
        elementId: "HourHand"
        rotation: 180 + _hours * 30 + (_minutes/2)
        svgScale: face.width / face.naturalSize.width
    }

    Hand {
        anchors.topMargin: 3
        elementId: "MinuteHandShadow"
        rotation: 180 + _minutes * 6
        svgScale: face.width / face.naturalSize.width
    }
    Hand {
        elementId: "MinuteHand"
        rotation: 180 + _minutes * 6
        svgScale: face.width / face.naturalSize.width
    }

    Hand {
        anchors.topMargin: 3
        elementId: "SecondHandShadow"
        rotation: 180 + _seconds * 6
        visible: showSecondsHand
        svgScale: face.width / face.naturalSize.width
    }
    Hand {
        elementId: "SecondHand"
        rotation: 180 + _seconds * 6
        visible: showSecondsHand
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
