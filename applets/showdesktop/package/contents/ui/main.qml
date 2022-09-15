/*
    SPDX-FileCopyrightText: 2014 Ashish Madeti <ashishmadeti@gmail.com>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.plasma.plasmoid 2.0

MouseArea {
    id: root

    readonly property bool inPanel: [PlasmaCore.Types.TopEdge, PlasmaCore.Types.RightEdge, PlasmaCore.Types.BottomEdge, PlasmaCore.Types.LeftEdge]
        .includes(Plasmoid.location)

    /**
     * @c true if the current applet is Minimize All, @c false if the
     * current applet is Show Desktop.
     */
    readonly property bool isMinimizeAll: Plasmoid.pluginName === "org.kde.plasma.minimizeall"

    readonly property Controller primaryController: isMinimizeAll ? minimizeAllController : peekController

    readonly property Controller activeController: {
        if (minimizeAllController.active) {
            return minimizeAllController;
        } else if (peekController.active) {
            return peekController;
        } else {
            return primaryController;
        }
    }

    Plasmoid.icon: Plasmoid.configuration.icon
    Plasmoid.title: activeController.title
    Plasmoid.toolTipSubText: activeController.description

    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    Layout.minimumWidth: PlasmaCore.Units.iconSizes.small
    Layout.minimumHeight: PlasmaCore.Units.iconSizes.small

    Layout.maximumWidth: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1
    Layout.maximumHeight: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1

    activeFocusOnTab: true
    hoverEnabled: true

    Plasmoid.onActivated: activeController.toggle();
    onClicked: Plasmoid.activated();

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

    PeekController {
        id: peekController
    }

    MinimizeAllController {
        id: minimizeAllController
    }

    PlasmaCore.IconItem {
        anchors.fill: parent
        active: root.containsMouse || activeController.active
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
        visible: opacity > 0
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
        opacity: activeController.active ? 1 : 0

        Behavior on opacity {
            NumberAnimation {
                duration: PlasmaCore.Units.shortDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    PlasmaCore.ToolTipArea {
        id: toolTip
        anchors.fill: parent
        mainText: Plasmoid.title
        subText: Plasmoid.toolTipSubText
        textFormat: Text.PlainText
    }

    function action_minimizeall() {
        minimizeAllController.toggle();
    }

    function action_peek() {
        peekController.toggle();
    }

    Component.onCompleted: {
        var action;

        Plasmoid.setAction("minimizeall", "");
        action = Plasmoid.action("minimizeall")
        action.checkable = true;
        action.checked = Qt.binding(() => minimizeAllController.active);
        action.text = Qt.binding(() => minimizeAllController.titleInactive);
        action.toolTip = Qt.binding(() => minimizeAllController.description);
        action.enabled = Qt.binding(() => !peekController.active)

        Plasmoid.setAction("peek", "");
        action = Plasmoid.action("peek")
        action.checkable = true;
        action.checked = Qt.binding(() => peekController.active);
        action.text = Qt.binding(() => peekController.titleInactive);
        action.toolTip = Qt.binding(() => peekController.description);
        action.enabled = Qt.binding(() => !minimizeAllController.active)
    }
}
