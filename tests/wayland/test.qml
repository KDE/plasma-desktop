
import QtQuick 2.2

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore

Item {

    width: 600
    height: 400

    Rectangle {
        anchors {
            fill: parent
        }

        color: "darkblue"
    }

    PlasmaCore.FrameSvgItem {
        anchors.fill : parent
        imagePath: "widgets/background"
    }

    PlasmaComponents.Button {
        id: button
        anchors.centerIn: parent
        text: "Hit me!"
        onClicked: print("you hit me!")
    }

    Flow {
        //height: units.gridUnit
        anchors.fill: parent
        anchors.margins: units.gridUnit
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
    Component.onCompleted: print("\n\n_______________ Done. :)\n\n")

}