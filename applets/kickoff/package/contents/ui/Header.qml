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

import org.kde.plasma.private.kickoff 0.1 as Kickoff

Item {
    id: header

    implicitHeight: units.gridUnit * 5

    property alias query: queryField.text
    property Item input: queryField

    Kickoff.KUser {
        id: kuser
    }

    state: (header.query != "") ? "query" : ""

    Timer {
        id: labelTimer
        interval: 5000
        running: header.state === "hint" && plasmoid.expanded && (header.query === "")
        onTriggered: {
            if (header.state == "info") {
                header.state = "hint";
            } else if (header.state == "hint") {
                header.state = "info";
            }
        }
    }

    Rectangle { color: "orange"; visible: root.debug; anchors.fill: parent; opacity: 0.3 }

    Image {
        id: faceIcon
        source: kuser.faceIconPath
        visible: source != ""

        width: units.gridUnit * 3
        height: width

        anchors {
            top: parent.top
            left: parent.left
            topMargin: units.gridUnit
            leftMargin: units.gridUnit
        }

        Rectangle { color: "green"; opacity: 0.3; anchors.fill: parent;  visible: root.debug;  }
    }

    PlasmaCore.IconItem {
        source: "user-identity"
        visible: faceIcon.source == ""
        width: units.gridUnit * 3
        height: width
        anchors {
            top: faceIcon.top
            right: faceIcon.right
            rightMargin: -units.gridUnit/2
        }
    }

    PlasmaExtras.Heading {
        id: nameLabel

        level: 2
        text: kuser.fullName
        elide: Text.ElideRight
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

        height: queryField.height
        anchors {
            left: nameLabel.left
            top: nameLabel.bottom
            right: nameLabel.right
        }

        PlasmaComponents.TextField {
            id: queryField
            anchors.fill: parent
            clearButtonShown: true
            placeholderText: i18n("Type to search...")

            onTextChanged: {
                if (root.state != "Search") {
                    root.previousState = root.state;
                    root.state = "Search";
                }
                if (text == "") {
                    root.state = root.previousState;
                    root.forceActiveFocus();
                    header.state = "";
                } else {
                    header.state = "query";
                }
            }
        }

        Connections {
            target: plasmoid
            onExpandedChanged: {
                if (!expanded) {
                    header.query = "";
                }
            }
        }
    }
}
