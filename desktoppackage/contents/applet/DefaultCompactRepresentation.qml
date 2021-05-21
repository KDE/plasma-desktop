/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.IconItem {
    id: icon

    readonly property bool inPanel: (plasmoid.location === PlasmaCore.Types.TopEdge
        || plasmoid.location === PlasmaCore.Types.RightEdge
        || plasmoid.location === PlasmaCore.Types.BottomEdge
        || plasmoid.location === PlasmaCore.Types.LeftEdge)

    Layout.minimumWidth: {
        switch (plasmoid.formFactor) {
        case PlasmaCore.Types.Vertical:
            return 0;
        case PlasmaCore.Types.Horizontal:
            return height;
        default:
            return PlasmaCore.Units.gridUnit * 3;
        }
    }

    Layout.minimumHeight: {
        switch (plasmoid.formFactor) {
        case PlasmaCore.Types.Vertical:
            return width;
        case PlasmaCore.Types.Horizontal:
            return 0;
        default:
            return PlasmaCore.Units.gridUnit * 3;
        }
    }

    Layout.maximumWidth: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1;
    Layout.maximumHeight: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1;

    source: plasmoid.icon ? plasmoid.icon : "plasma"
    active: mouseArea.containsMouse

    MouseArea {
        id: mouseArea

        property bool wasExpanded: false

        anchors.fill: parent
        hoverEnabled: true
        onPressed: wasExpanded = plasmoid.expanded
        onClicked: plasmoid.expanded = !wasExpanded
    }
}
