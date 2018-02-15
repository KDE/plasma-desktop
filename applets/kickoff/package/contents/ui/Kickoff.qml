/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2012  Gregor Taetzner <gregor@freenet.de>
    Copyright (C) 2012  Marco Martin <mart@kde.org>
    Copyright (C) 2013  David Edmundson <davidedmundson@kde.org>
    Copyright (C) 2015  Eike Hein <hein@kde.org>

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
import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0

import org.kde.plasma.private.kicker 0.1 as Kicker

Item {
    id: kickoff

    readonly property bool inPanel: (plasmoid.location == PlasmaCore.Types.TopEdge
        || plasmoid.location == PlasmaCore.Types.RightEdge
        || plasmoid.location == PlasmaCore.Types.BottomEdge
        || plasmoid.location == PlasmaCore.Types.LeftEdge)

    Plasmoid.switchWidth: units.gridUnit * 20
    Plasmoid.switchHeight: units.gridUnit * 30

    Plasmoid.fullRepresentation: FullRepresentation {}

    Plasmoid.icon: plasmoid.configuration.icon

    Plasmoid.compactRepresentation: MouseArea {
        //AppletQuickItem expects a layout to exist
        //setting even a default property forces it to be created
        Layout.fillWidth: false
        Layout.maximumWidth: inPanel ? units.iconSizeHints.panel : -1
        Layout.maximumHeight: inPanel ? units.iconSizeHints.panel : -1
        hoverEnabled: true
        onClicked: plasmoid.expanded = !plasmoid.expanded

        DropArea {
            id: compactDragArea
            anchors.fill: parent
        }

        Timer {
            id: expandOnDragTimer
            interval: 250
            running: compactDragArea.containsDrag
            onTriggered: plasmoid.expanded = true
        }

        PlasmaCore.IconItem {
            anchors.fill: parent
            source: plasmoid.icon
            active: parent.containsMouse || compactDragArea.containsDrag
            smooth: true
        }
    }

    property Item dragSource: null

    Kicker.ProcessRunner {
        id: processRunner;
    }

    function action_menuedit() {
        processRunner.runMenuEditor();
    }

    Component.onCompleted: {
        if (plasmoid.hasOwnProperty("activationTogglesExpanded")) {
            plasmoid.activationTogglesExpanded = true
        }
        if (plasmoid.immutability !== PlasmaCore.Types.SystemImmutable) {
            plasmoid.setAction("menuedit", i18n("Edit Applications..."));
        }
    }
} // root
