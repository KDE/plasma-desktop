/*
    SPDX-FileCopyrightText: 2014 Ashish Madeti <ashishmadeti@gmail.com>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.plasma.plasmoid 2.0

import org.kde.plasma.private.showdesktop 0.1

MouseArea {
    id: root

    readonly property bool inPanel: [PlasmaCore.Types.TopEdge, PlasmaCore.Types.RightEdge, PlasmaCore.Types.BottomEdge, PlasmaCore.Types.LeftEdge]
        .includes(Plasmoid.location)

    Plasmoid.icon: Plasmoid.configuration.icon
    Plasmoid.title: i18n("Show Desktop")
    Plasmoid.toolTipSubText: i18n("Show the desktop by moving windows aside")

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    Layout.minimumWidth: PlasmaCore.Units.iconSizes.small
    Layout.minimumHeight: PlasmaCore.Units.iconSizes.small

    Layout.maximumWidth: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1
    Layout.maximumHeight: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1

    Plasmoid.onActivated: showdesktop.toggleDesktop()
    onClicked: Plasmoid.activated();

    hoverEnabled: true

    activeFocusOnTab: true
    Keys.onPressed: {
        switch (event.key) {
        case Qt.Key_Space:
        case Qt.Key_Enter:
        case Qt.Key_Return:
        case Qt.Key_Select:
            Plasmoid.activated();
            break;
        }
    }
    Accessible.name: Plasmoid.title
    Accessible.description: Plasmoid.toolTipSubText
    Accessible.role: Accessible.Button

    ShowDesktop { id: showdesktop }

    PlasmaCore.IconItem {
        anchors.fill: parent
        active: root.containsMouse || showdesktop.showingDesktop
        source: Plasmoid.icon
    }

    // also activate when dragging an item over the plasmoid so a user can easily drag data to the desktop
    DropArea {
        anchors.fill: parent
        onEntered: activateTimer.start()
        onExited: activateTimer.stop()
    }

    Timer {
        id: activateTimer
        interval: 250 // to match TaskManager
        onTriggered: Plasmoid.activated()
    }

    // Active/not active indicator
    PlasmaCore.FrameSvgItem {
        property var containerMargins: {
            let item = this;
            while (item.parent) {
                item = item.parent;
                if (item.isAppletContainer) {
                    return item.getMargins;
                }
            }
            return undefined;
        }

        anchors {
            fill: parent
            property bool returnAllMargins: true
            // The above makes sure margin is returned even for side margins
            // that would be otherwise turned off.
            topMargin: containerMargins ? -containerMargins('top', returnAllMargins) : 0
            leftMargin: containerMargins ? -containerMargins('left', returnAllMargins) : 0
            rightMargin: containerMargins ? -containerMargins('right', returnAllMargins) : 0
            bottomMargin: containerMargins ? -containerMargins('bottom', returnAllMargins) : 0
        }
        imagePath: "widgets/tabbar"
        visible: fromCurrentTheme && opacity > 0
        prefix: {
            let prefix;
            switch (Plasmoid.location) {
            case PlasmaCore.Types.LeftEdge:
                prefix = "west-active-tab";
                break;
            case PlasmaCore.Types.TopEdge:
                prefix = "north-active-tab";
                break;
            case PlasmaCore.Types.RightEdge:
                prefix = "east-active-tab";
                break;
            default:
                prefix = "south-active-tab";
            }
            if (!hasElementPrefix(prefix)) {
                prefix = "active-tab";
            }
            return prefix;
        }
        opacity: showdesktop.showingDesktop ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                duration: PlasmaCore.Units.shortDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    function action_minimizeall() {
        showdesktop.minimizeAll();
    }

    Component.onCompleted: {
        Plasmoid.setAction("minimizeall", i18nc("@action", "Minimize All Windows"))
    }
}
