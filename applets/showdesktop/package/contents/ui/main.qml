/*
    SPDX-FileCopyrightText: 2014 Ashish Madeti <ashishmadeti@gmail.com>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
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
    Plasmoid.toolTipSubText: i18n("Show the desktop by moving windows aside")
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

        Layout.minimumWidth: PlasmaCore.Units.iconSizes.small
        Layout.minimumHeight: Layout.minimumWidth

        Layout.maximumWidth: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1
        Layout.maximumHeight: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1

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
