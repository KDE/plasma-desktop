/***************************************************************************
 *   Copyright 2012,2015 by Sebastian Kügler <sebas@kde.org>               *
 *   Copyright 2015 by Kai Uwe Broulik <kde@privat.broulik.de>             *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

import QtQuick 2.4
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.plasma.plasmoid 2.0

Item {
    id: toolBoxButton

    property string text: main.Plasmoid.activityName
    property bool isCorner: !buttonMouse.dragging &&
                            ((state == "topleft") || (state == "topright") ||
                             (state == "bottomright") || (state == "bottomleft"))
    property bool isHorizontal: (state != "left" && state != "right")

    rotation: switch(state) {
        case "left":
            return -90;
        case "right":
            return 90;
        default:
            return 0;
    }

    transform: Translate {
        x: state == "left" ? Math.round(-width/2 + height/2) : state == "right" ? + Math.round(width/2 - height/2) : 0
        Behavior on x {
            NumberAnimation {
                duration: units.shortDuration * 3;
                easing.type: Easing.InOutExpo;
            }
        }
    }
    transformOrigin: Item.Center
    Behavior on rotation {
        NumberAnimation {
            duration: units.shortDuration * 3;
            easing.type: Easing.InOutExpo;
        }
        enabled: visible
    }
    Behavior on x {
        NumberAnimation {
            duration: units.shortDuration * 3;
            easing.type: Easing.InOutExpo;
        }
        enabled: visible

    }
    Behavior on y {
        NumberAnimation {
            duration: units.shortDuration * 3;
            easing.type: Easing.InOutExpo;
        }
        enabled: visible
    }

    clip: backgroundFrameWidthAnimation.running
    width: backgroundFrame.width + backgroundFrame.width % 2
    height: backgroundFrame.height + backgroundFrame.height % 2

    //x and y default to 0, so top left would be correct
    //If the position is anything else it will updated via onXChanged during intialisation
    state: "topleft"

    onXChanged: stateTimer.restart()
    onYChanged: stateTimer.restart()

    Timer {
        id: stateTimer
        interval: 100
        onTriggered: updateState()
    }
    function updateState() {
        var container = main;
        //print("    w: " + container.width +"x"+container.height+" : "+x+"/"+y+" tbw: " + toolBoxButton.width);

        var x = toolBoxButton.x - main.x;
        var y = toolBoxButton.y - main.y;

        var cornerSnap = iconWidth

        if (x < cornerSnap && y < cornerSnap) {
            toolBoxButton.state = "topleft";
        } else if (container.width - x - buttonLayout.width < cornerSnap && y < cornerSnap) {
            toolBoxButton.state = "topright";
        } else if (container.width - x - buttonLayout.width < cornerSnap && container.height - y - buttonLayout.width  < cornerSnap) {
            toolBoxButton.state = "bottomright";
        } else if (x < cornerSnap && container.height - y - buttonLayout.width < cornerSnap) {
            toolBoxButton.state = "bottomleft";
        //top diagonal half
        } else if (x > (y * (container.width/container.height))) {
            //Top edge
            if (container.width - x > y ) {
                toolBoxButton.state = "top";
            //right edge
            } else {
                //toolBoxButton.transformOrigin = Item.BottomRight
                toolBoxButton.state = "right";
            }
        //bottom diagonal half
        } else {
            //Left edge
            if (container.height - y > x ) {
                //toolBoxButton.transformOrigin = Item.TopLeft
                toolBoxButton.state = "left";
            //Bottom edge
            } else {
                toolBoxButton.state = "bottom";
            }
        }

        if (!buttonMouse.pressed) {
            main.placeToolBox(toolBoxButton.state);
        }

        stateTimer.running = false;
    }

    PlasmaCore.FrameSvgItem {
        id: backgroundFrame
        anchors {
            left: parent.left
            top: parent.top
        }
        imagePath: "widgets/translucentbackground"
        opacity: buttonMouse.containsMouse || (toolBoxLoader.item && toolBoxLoader.item.visible) ? 1.0 : 0.4
        width: Math.round((isCorner ? buttonLayout.height : buttonLayout.width) + margins.horizontal)
        height: Math.round(buttonLayout.height + margins.vertical)
        Behavior on width {
            NumberAnimation {
                id: backgroundFrameWidthAnimation
                duration: units.longDuration;
                easing.type: Easing.InOutQuad;
            }
        }
        Behavior on opacity {
            NumberAnimation {
                duration: units.longDuration;
            }
        }
    }

    Row {
        id: buttonLayout
        anchors.centerIn: parent
        height: Math.max(toolBoxIcon.height, fontMetrics.height)
        spacing: units.smallSpacing

        Behavior on x {
            NumberAnimation {
                duration: units.longDuration;
                easing.type: Easing.InOutQuad;
            }
        }

        PlasmaCore.SvgItem {
            id: toolBoxIcon
            svg: PlasmaCore.Svg {
                id: iconSvg
                imagePath: "widgets/configuration-icons"
                onRepaintNeeded: toolBoxIcon.elementId = iconSvg.hasElement("menu") ? "menu" : "configure"
            }
            elementId: iconSvg.hasElement("menu") ? "menu" : "configure"
            anchors.verticalCenter: parent.verticalCenter
            width: iconSize
            height: iconSize
            opacity: buttonMouse.containsMouse || (toolBoxLoader.item && toolBoxLoader.item.visible) ? 1 : 0.5
            rotation: isHorizontal ? 0 : -90;
            transformOrigin: Item.Center
            Behavior on opacity {
                NumberAnimation {
                    duration: units.longDuration;
                    easing.type: Easing.InOutExpo;
                }
            }
        }

        PlasmaComponents.Label {
            id: activityName
            anchors.verticalCenter: parent.verticalCenter
            opacity: isCorner ? 0 : 1
            text: toolBoxButton.text
            visible: opacity
            Behavior on opacity {
                //only have this animation when going from hidden -> shown
                enabled: activityName.opacity == 0

                SequentialAnimation {
                    //pause to allow the toolbox frame to resize
                    //otherwise we see the text overflow the box
                    //whilst that animates
                    PauseAnimation {
                        duration: units.longDuration
                    }
                    NumberAnimation {
                        duration: units.shortDuration
                        easing.type: Easing.InOutExpo
                    }
                }
            }
        }

        FontMetrics {
            id: fontMetrics
            font: activityName.font
        }
    }

    MouseArea {
        id: buttonMouse

        property QtObject container: main
        property int pressedX
        property int pressedY
        property bool dragging: false

        anchors {
            fill: parent
            margins: -10
        }

        drag {
            target: main.Plasmoid.immutable ? undefined : toolBoxButton
            minimumX: 0
            maximumX: container.width - toolBoxIcon.width
            minimumY: 0
            maximumY: container.height - toolBoxIcon.height
        }

        hoverEnabled: true

        onPressed: {
            pressedX = toolBoxButton.x
            pressedY = toolBoxButton.y
        }
        onPositionChanged: {
            if (pressed && (Math.abs(toolBoxButton.x - pressedX) > iconSize ||
                Math.abs(toolBoxButton.y - pressedY) > iconSize)) {
                dragging = true;
            }
        }
        onClicked: {
            // the dialog auto-closes on losing focus
            main.open = !main.dialogWasVisible
            plasmoid.focus = true;
        }
        onReleased: {
            main.Plasmoid.configuration.ToolBoxButtonState = toolBoxButton.state;
            main.Plasmoid.configuration.ToolBoxButtonX = toolBoxButton.x;
            main.Plasmoid.configuration.ToolBoxButtonY = toolBoxButton.y;
            //print("Saved coordinates for ToolBox in config: " + toolBoxButton.x + ", " +toolBoxButton.x);
            if (dragging) {
                main.placeToolBox();
            }
            dragging = false;
            stateTimer.stop();
            updateState();
        }
        onCanceled: dragging = false;
    }

    states: [
        State {
            name: "topleft"
        },
        State {
            name: "top"
        },
        State {
            name: "topright"
        },
        State {
            name: "right"
        },
        State {
            name: "bottomright"
        },
        State {
            name: "bottom"
        },
        State {
            name: "bottomleft"
        },
        State {
            name: "topleft"
        },
        State {
            name: "left"
        }
    ]
}
