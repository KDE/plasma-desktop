/*
 *    Copyright 2014  Sebastian Kügler <sebas@kde.org>
 *    SPDX-FileCopyrightText: (C) 2020 Carl Schwan <carl@carlschwan.eu>
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

import QtQuick 2.12
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kcoreaddons 1.0 as KCoreAddons
import org.kde.kquickcontrolsaddons 2.0
import QtGraphicalEffects 1.0

PlasmaExtras.PlasmoidHeading {
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

    RowLayout {
        anchors.fill: parent

        Image {
            id: faceIcon
            source: kuser.faceIconUrl
            cache: false
            visible: source !== ""

            Layout.preferredWidth: units.gridUnit * 3
            Layout.preferredHeight: units.gridUnit * 3

            sourceSize.width: width
            sourceSize.height: height

            fillMode: Image.PreserveAspectFit
            Layout.margins: units.gridUnit

            // Crop the avatar to fit in a circle, like the lock and login screens
            // but don't on software rendering where this won't render
            layer.enabled: faceIcon.GraphicsInfo.api !== GraphicsInfo.Software
            layer.effect: OpacityMask {
                // this Rectangle is a circle due to radius size
                maskSource: Rectangle {
                    width: faceIcon.width
                    height: faceIcon.height
                    radius: height / 2
                    visible: false
                }
            }
            // Border for the circular avatar
            Rectangle {
                anchors.fill: faceIcon
                // Don't show the circle if we're using software rendering, because
                // the cropping effect won't be displayed under it
                visible: GraphicsInfo.api !== GraphicsInfo.Software

                radius: height / 2
                border.width: 1
                border.color: theme.textColor
                opacity: 0.4
                color: "transparent"
            }
            MouseArea {
                anchors.fill: faceIcon
                acceptedButtons: Qt.LeftButton
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    KCMShell.openSystemSettings("kcm_users")
                }
                visible: KCMShell.authorize("kcm_users.desktop").length > 0
            }
            PlasmaCore.IconItem {
                source: "user-identity"
                visible: faceIcon.status !== Image.Ready
                anchors.fill: faceIcon
                anchors.bottomMargin: units.smallSpacing
                usesPlasmaTheme: false
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.rightMargin: units.gridUnit

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: units.gridUnit

                PlasmaExtras.Heading {
                    id: nameLabel
                    anchors.fill: parent

                    level: 2
                    text: kuser.fullName
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignBottom

                    Behavior on opacity { NumberAnimation { duration:  units.longDuration; easing.type: Easing.InOutQuad; } }

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
                }

                PlasmaExtras.Heading {
                    id: infoLabel
                    anchors.fill: parent
                    level: 5
                    opacity: 0
                    text: kuser.os !== "" ? i18n("%2@%3 (%1)", kuser.os, kuser.loginName, kuser.host) : i18n("%1@%2", kuser.loginName, kuser.host)
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignBottom

                    Behavior on opacity { NumberAnimation { duration:  units.longDuration; easing.type: Easing.InOutQuad; } }
                }
            }

            PlasmaComponents.TextField {
                id: queryField
                Layout.fillWidth: true

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
            }
        }
    }
}
