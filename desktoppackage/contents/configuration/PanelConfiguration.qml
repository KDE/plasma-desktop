/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.configuration 2.0
import "panelconfiguration"


//TODO: all of this will be done with desktop components
PlasmaCore.FrameSvgItem {
    id: dialogRoot

    signal closeContextMenu

//BEGIN Properties
    width: 640
    height: 64
    imagePath: "dialogs/background"

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    state: {
        switch (panel.location) {
        case PlasmaCore.Types.TopEdge:
            return "TopEdge"
        case PlasmaCore.Types.LeftEdge:
            return "LeftEdge"
        case PlasmaCore.Types.RightEdge:
            return "RightEdge"
        case PlasmaCore.Types.BottomEdge:
        default:
            return "BottomEdge"
        }
    }

    property bool vertical: (panel.location === PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge)
//END properties

//BEGIN Connections
    Connections {
        target: panel
        function onOffsetChanged() {
            ruler.offset = panel.offset
        }
        function onMinimumLengthChanged() {
            ruler.minimumLength = panel.minimumLength
        }
        function onMaximumLengthChanged() {
            ruler.maximumLength = panel.maximumLength
        }
    }

    Connections {
        target: plasmoid
        function onImmutableChanged() {
            configDialog.close()
        }
    }
//END Connections


//BEGIN UI components

    Ruler {
        id: ruler
        state: dialogRoot.state
    }

    ToolBar {
        id: toolBar
        state: dialogRoot.state
    }
//END UI components

//BEGIN Animations
    //when EdgeHandle is released animate to old panel position
    ParallelAnimation {
        id: panelResetAnimation
        NumberAnimation {
            target: panel
            properties: (panel.location === PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge) ? "x" : "y"
            to:  {
                switch (panel.location) {
                case PlasmaCore.Types.TopEdge:
                    return panel.screenGeometry.y + panel.distance
                case PlasmaCore.Types.LeftEdge:
                    return panel.screenGeometry.x + panel.distance
                case PlasmaCore.Types.RightEdge:
                    return panel.screenGeometry.x + panel.screenGeometry.width - panel.width - panel.distance
                case PlasmaCore.Types.BottomEdge:
                default:
                    return panel.screenGeometry.y + panel.screenGeometry.height - panel.height - panel.distance
                }
            }
            duration: PlasmaCore.Units.shortDuration
        }

        NumberAnimation {
            target: configDialog
            properties: (panel.location === PlasmaCore.Types.LeftEdge || panel.location === PlasmaCore.Types.RightEdge) ? "x" : "y"
            to: {
                switch (panel.location) {
                case PlasmaCore.Types.TopEdge:
                    return panel.screenGeometry.y + panel.height + panel.distance
                case PlasmaCore.Types.LeftEdge:
                    return panel.screenGeometry.x + panel.width + panel.distance
                case PlasmaCore.Types.RightEdge:
                    return panel.screenGeometry.x + panel.screenGeometry.width - panel.width - configDialog.width - panel.distance
                case PlasmaCore.Types.BottomEdge:
                default:
                    return panel.screenGeometry.y + panel.screenGeometry.height - panel.height - configDialog.height - panel.distance
                }
            }
            duration: PlasmaCore.Units.shortDuration
        }
    }
//END Animations

//BEGIN States
states: [
        State {
            name: "TopEdge"
            PropertyChanges {
                target: dialogRoot
                enabledBorders: "TopBorder|BottomBorder"
            }
            PropertyChanges {
                target: dialogRoot
                implicitHeight: ruler.implicitHeight + toolBar.implicitHeight
            }
        },
        State {
            name: "BottomEdge"
            PropertyChanges {
                target: dialogRoot
                enabledBorders: "TopBorder|BottomBorder"
            }
            PropertyChanges {
                target: dialogRoot
                implicitHeight: ruler.implicitHeight + toolBar.implicitHeight
            }
        },
        State {
            name: "LeftEdge"
            PropertyChanges {
                target: dialogRoot
                enabledBorders: "LeftBorder|RightBorder"
            }
            PropertyChanges {
                target: dialogRoot
                implicitWidth: ruler.implicitWidth + toolBar.implicitWidth
            }
        },
        State {
            name: "RightEdge"
            PropertyChanges {
                target: dialogRoot
                enabledBorders: "LeftBorder|RightBorder"
            }
            PropertyChanges {
                target: dialogRoot
                implicitWidth: ruler.implicitWidth + toolBar.implicitWidth
            }
        }
    ]
//END States
}
