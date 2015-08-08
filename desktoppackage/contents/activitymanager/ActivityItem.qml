import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddonsComponents
import org.kde.plasma.activityswitcher 1.0 as ActivitySwitcher
import org.kde.activities 0.1 as Activities

import "static.js" as S

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

            width   : units.iconSizes.medium
            height  : width
            visible : shade.visible

            anchors {
                bottom  : shade.bottom
                left    : shade.left
                margins : root.innerPadding
            }
        }

        PlasmaComponents.Label {
            id: title

            color   : "white"
            elide   : Text.ElideRight
            visible : shade.visible

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

            onClicked: activitiesModel.stopActivity(activityId, function () {});

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

            onClicked: S.openActivityConfigurationDialog(
                            configureDialog,
                            root.activityId,
                            root.title,
                            root.icon,
                            // We need to pass some QML-only variables
                            {
                                kactivities: activitiesModel,
                                readyStatus: Loader.Ready,
                                i18nd:       i18nd
                            }
                        )

            icon: "configure"

            anchors {
                left    : parent.left
                top     : parent.top
                margins : root.innerPadding
            }
        }

        Loader {
            id: configureDialog

            anchors.fill: parent

            property bool itemVisible: status == Loader.Ready && item.visible
        }


        states: [
            State {
                name: "plain"
                PropertyChanges { target: stopButton   ; opacity: 0.2 }
                PropertyChanges { target: configButton ; opacity: 0.2 }
                PropertyChanges { target: shade        ; visible: true }
            },
            State {
                name: "showingControls"
                PropertyChanges { target: stopButton   ; opacity: 1 }
                PropertyChanges { target: configButton ; opacity: 1 }
                PropertyChanges { target: shade        ; visible: true }
            },
            State {
                name: "showingOverlayDialog"
                PropertyChanges { target: stopButton   ; opacity: 0 }
                PropertyChanges { target: configButton ; opacity: 0 }
                PropertyChanges { target: shade        ; visible: false }
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

        state:
            configureDialog.itemVisible ? "showingOverlayDialog"
            : rootArea.containsMouse    ? "showingControls"
            : /* otherwise */             "plain"
    }
}
