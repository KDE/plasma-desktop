/*
    SPDX-FileCopyrightText: 2013-2014 Eike Hein <hein@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.draganddrop 2.0 as DragDrop
import org.kde.kirigami 2.20 as Kirigami

DragDrop.DropArea {
    readonly property bool inPanel: [
        PlasmaCore.Types.TopEdge,
        PlasmaCore.Types.LeftEdge,
        PlasmaCore.Types.RightEdge,
        PlasmaCore.Types.BottomEdge,
    ].includes(Plasmoid.location)

    Layout.minimumWidth: Plasmoid.formFactor === PlasmaCore.Types.Horizontal ? height : Kirigami.Units.iconSizes.small
    Layout.minimumHeight: Plasmoid.formFactor === PlasmaCore.Types.Vertical ? width : (Kirigami.Units.iconSizes.small + 2 * Kirigami.Units.iconSizes.sizeForLabels)

    property Item folderView: null

    onContainsDragChanged: contained => {
        if (containsDrag) {
            hoverActivateTimer.restart();
        } else {
            hoverActivateTimer.stop();
        }
    }

    onDrop: event => {
        folderView.model.dropCwd(event);
    }

    preventStealing: true

    function toggle() {
        root.expanded = !root.expanded;
    }

    Kirigami.Icon {
        id: icon

        anchors.fill: parent

        active: mouseArea.containsMouse

        source: Plasmoid.configuration.useCustomIcon ? Plasmoid.configuration.icon : folderView.model.iconName
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent

        hoverEnabled: true

        onClicked: mouse => toggle()
    }

    Timer {
        id: hoverActivateTimer

        interval: root.hoverActivateDelay

        onTriggered: toggle()
    }
}
