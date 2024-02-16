/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.20 as Kirigami

Kirigami.Icon {
    property PlasmoidItem plasmoidItem
    readonly property bool inPanel: [PlasmaCore.Types.TopEdge, PlasmaCore.Types.RightEdge, PlasmaCore.Types.BottomEdge, PlasmaCore.Types.LeftEdge]
        .includes(Plasmoid.location)

    Layout.minimumWidth: {
        switch (Plasmoid.formFactor) {
        case PlasmaCore.Types.Vertical:
            return 0;
        case PlasmaCore.Types.Horizontal:
            return height;
        default:
            return Kirigami.Units.gridUnit * 3;
        }
    }

    Layout.minimumHeight: {
        switch (Plasmoid.formFactor) {
        case PlasmaCore.Types.Vertical:
            return width;
        case PlasmaCore.Types.Horizontal:
            return 0;
        default:
            return Kirigami.Units.gridUnit * 3;
        }
    }

    source: Plasmoid.icon || "plasma"
    active: mouseArea.containsMouse

    activeFocusOnTab: true

    Keys.onPressed: event => {
        switch (event.key) {
        case Qt.Key_Space:
        case Qt.Key_Enter:
        case Qt.Key_Return:
        case Qt.Key_Select:
            Plasmoid.activated();
            event.accepted = true; // BUG 481393: Prevent system tray from receiving the event
            break;
        }
    }

    Accessible.name: Plasmoid.title
    Accessible.description: plasmoidItem.toolTipSubText ?? ""
    Accessible.role: Accessible.Button

    MouseArea {
        id: mouseArea

        property bool wasExpanded: false

        anchors.fill: parent
        hoverEnabled: true
        onPressed: wasExpanded = plasmoidItem.expanded
        onClicked: plasmoidItem.expanded = !wasExpanded
    }
}
