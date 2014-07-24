
import QtQuick 2.2

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore

Item {

    width: 600
    height: 400

    Rectangle {
        anchors {
            fill: parent
        }

        color: "darkgrey"
    }

    PlasmaCore.FrameSvgItem {
        id: frame
        anchors.fill : parent
        imagePath: "widgets/background"

        PlasmaExtras.AppearAnimation {
            id: appear
            targetItem: frame
            duration: 500
            running: false
        }
        PlasmaExtras.DisappearAnimation {
            id: disappear
            targetItem: frame
            duration: 500
            running: false
        }

        Column {
            anchors.fill: parent
            anchors.margins: units.gridUnit
            spacing: units.gridUnit / 2

            PlasmaExtras.Title {
                width: parent.width
                elide: Text.ElideRight
                text: "Icons"
            }
            PlasmaComponents.Label {
                text: "iconSizes.small  : " + units.iconSizes.small +
                        ", iconSizes.desktop: " + units.iconSizes.desktop +
                        ",<br />iconSizes.toolbar: " + units.iconSizes.toolbar +
                        ", iconSizes.dialog : " + units.iconSizes.dialog

            }

            Flow {
                //height: units.gridUnit
                //anchors.fill: parent
                //anchors.margins: units.gridUnit
                //width: parent.width
                spacing: units.gridUnit

                PlasmaCore.IconItem {
                    source: "configure"
                    width: units.gridUnit * 4
                    height: width
                }
                PlasmaCore.IconItem {
                    source: "dialog-ok"
                    width: units.gridUnit * 4
                    height: width
                }
                PlasmaCore.IconItem {
                    source: "folder-green"
                    width: units.gridUnit * 4
                    height: width
                }
                PlasmaCore.IconItem {
                    source: "akonadi"
                    width: units.gridUnit * 4
                    height: width
                }
                PlasmaCore.IconItem {
                    source: "clock"
                    width: units.gridUnit * 4
                    height: width
                }
        //         KQuickControlsAddons.QIconItem {
        //             icon: "preferences-desktop-icons"
        //             width: units.gridUnit * 4
        //             height: width
        //         }
            }

        }
    }

    PlasmaComponents.Button {
        id: button
        anchors {
            bottom: parent.bottom
            right: parent.right
            margins: units.gridUnit
        }
        text: hit ? "Show" : "Hide"
        property bool hit: false
        onClicked: {
            print("you hit me!");
            if (hit) {
                appear.running = true;
                hit = false;
            } else {
                disappear.running = true;
                hit = true;
            }

        }
    }
    Component.onCompleted: print("\n\n_______________ Done. :)\n\n")

}