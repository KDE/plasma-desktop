/*
    SPDX-FileCopyrightText: 2014 Ashish Madeti <ashishmadeti@gmail.com>
    SPDX-FileCopyrightText: 2016 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg

import org.kde.plasma.plasmoid

PlasmoidItem {
    id: root

    preferredRepresentation: fullRepresentation
    toolTipSubText: activeController.description

    Plasmoid.icon: Plasmoid.configuration.icon
    Plasmoid.title: activeController.title
    Plasmoid.onActivated: activeController.toggle();

    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    Layout.minimumWidth: Kirigami.Units.iconSizes.medium
    Layout.minimumHeight: Kirigami.Units.iconSizes.medium

    Layout.maximumWidth: Layout.minimumWidth
    Layout.maximumHeight: Layout.minimumHeight

    Layout.preferredWidth: Layout.minimumWidth
    Layout.preferredHeight: Layout.minimumHeight

    readonly property bool inPanel: [PlasmaCore.Types.TopEdge, PlasmaCore.Types.RightEdge, PlasmaCore.Types.BottomEdge, PlasmaCore.Types.LeftEdge]
            .includes(Plasmoid.location)

    readonly property bool vertical: Plasmoid.location === PlasmaCore.Types.RightEdge || Plasmoid.location === PlasmaCore.Types.LeftEdge

    /**
    * @c true if the current applet is Minimize All, @c false if the
    * current applet is Show Desktop.
    */
    readonly property bool isMinimizeAll: Plasmoid.pluginName === "org.kde.plasma.minimizeall"

    readonly property Controller primaryController: isMinimizeAll ? minimizeAllController : peekController

    readonly property Controller activeController: {
        if (Plasmoid.containment.corona.editMode) {
            return primaryController;
        } else if (minimizeAllController.active) {
            return minimizeAllController;
        } else if (peekController.active) {
            return peekController;
        } else {
            return primaryController;
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent

        activeFocusOnTab: true
        hoverEnabled: true

        onClicked: Plasmoid.activated();

        Keys.onPressed: event => {
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
        Accessible.description: root.toolTipSubText
        Accessible.role: Accessible.Button
        Accessible.onPressAction: Plasmoid.activated()

        PeekController {
            id: peekController
        }

        MinimizeAllController {
            id: minimizeAllController
        }

        Kirigami.Icon {
            anchors.fill: parent
            active: mouseArea.containsMouse || root.activeController.active
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
        KSvg.FrameSvgItem {
            Accessible.name: i18n("Minimize All Applet Active Indicator")  // qmllint disable unqualified
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
                topMargin: !root.vertical && containerMargins ? -containerMargins('top', returnAllMargins) : 0
                leftMargin: root.vertical && containerMargins ? -containerMargins('left', returnAllMargins) : 0
                rightMargin: root.vertical && containerMargins ? -containerMargins('right', returnAllMargins) : 0
                bottomMargin: !root.vertical && containerMargins ? -containerMargins('bottom', returnAllMargins) : 0
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
            opacity: root.activeController.active ? 1 : 0

            Behavior on opacity {
                NumberAnimation {
                    duration: Kirigami.Units.shortDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }

        PlasmaCore.ToolTipArea {
            id: toolTip
            anchors.fill: parent
            mainText: Plasmoid.title
            subText: root.toolTipSubText
            textFormat: Text.PlainText
        }
    }

    Plasmoid.contextualActions: [
        PlasmaCore.Action {
            text: minimizeAllController.title
            icon.name: minimizeAllController.active ? "window-restore-symbolic" : "window-minimize-symbolic"
            toolTip: minimizeAllController.description
            enabled: !peekController.active
            onTriggered: minimizeAllController.toggle()
        },
        PlasmaCore.Action {
            text: peekController.title
            icon.name: peekController.active ? "window-symbolic" : "desktop-symbolic"
            toolTip: peekController.description
            enabled: !minimizeAllController.active
            onTriggered: peekController.toggle()
        }
    ]
}
