/*
    SPDX-FileCopyrightText: 2012, 2015 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.4
import QtQuick.Layouts 1.4
import QtQuick.Window 2.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.20 as Kirigami

MouseArea {
    id: toolBoxContent

    property alias enterAnimation: enterAnimation
    property alias exitAnimation: exitAnimation

    visible: false
    opacity: 0
    transform: Translate {
        id: translator
    }
    transformOrigin: Item.Center

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

    width: buttonRow.width
    height: buttonRow.height

    state: "topcenter"

    property QtObject container: main
    property int pressedX
    property int pressedY
    property int snapStartX
    property bool snapX: false;
    property bool dragging: false

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

        if (!toolBoxContent.pressed) {
            main.placeToolBox(toolBoxContent.state);
        }

        stateTimer.running = false;
    }

    function destroyToolBox() {
        main.toolBoxContent = null;
        toolBoxContent.destroy();
    }

    SequentialAnimation {
        id: enterAnimation

        PropertyAction {
            target: toolBoxContent
            property: "visible"
            value: true
        }
        ParallelAnimation {
            NumberAnimation {
                target: toolBoxContent
                property: "opacity"
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.OutCubic
                to: 1
            }
            NumberAnimation {
                target: translator
                property: "y"
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.OutCubic
                from: state == "top" || state == "topcenter" ? -height
                    : state == "bottom" || state == "bottomcenter" ? height
                    : 0
                to: 0
            }
        }
    }

    SequentialAnimation {
        id: exitAnimation

        ParallelAnimation {
            NumberAnimation {
                target: toolBoxContent
                property: "opacity"
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InCubic
                to: 0
            }
            NumberAnimation {
                target: translator
                property: "y"
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InCubic
                to: state == "top" || state == "topcenter" ? -toolBoxContent.height
                    : state == "bottom" || state == "bottomcenter" ? toolBoxContent.height
                    : 0
            }
        }
        ScriptAction {
            script: toolBoxContent.destroyToolBox()
        }
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

    Row {
        id: buttonRow
        spacing: buttonLayout.columnSpacing

        Grid {
            id: buttonLayout
            rowSpacing: PlasmaCore.Units.smallSpacing
            columnSpacing: rowSpacing

            // Show buttons in two lines if screen space is limited
            readonly property real buttonWidth: addWidgetButton.implicitWidth
                + configureButton.implicitWidth
                + themeButton.implicitWidth
                + displaySettingsButton.implicitWidth
                + manageContainmentsButton.implicitWidth
                + (menuButton.visible ? menuButton.implicitWidth : 0)
                + closeButton.implicitWidth
            rows: Math.ceil(buttonWidth / (Screen.width * 0.8))

            PlasmaComponents3.ToolButton {
                id: addWidgetButton
                property QtObject qAction: plasmoid.action("add widgets")
                text: qAction.text
                icon.name: "list-add"
                onClicked: qAction.trigger()
            }

            PlasmaComponents3.ToolButton {
                id: configureButton
                property QtObject qAction: plasmoid.action("configure")
                text: qAction.text
                icon.name: "preferences-desktop-wallpaper"
                onClicked: qAction.trigger()
            }

            PlasmaComponents3.ToolButton {
                id: themeButton
                text: i18nd("plasma_toolbox_org.kde.desktoptoolbox", "Choose Global Theme…")
                icon.name: "preferences-desktop-theme-global"
                onClicked: KQuickControlsAddons.KCMShell.openSystemSettings("kcm_lookandfeel")
            }

            PlasmaComponents3.ToolButton {
                id: displaySettingsButton
                text: i18nd("plasma_toolbox_org.kde.desktoptoolbox", "Configure Display Settings…")
                icon.name: "preferences-desktop-display"
                onClicked: KQuickControlsAddons.KCMShell.openSystemSettings("kcm_kscreen")
            }

            PlasmaComponents3.ToolButton {
                id: manageContainmentsButton
                property QtObject qAction: plasmoid.globalAction("manage-containments")
                text: qAction.text
                visible: qAction.visible
                icon.name: "preferences-system-windows-effect-fadedesktop"
                onClicked: qAction.trigger()
            }

            PlasmaComponents3.ToolButton {
                id: menuButton

                height: addWidgetButton.height
                visible: Kirigami.Settings.hasTransientTouchInput || Kirigami.Settings.tabletMode

                icon.name: "overflow-menu"
                text: i18ndc("plasma_toolbox_org.kde.desktoptoolbox", "@action:button", "More")

                onClicked: {
                    Plasmoid.openContextMenu(mapToGlobal(0, height));
                }
            }
        }

        PlasmaComponents3.ToolButton {
            id: closeButton
            anchors.verticalCenter: buttonLayout.verticalCenter
            height: addWidgetButton.height
            icon.name: "window-close"
            onClicked: plasmoid.editMode = false
            PlasmaComponents3.ToolTip {
                text: i18nd("plasma_toolbox_org.kde.desktoptoolbox", "Exit Edit Mode")
            }
        }
    }
}
