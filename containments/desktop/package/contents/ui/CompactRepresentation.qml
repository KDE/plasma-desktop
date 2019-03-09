/***************************************************************************
 *   Copyright (C) 2013-2014 by Eike Hein <hein@kde.org>                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.draganddrop 2.0 as DragDrop

DragDrop.DropArea {
    readonly property bool inPanel: (plasmoid.location === PlasmaCore.Types.TopEdge
        || plasmoid.location === PlasmaCore.Types.RightEdge
        || plasmoid.location === PlasmaCore.Types.BottomEdge
        || plasmoid.location === PlasmaCore.Types.LeftEdge)

    Layout.minimumWidth: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? height : units.iconSizes.small
    Layout.minimumHeight: plasmoid.formFactor === PlasmaCore.Types.Vertical ? width : (units.iconSizes.small + 2 * theme.mSize(theme.defaultFont).height)

    Layout.maximumWidth: inPanel && plasmoid.formFactor !== PlasmaCore.Types.Vertical ? units.iconSizeHints.panel : -1
    Layout.maximumHeight: inPanel && plasmoid.formFactor !== PlasmaCore.Types.Vertical ? units.iconSizeHints.panel : -1

    property Item folderView: null

    onContainsDragChanged: {
        if (containsDrag) {
            hoverActivateTimer.restart();
        } else {
            hoverActivateTimer.stop();
        }
    }

    onDrop: folderView.model.dropCwd(event)
    preventStealing: true

    function toggle() {
        plasmoid.expanded = !plasmoid.expanded;
    }

    PlasmaCore.IconItem {
        id: icon

        anchors.fill: parent

        active: mouseArea.containsMouse

        source: plasmoid.configuration.useCustomIcon ? plasmoid.configuration.icon : folderView.model.iconName
    }

    MouseArea
    {
        id: mouseArea

        anchors.fill: parent

        hoverEnabled: true

        onClicked: toggle()
    }

    Timer {
        id: hoverActivateTimer

        interval: root.hoverActivateDelay

        onTriggered: toggle()
    }
}
