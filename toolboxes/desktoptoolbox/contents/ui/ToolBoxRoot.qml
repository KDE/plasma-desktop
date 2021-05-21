/*
    SPDX-FileCopyrightText: 2011 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Window 2.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.plasmoid 2.0


Item {
    id: main
    objectName: "org.kde.desktoptoolbox"

    z: 999
    anchors.fill: parent

    Connections {
        target: plasmoid
        function onAvailableScreenRegionChanged() {
            placeToolBoxTimer.restart();
        }
    }

    property int iconSize: PlasmaCore.Units.iconSizes.small
    property int iconWidth: PlasmaCore.Units.iconSizes.smallMedium
    property int iconHeight: iconWidth
    property bool dialogWasVisible: false
    property bool open: false

    onWidthChanged: placeToolBoxTimer.restart();
    onHeightChanged: placeToolBoxTimer.restart();

    LayoutMirroring.enabled: (Qt.application.layoutDirection === Qt.RightToLeft)
    LayoutMirroring.childrenInherit: true

    ToolBoxButton {
        id: toolBoxButton
        Component.onCompleted: {
            placeToolBox(plasmoid.configuration.ToolBoxButtonState);
        }
    }

    Timer {
        id: placeToolBoxTimer
        interval: 100
        repeat: false
        running: false
        onTriggered: {
            placeToolBox(plasmoid.configuration.ToolBoxButtonState);
        }
    }

    function placeToolBox(ts) {
        // if nothing has been setup yet, determine default position based on layout direction
        if (!ts) {
            placeToolBox("topcenter");
            return;
        }

        var tx = Plasmoid.configuration.ToolBoxButtonX
        var ty = Plasmoid.configuration.ToolBoxButtonY
        var pos;

        switch (ts) {
        case "top":
            ty = main.y;
            pos = plasmoid.adjustToAvailableScreenRegion(tx, ty, toolBoxButton.width, toolBoxButton.height);
            break;
        case "bottom":
            ty = main.height + main.y - toolBoxButton.height;
            pos = plasmoid.adjustToAvailableScreenRegion(tx, ty, toolBoxButton.width, toolBoxButton.height);
            break;
        case "bottomcenter":
            tx = main.width / 2 - toolBoxButton.width / 2;
            ty = main.height + main.y - toolBoxButton.height;
            pos = plasmoid.adjustToAvailableScreenRegion(tx, ty, toolBoxButton.width, toolBoxButton.height);
            break;
        case "topcenter":
        default:
            tx = main.width / 2 - toolBoxButton.width / 2;
            ty = main.y;
            pos = plasmoid.adjustToAvailableScreenRegion(tx, ty, toolBoxButton.width, toolBoxButton.height);
            break;
        }
        //print("XXXY Setting toolbox to: " + ts + " " + tx + "x" + ty + " screen: " + main.width+ "x" + main.height+"");

        toolBoxButton.x = pos.x;
        toolBoxButton.y = pos.y;
    }
}
