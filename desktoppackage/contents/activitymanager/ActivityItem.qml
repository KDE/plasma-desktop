import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: root

    property int innerPadding  : units.largeSpacing

    property bool current      : false
    property bool inFocus      : false

    property alias title       : title.text
    property alias icon        : icon.source

    property string background : ""

    onBackgroundChanged: if (background[0] != '#') {
        // We have a proper wallpaper, hurroo!
        backgroundWallpaper.source = background
        backgroundColor.visible = false

    } else {
        // We have only a color
        backgroundColor.color = background
        backgroundColor.visible = true
    }

    signal clicked
    signal configureClicked
    signal stopClicked

    width  : 200
    height : width * 1 / units.displayAspectRatio

    // Background until we get something real
    PlasmaCore.FrameSvgItem {
        id: highlight
        imagePath: "widgets/viewitem"
        visible: root.current || root.inFocus

        anchors {
            fill: parent
            rightMargin: -highlight.margins.right
            bottomMargin: -highlight.margins.bottom
        }

        prefix:
            root.current ? "selected" : "normal"

    }

    Item {
        anchors {
            fill: parent

            leftMargin: highlight.margins.left
            topMargin: highlight.margins.top
        }

        // Background until we get something real
        Rectangle {
            id: backgroundColor
            anchors.fill: parent
            color: "black"

            opacity: .5
        }

        Image {
            id: backgroundWallpaper
            anchors.fill: parent

            visible: !backgroundColor.visible
        }

        // Title and the icon

        Rectangle {
            id: shade

            color   : "black"
            opacity : .5

            height  : title.height + 2 * title.anchors.margins
            width   : parent.width

            anchors {
                bottom : parent.bottom
                left   : parent.left
                right  : parent.right
            }
        }

        PlasmaCore.IconItem {
            id: icon

            width  : units.iconSizes.medium
            height : width

            anchors {
                bottom  : parent.bottom
                left    : parent.left
                margins : root.innerPadding
            }
        }

        PlasmaComponents.Label {
            id: title

            color : "white"
            elide : Text.ElideRight

            anchors {
                bottom  : parent.bottom
                left    : icon.right
                right   : parent.right
                margins : root.innerPadding * 2
            }
        }

        // Controls

        MouseArea {
            id: rootArea

            anchors.fill : parent
            hoverEnabled : true

            onClicked    : root.clicked()
        }

        ControlButton {
            id: stopButton

            onClicked: root.stopClicked()

            icon: "close"

            anchors {
                right   : parent.right
                top     : parent.top
                margins : root.innerPadding
            }
        }

        ControlButton {
            id: configButton

            onClicked: root.configureClicked()

            icon: "configure"

            anchors {
                left    : parent.left
                top     : parent.top
                margins : root.innerPadding
            }
        }

        states: [
            State {
                name: "plain"
                PropertyChanges { target: stopButton; opacity: 0 }
                PropertyChanges { target: configButton; opacity: 0 }
            },
            State {
                name: "showControls"
                PropertyChanges { target: stopButton; opacity: 1 }
                PropertyChanges { target: configButton; opacity: 1 }
            }
        ]

        transitions: [
            Transition {
                NumberAnimation {
                    properties : "opacity"
                    duration   : units.shortDuration
                }
            }
        ]

        state: rootArea.containsMouse ? "showControls" : "plain"
    }
}
