/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.plasma.workspace.components as WorkspaceComponents
import org.kde.kirigami as Kirigami

Kirigami.Icon {
    id: defaultCompactRepresentation
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
        onClicked: mouse => {
            if (mouse.button === Qt.MiddleButton) {
                Plasmoid.secondaryActivated();
            } else {
                defaultCompactRepresentation.plasmoidItem.expanded = !wasExpanded;
            }
        }
    }

    // Open the FullRepresentation on drag-hover if the applet wants it
    Loader {
        anchors.fill: parent

        active: defaultCompactRepresentation.plasmoidItem.expandedOnDragHover

        sourceComponent: DropArea {
            anchors.fill: parent

            onEntered: dropTimer.restart()
            onExited: dropTimer.stop()

            Timer {
                id: dropTimer
                interval: 250 // matches taskmanager delay
                onTriggered: {
                    defaultCompactRepresentation.plasmoidItem.expanded = true;
                    mouseArea.wasExpanded = true;
                }
            }
        }
    }

    Loader {
        id: badgeLoader

        anchors.bottom: defaultCompactRepresentation.bottom
        anchors.right: defaultCompactRepresentation.right

        active: defaultCompactRepresentation.plasmoidItem.badgeText.length != 0

        sourceComponent: WorkspaceComponents.BadgeOverlay {
            text: defaultCompactRepresentation.plasmoidItem.badgeText
            icon: defaultCompactRepresentation
        }

        // Non-default state to center if the badge is wider than the icon
        states: [
            State {
                when: badgeLoader.width >= defaultCompactRepresentation.width
                AnchorChanges {
                    target: badgeLoader
                    anchors.right: undefined
                    anchors.horizontalCenter: defaultCompactRepresentation.horizontalCenter
                }
            }
        ]
    }
}
