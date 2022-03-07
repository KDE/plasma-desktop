/*
    SPDX-FileCopyrightText: 2012, 2015 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.4
import QtQuick.Layouts 1.4
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.plasma.plasmoid 2.0

Item {
    id: toolBoxContent

    transform: Translate {
        y: Plasmoid.editMode ? 0
           : state == "top" || state == "topcenter" ? -height
           : state == "bottom" || state == "bottomcenter" ? height
           : 0

        Behavior on y {
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
    transformOrigin: Item.Center
    opacity: Plasmoid.editMode
    visible: Plasmoid.editMode
    Behavior on opacity {
        OpacityAnimator {
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.InOutQuad
        }
        enabled: visible
    }
    Behavior on rotation {
        NumberAnimation {
            duration: PlasmaCore.Units.shortDuration;
            easing.type: Easing.InOutExpo;
        }
        enabled: visible
    }
    Behavior on x {
        NumberAnimation {
            duration: PlasmaCore.Units.shortDuration;
            easing.type: Easing.InOutExpo;
        }
        enabled: visible

    }
    Behavior on y {
        NumberAnimation {
            duration: PlasmaCore.Units.shortDuration;
            easing.type: Easing.InOutExpo;
        }
        enabled: visible
    }

    width: buttonLayout.width
    height: buttonLayout.height

    state: "topcenter"

    onXChanged: stateTimer.restart()
    onYChanged: stateTimer.restart()

    Timer {
        id: stateTimer
        interval: 0
        onTriggered: updateState()
    }
    function updateState() {
        var container = main;
        //print("    w: " + container.width +"x"+container.height+" : "+x+"/"+y+" tbw: " + toolBoxContent.width);

        var x = toolBoxContent.x - main.x;
        var y = toolBoxContent.y - main.y;

        var cornerSnap = iconWidth

        //top
        if (y + height / 2 < container.height / 2) {
            if (Math.abs(container.width/2 - (x + width/2)) < PlasmaCore.Units.gridUnit) {
                toolBoxContent.state = "topcenter";
            } else {
                toolBoxContent.state = "top";
            }
        //bottom
        } else {
            if (Math.abs(container.width/2 - (x + height/2)) < PlasmaCore.Units.gridUnit) {
                toolBoxContent.state = "bottomcenter";
            } else {
                toolBoxContent.state = "bottom";
            }
        }

        if (!buttonMouse.pressed) {
            main.placeToolBox(toolBoxContent.state);
        }

        stateTimer.running = false;
    }

    PlasmaCore.FrameSvgItem {
        id: backgroundFrame
        anchors {
            fill: parent
            leftMargin: -backgroundFrame.margins.left
            topMargin: -backgroundFrame.margins.top
            rightMargin: -backgroundFrame.margins.right
            bottomMargin: -backgroundFrame.margins.bottom
        }
        imagePath: "widgets/background"
        width: Math.round(buttonLayout.width + margins.horizontal)
        height: Math.round(buttonLayout.height + margins.vertical)
    }

    MouseArea {
        id: buttonMouse

        property QtObject container: main
        property int pressedX
        property int pressedY
        property int snapStartX
        property bool snapX: false;
        property bool dragging: false

        anchors.fill: parent

        drag {
            filterChildren: true
            target: main.Plasmoid.immutable ? undefined : toolBoxContent
            minimumX: 0
            maximumX: container.width - toolBoxContent.width
            minimumY: 0
            maximumY: container.height
        }

        hoverEnabled: true

        onPressed: {
            pressedX = toolBoxContent.x
            pressedY = toolBoxContent.y
        }
        onPositionChanged: {
            if (pressed && (Math.abs(toolBoxContent.x - pressedX) > iconSize ||
                Math.abs(toolBoxContent.y - pressedY) > iconSize)) {
                dragging = true;
            }

            // Center snapping X
            if (snapX && Math.abs(snapStartX - mouse.x) > PlasmaCore.Units.gridUnit) {
                toolBoxContent.anchors.horizontalCenter = undefined;
                snapX = false;
            } else if (!snapX && Math.abs(main.width/2 - (toolBoxContent.x + toolBoxContent.width/2)) < PlasmaCore.Units.gridUnit) {
                toolBoxContent.anchors.horizontalCenter = main.horizontalCenter;
                snapStartX = mouse.x;
                snapX = true;
            }
        }

        onReleased: {
            toolBoxContent.anchors.horizontalCenter = undefined;
            toolBoxContent.anchors.verticalCenter = undefined;
            snapX = false;
            main.Plasmoid.configuration.ToolBoxButtonState = toolBoxContent.state;
            main.Plasmoid.configuration.ToolBoxButtonX = toolBoxContent.x;
            main.Plasmoid.configuration.ToolBoxButtonY = toolBoxContent.y;
            //print("Saved coordinates for ToolBox in config: " + toolBoxContent.x + ", " +toolBoxContent.x);
            if (dragging) {
                main.placeToolBox();
            }
            dragging = false;
            stateTimer.stop();
            updateState();
        }
        onCanceled: dragging = false;

        RowLayout {
            id: buttonLayout
            anchors.centerIn: parent
            spacing: PlasmaCore.Units.smallSpacing


            PlasmaComponents3.ToolButton {
                property QtObject qAction: Plasmoid.action("add widgets")
                text: qAction.text
                icon.name: "list-add"
                onClicked: qAction.trigger()
            }
            PlasmaComponents3.ToolButton {
                property QtObject qAction: Plasmoid.action("configure")
                text: qAction.text
                icon.name: "preferences-desktop-wallpaper"
                onClicked: qAction.trigger()
            }
            PlasmaComponents3.ToolButton {
                text: i18nd("plasma_toolbox_org.kde.desktoptoolbox", "Choose Global Theme…")
                icon.name: "preferences-desktop-theme-global"
                onClicked: KQuickControlsAddons.KCMShell.openSystemSettings("kcm_lookandfeel")
            }
            PlasmaComponents3.ToolButton {
                text: i18nd("plasma_toolbox_org.kde.desktoptoolbox", "Configure Display Settings…")
                icon.name: "preferences-desktop-display"
                onClicked: KQuickControlsAddons.KCMShell.openSystemSettings("kcm_kscreen")
            }

            PlasmaComponents3.ToolButton {
                property QtObject qAction: Plasmoid.globalAction("manage-containments")
                text: qAction.text
                visible: qAction.visible
                icon.name: "preferences-system-windows-effect-fadedesktop"
                onClicked: qAction.trigger()
            }

            PlasmaComponents3.ToolButton {
                icon.name: "window-close"
                Layout.preferredWidth: height
                onClicked: Plasmoid.editMode = false
                PlasmaComponents3.ToolTip {
                    text: i18nd("plasma_toolbox_org.kde.desktoptoolbox", "Exit Edit Mode")
                }
            }
        }
    }
}
