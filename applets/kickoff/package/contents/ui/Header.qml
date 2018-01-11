/*
 *    Copyright 2014  Sebastian Kügler <sebas@kde.org>
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License along
 *    with this program; if not, write to the Free Software Foundation, Inc.,
 *    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kcoreaddons 1.0 as KCoreAddons
import org.kde.kquickcontrolsaddons 2.0

Item {
    id: header

    implicitHeight: units.gridUnit * 5

    property alias query: queryField.text
    property Item input: queryField

    KCoreAddons.KUser {
        id: kuser
    }

    state: (query !== "") ? "query" : "hint"

    Timer {
        id: labelTimer
        interval: 8000
        repeat: true
        running: (header.state === "hint" || header.state === "info") && plasmoid.expanded && (header.query === "")
        onTriggered: {
            if (header.state == "info") {
                header.state = "hint";
            } else if (header.state == "hint") {
                header.state = "info";
            }
        }
    }

    Image {
        id: faceIcon
        source: kuser.faceIconUrl
        cache: false
        visible: source !== ""

        width: units.gridUnit * 3
        height: width

        anchors {
            top: parent.top
            left: parent.left
            topMargin: units.gridUnit
            leftMargin: units.gridUnit
        }
    }

    PlasmaCore.IconItem {
        source: "user-identity"
        visible: faceIcon.status !== Image.Ready
        width: units.gridUnit * 3
        height: width
        anchors {
            top: faceIcon.top
            right: faceIcon.right
            rightMargin: -units.gridUnit/2
        }
        usesPlasmaTheme: false
    }

    MouseArea {
        anchors.fill: faceIcon
        acceptedButtons: Qt.LeftButton
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            KCMShell.open("user_manager")
        }
        visible: KCMShell.authorize("user_manager.desktop").length > 0
    }

    PlasmaExtras.Heading {
        id: nameLabel

        level: 2
        text: kuser.fullName
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignTop
        height: paintedHeight

        anchors {
            left: faceIcon.right
            top: faceIcon.top
            right: parent.right
            leftMargin: units.gridUnit
            rightMargin: units.gridUnit
        }
    }

    Item {
        id: searchWidget

        property int animationDuration: units.longDuration
        property real normalOpacity: .6
        property int yOffset: height / 2

        height: infoLabel.height
        anchors {
            left: nameLabel.left
            top: nameLabel.bottom
            right: nameLabel.right
        }

        PlasmaComponents.Label {
            id: infoLabel
            anchors {
                left: parent.left
                right: parent.right
            }
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignBottom
            text: kuser.os != "" ? i18n("%2@%3 (%1)", kuser.os, kuser.loginName, kuser.host) : i18n("%1@%2", kuser.loginName, kuser.host)
            elide: Text.ElideRight

            Behavior on opacity { NumberAnimation { duration:  searchWidget.animationDuration; easing.type: Easing.InOutQuad; } }
            Behavior on y { NumberAnimation { duration: searchWidget.animationDuration; easing.type: Easing.InOutQuad; } }
        }

        PlasmaComponents.Label {
            id: hintLabel
            anchors {
                left: parent.left
                right: parent.right
            }
            verticalAlignment: Text.AlignBottom
            text: i18nc("Type is a verb here, not a noun", "Type to search...")
            Behavior on opacity { NumberAnimation { duration: searchWidget.animationDuration; easing.type: Easing.InOutQuad; } }
            Behavior on y { NumberAnimation { duration: searchWidget.animationDuration; easing.type: Easing.InOutQuad; } }
        }


        MouseArea {
            anchors.fill: parent
            cursorShape: header.state === "hint" ? Qt.PointingHandCursor : Qt.ArrowCursor
            acceptedButtons: Qt.LeftButton
            enabled: header.state === "hint"
            onClicked: {
                root.previousState = "Normal"
                root.state = "Search"
                header.state = "query"
                queryField.forceActiveFocus()
            }

            PlasmaComponents.TextField {
                id: queryField
                anchors.fill: parent
                clearButtonShown: true
                visible: opacity > 0
                placeholderText: i18nc("Type is a verb here, not a noun", "Type to search...")
                Behavior on opacity { NumberAnimation { duration: searchWidget.animationDuration / 4 } }

                onTextChanged: {
                    if (root.state != "Search") {
                        root.previousState = root.state;
                        root.state = "Search";
                    }
                    if (text == "") {
                        root.state = root.previousState;
                        header.state = "info";
                    } else {
                        header.state = "query";
                    }
                }
            }
        }
    }

    states: [
        State {
            name: "info"
            PropertyChanges {
                target: infoLabel
                opacity: searchWidget.normalOpacity
                y: 0
            }
            PropertyChanges {
                target: hintLabel
                opacity: 0
                y: searchWidget.yOffset
            }
            PropertyChanges {
                target: queryField
                opacity: 0
            }
        },
        State {
            name: "hint"
            PropertyChanges {
                target: infoLabel
                y: -searchWidget.yOffset
                opacity: 0
            }
            PropertyChanges {
                target: hintLabel
                y: 0
                opacity: searchWidget.normalOpacity
            }
            PropertyChanges {
                target: queryField
                opacity: 0
            }
        },
        State {
            name: "query"
            PropertyChanges {
                target: infoLabel
                opacity: 0
            }
            PropertyChanges {
                target: hintLabel
                opacity: 0
            }
            PropertyChanges {
                target: queryField
                opacity: 1
            }
        }
    ] // states
}
