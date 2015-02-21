import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddonsComponents
import org.kde.plasma.activityswitcher 1.0 as ActivitySwitcher

Item {
    id: root

    property int innerPadding  : units.largeSpacing

    property bool current      : false
    property bool selected     : false
    property bool stoppable    : true

    property alias title       : title.text
    property alias icon        : icon.source

    z : current  ? 10 :
        selected ?  5 : 0

    property string activityId : ""

    property string background : ""

    function updateBackground(valid) {
        if (valid) {
            console.log("Switcher: QML Getting the wallpaperThumbnail");

            // Try to get the pixmap, if it is not available, this function
            // will be called again when the thumbnailer finishes its job
            // console.log("Loading background for " + root.title);
            backgroundWallpaper.pixmap = ActivitySwitcher.Backend.wallpaperThumbnail(
                background,
                backgroundWallpaper.width,
                backgroundWallpaper.height,
                updateBackground
                );
            backgroundColor.visible = false;

        } else {
            backgroundColor.color = "#000000";
            backgroundColor.visible = true;

        }
    }

    onBackgroundChanged: if (background[0] != '#') {
        // We have a proper wallpaper, hurroo!
        updateBackground(true);

    } else {
        // We have only a color
        backgroundColor.color = background
        backgroundColor.visible = true
    }

    signal clicked
    signal configureClicked
    signal stopClicked

    width  : 200
    // height : width * 1 / units.displayAspectRatio
    // Marco removed displayAspectRatio
    height : width * 9.0 / 16.0

    onWidthChanged: updateBackground(true)
    onHeightChanged: updateBackground(true)


    Item {
        anchors {
            fill: parent
        }

        // Background until we get something real
        Rectangle {
            id: backgroundColor
            anchors.fill: parent
            color: "black"

            opacity: root.selected ? .8 : .5
        }

        KQuickControlsAddonsComponents.QPixmapItem {
            id: backgroundWallpaper
            anchors.fill: parent

            visible: !backgroundColor.visible
        }

        // Title and the icon

        Rectangle {
            id: shade

            color   : "black"
            opacity : root.selected ? 1.0 : 0.5

            height  : title.height + 2 * title.anchors.margins
            width   : parent.width

            anchors {
                bottom : parent.bottom
                left   : parent.left
                right  : parent.right
            }
        }

        Rectangle {
            id: highlight

            // opacity: root.current ? 1.0 : 0
            visible:  root.current

            border.width: root.current ? units.smallSpacing * 1.5 : 0
            border.color: theme.highlightColor

            anchors {
                fill: parent
                // Hide the rounding error on the bottom of the rectangle
                bottomMargin: -1
            }

            color: "transparent"

            // Behavior on opacity {
            //     PropertyAnimation {
            //         duration: units.shortDuration
            //         easing.type: Easing.OutQuad
            //     }
            // }
        }

        PlasmaCore.IconItem {
            id: icon

            width  : units.iconSizes.medium
            height : width

            anchors {
                bottom  : shade.bottom
                left    : shade.left
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
            visible: root.stoppable

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
