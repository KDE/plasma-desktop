/*
    Copyright (C) 2014 Ashish Madeti <ashishmadeti@gmail.com>
    Copyright (C) 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.private.showdesktop 0.1

QtObject {
    id: root

    // you can't have an applet with just a compact representation :(
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.onActivated: showdesktop.showingDesktop = !showdesktop.showingDesktop
    Plasmoid.icon: plasmoid.configuration.icon
    Plasmoid.title: i18n("Show Desktop")
    Plasmoid.toolTipSubText: i18n("Show the Plasma desktop")
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    // QtObject has no default property
    property QtObject showdesktop: ShowDesktop { }

    Component.onCompleted: {
        plasmoid.setAction("minimizeall", i18nc("@action", "Minimize All Windows"))
    }

    function action_minimizeall() {
        showdesktop.minimizeAll()
    }

    Plasmoid.fullRepresentation: PlasmaCore.ToolTipArea {
        readonly property bool inPanel: (plasmoid.location === PlasmaCore.Types.TopEdge
            || plasmoid.location === PlasmaCore.Types.RightEdge
            || plasmoid.location === PlasmaCore.Types.BottomEdge
            || plasmoid.location === PlasmaCore.Types.LeftEdge)

        Layout.minimumWidth: units.iconSizes.small
        Layout.minimumHeight: Layout.minimumWidth

        Layout.maximumWidth: inPanel ? units.iconSizeHints.panel : -1
        Layout.maximumHeight: inPanel ? units.iconSizeHints.panel : -1

        mainText: plasmoid.title
        subText: plasmoid.toolTipSubText

        MouseArea {
            anchors.fill: parent
            onClicked: showdesktop.showingDesktop = !showdesktop.showingDesktop
        }

        PlasmaCore.IconItem {
            anchors.fill: parent
            source: plasmoid.icon
            active: parent.containsMouse || showdesktop.showingDesktop
        }

        // also activate when dragging an item over the plasmoid so a user can easily drag data to the desktop
        DropArea {
            anchors.fill: parent
            onEntered: activateTimer.start()
            onExited: activateTimer.stop()

            Timer {
                id: activateTimer
                interval: 250 //to match TaskManager
                onTriggered: plasmoid.activated()
            }
        }
    }

}
