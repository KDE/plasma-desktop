/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.draganddrop 2.0 as DragDrop

DragDrop.DropArea {
    readonly property bool inPanel: [
        PlasmaCore.Types.TopEdge,
        PlasmaCore.Types.LeftEdge,
        PlasmaCore.Types.RightEdge,
        PlasmaCore.Types.BottomEdge,
    ].includes(plasmoid.location)

    Layout.minimumWidth: plasmoid.formFactor === PlasmaCore.Types.Horizontal ? height : PlasmaCore.Units.iconSizes.small
    Layout.minimumHeight: plasmoid.formFactor === PlasmaCore.Types.Vertical ? width : (PlasmaCore.Units.iconSizes.small + 2 * PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height)

    Layout.maximumWidth: inPanel && plasmoid.formFactor !== PlasmaCore.Types.Vertical ? PlasmaCore.Units.iconSizeHints.panel : -1
    Layout.maximumHeight: inPanel && plasmoid.formFactor !== PlasmaCore.Types.Vertical ? PlasmaCore.Units.iconSizeHints.panel : -1

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

    MouseArea {
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
