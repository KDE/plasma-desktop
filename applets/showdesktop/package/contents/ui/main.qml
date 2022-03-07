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

QtObject {
    id: root

    // you can't have an applet with just a compact representation :(
    Plasmoid.preferredRepresentation: Plasmoid.fullRepresentation
    Plasmoid.onActivated: showdesktop.showingDesktop = !showdesktop.showingDesktop
    Plasmoid.icon: Plasmoid.configuration.icon
    Plasmoid.title: i18n("Show Desktop")
    Plasmoid.toolTipSubText: i18n("Show the desktop by moving windows aside")
    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    // QtObject has no default property
    property QtObject showdesktop: ShowDesktop { }

    Component.onCompleted: {
        Plasmoid.setAction("minimizeall", i18nc("@action", "Minimize All Windows"))
    }

    function action_minimizeall() {
        showdesktop.minimizeAll()
    }

    Plasmoid.fullRepresentation: PlasmaCore.ToolTipArea {
        id: fullRep

        readonly property bool inPanel: (Plasmoid.location === PlasmaCore.Types.TopEdge
            || Plasmoid.location === PlasmaCore.Types.RightEdge
            || Plasmoid.location === PlasmaCore.Types.BottomEdge
            || Plasmoid.location === PlasmaCore.Types.LeftEdge)

        Layout.minimumWidth: PlasmaCore.Units.iconSizes.small
        Layout.minimumHeight: Layout.minimumWidth

        Layout.maximumWidth: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1
        Layout.maximumHeight: inPanel ? PlasmaCore.Units.iconSizeHints.panel : -1

        mainText: Plasmoid.title
        subText: Plasmoid.toolTipSubText

        MouseArea {
            anchors.fill: parent
            activeFocusOnTab: true
            Keys.onPressed: {
                switch (event.key) {
                case Qt.Key_Space:
                case Qt.Key_Enter:
                case Qt.Key_Return:
                case Qt.Key_Select:
                    showdesktop.showingDesktop = !showdesktop.showingDesktop;
                    break;
                }
            }
            Accessible.name: root.Plasmoid.title
            Accessible.description: root.Plasmoid.toolTipSubText
            Accessible.role: Accessible.Button
            onClicked: showdesktop.showingDesktop = !showdesktop.showingDesktop
        }

        PlasmaCore.IconItem {
            anchors.fill: parent
            source: Plasmoid.icon
            active: parent.containsMouse || showdesktop.showingDesktop
        }

        // also activate when dragging an item over the plasmoid so a user can easily drag data to the desktop
        DropArea {
            anchors.fill: parent
            onEntered: activateTimer.start()
            onExited: activateTimer.stop()

            Timer {
                id: activateTimer
                interval: 250 //to match TaskManager
                onTriggered: Plasmoid.activated()
            }
        }

        // Active/not active indicator
        PlasmaCore.FrameSvgItem {
            property var containerMargins: {
                let item = fullRep;
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
                bottomMargin: containerMargins ? -containerMargins('bottom', returnAllMargins) : 0;
                topMargin: containerMargins ? -containerMargins('top', returnAllMargins) : 0;
                leftMargin: containerMargins ? -containerMargins('left', returnAllMargins) : 0;
                rightMargin: containerMargins ? -containerMargins('right', returnAllMargins) : 0;
            }
            imagePath: "widgets/tabbar"
            visible: fromCurrentTheme && opacity > 0
            prefix: {
                var prefix;
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
    }
}
