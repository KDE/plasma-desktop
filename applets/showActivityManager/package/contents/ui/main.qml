/*
 *   Copyright 2012 Gregor Taetzner <gregor@freenet.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore

MouseArea {
    id: iconContainer
    property string activeSource: "Status"
    height: units.iconSizes.large
    width: units.iconSizes.large

    readonly property bool inPanel: (plasmoid.location == PlasmaCore.Types.TopEdge
        || plasmoid.location == PlasmaCore.Types.RightEdge
        || plasmoid.location == PlasmaCore.Types.BottomEdge
        || plasmoid.location == PlasmaCore.Types.LeftEdge)

    Layout.maximumWidth: inPanel ? units.iconSizeHints.panel : -1
    Layout.maximumHeight: inPanel ? units.iconSizeHints.panel : -1

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation

    onClicked: {
        var service = dataSource.serviceForSource(activeSource)
        var operation = service.operationDescription("toggleActivityManager")
        service.startOperationCall(operation)
    }

    PlasmaCore.DataSource {
        id: dataSource
        engine: "org.kde.activities"
        connectedSources: [activeSource]
    }

    PlasmaCore.ToolTipArea {
        id: tooltip
        mainText: i18n("Show Activity Manager")
        subText: i18n("Click to show the activity manager")
        anchors.fill: parent
        icon: "preferences-activities"
    }

    PlasmaCore.IconItem {
        id: icon
        source: "preferences-activities"
        width: parent.width
        height: parent.height
    }
}

