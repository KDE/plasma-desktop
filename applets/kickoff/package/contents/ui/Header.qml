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

    state: "name"

    states: [
        State {
            name: "name"
            PropertyChanges {
                target: nameLabel
                opacity: 1
            }
            PropertyChanges {
                target: infoLabel
                opacity: 0
            }
        },
        State {
            name: "info"
            PropertyChanges {
                target: nameLabel
                opacity: 0
            }
            PropertyChanges {
                target: infoLabel
                opacity: 1
            }
        }
    ] // states


    Image {
        id: faceIcon
        source: kuser.faceIconUrl
        cache: false
        visible: source !== ""

        width: units.gridUnit * 3
        height: width

        sourceSize.width: width
        sourceSize.height: height

        fillMode: Image.PreserveAspectFit

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
        verticalAlignment: Text.AlignBottom

        Behavior on opacity { NumberAnimation { duration:  units.longDuration; easing.type: Easing.InOutQuad; } }

        anchors {
            left: faceIcon.right
            bottom: queryField.top
            right: parent.right
            leftMargin: units.gridUnit
            rightMargin: units.gridUnit
        }
    }

    PlasmaExtras.Heading {
        id: infoLabel

        level: 5
        opacity: 0
        text: kuser.os !== "" ? i18n("%2@%3 (%1)", kuser.os, kuser.loginName, kuser.host) : i18n("%1@%2", kuser.loginName, kuser.host)
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignBottom

        Behavior on opacity { NumberAnimation { duration:  units.longDuration; easing.type: Easing.InOutQuad; } }

        anchors {
            left: faceIcon.right
            bottom: queryField.top
            right: parent.right
            leftMargin: units.gridUnit
            rightMargin: units.gridUnit
        }
    }

    // Show the info instead of the user's name when hovering over it
    MouseArea {
        anchors.fill: nameLabel
        hoverEnabled: true
        onEntered: {
            header.state = "info"
        }
        onExited: {
            header.state = "name"
        }
    }

    PlasmaComponents.TextField {
        id: queryField

        placeholderText: i18n("Search...")
        clearButtonShown: true

        onTextChanged: {
            if (root.state != "Search") {
                root.previousState = root.state;
                root.state = "Search";
            }
            if (text == "") {
                root.state = root.previousState;
            }
        }

        anchors {
            left: faceIcon.right
            bottom: faceIcon.bottom
            right: parent.right
            leftMargin: units.gridUnit
            rightMargin: units.gridUnit
        }
    }
}
