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
// While using Kirigami in applets is normally a no, we
// use Avatar, which doesn't need to read the colour scheme
// at all to function, so there won't be any oddities with colours.
import org.kde.kirigami 2.13 as Kirigami
import org.kde.kquickcontrolsaddons 2.0
import QtGraphicalEffects 1.0

PlasmaExtras.PlasmoidHeading {
    id: header

    implicitHeight: PlasmaCore.Units.gridUnit * 5

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

        PlasmaComponents.RoundButton {
            visible: KCMShell.authorize("kcm_users.desktop").length > 0

            flat: true

            Layout.preferredWidth: PlasmaCore.Units.gridUnit * 3
            Layout.preferredHeight: PlasmaCore.Units.gridUnit * 3

            Kirigami.Avatar {
                source: kuser.faceIconUrl
                name: nameLabel
                anchors {
                    fill: parent
                    margins: PlasmaCore.Units.smallSpacing
                }
            }

            onClicked: {
                KCMShell.openSystemSettings("kcm_users")
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.rightMargin: PlasmaCore.Units.gridUnit

            Item {
                Layout.fillWidth: true
                Layout.preferredHeight: PlasmaCore.Units.gridUnit

                PlasmaExtras.Heading {
                    id: nameLabel
                    anchors.fill: parent

                    level: 2
                    text: kuser.fullName
                    elide: Text.ElideRight
                    horizontalAlignment: Text.AlignLeft
                    verticalAlignment: Text.AlignBottom

                    Behavior on opacity { NumberAnimation { duration:  PlasmaCore.Units.longDuration; easing.type: Easing.InOutQuad; } }

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

                    Behavior on opacity { NumberAnimation { duration:  PlasmaCore.Units.longDuration; easing.type: Easing.InOutQuad; } }
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
