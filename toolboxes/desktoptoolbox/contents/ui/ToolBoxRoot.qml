/*
    SPDX-FileCopyrightText: 2011 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Window 2.2
import org.kde.plasma.plasmoid 2.0
import org.kde.kirigami 2.20 as Kirigami


Item {
    id: main
    objectName: "org.kde.desktoptoolbox"

    z: 999
    anchors.fill: parent

    property Item toolBoxContent

    Connections {
        target: plasmoid
        function onAvailableScreenRegionChanged() {
            placeToolBoxTimer.restart();
        }
    }

    property int iconSize: Kirigami.Units.iconSizes.small
    property int iconWidth: Kirigami.Units.iconSizes.smallMedium
    property int iconHeight: iconWidth
    property bool dialogWasVisible: false
    property bool open: false

    onWidthChanged: placeToolBoxTimer.restart();
    onHeightChanged: placeToolBoxTimer.restart();

    LayoutMirroring.enabled: (Qt.application.layoutDirection === Qt.RightToLeft)
    LayoutMirroring.childrenInherit: true

    Plasmoid.containment.corona.onEditModeChanged: {
        if (!Plasmoid.containment.corona.editMode) {
            toolBoxContent.exitAnimation.start();
            return;
        }

        const component = Qt.createComponent(Qt.resolvedUrl("./ToolBoxContent.qml"), main);
        toolBoxContent = component.createObject(main);
        component.destroy();
        placeToolBox(Plasmoid.configuration.ToolBoxButtonState);
        toolBoxContent.enterAnimation.start();
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
        if (!main.toolBoxContent) {
            return;
        }
        // if nothing has been setup yet, determine default position based on layout direction
        if (!ts) {
            placeToolBox("topcenter");
            return;
        }

        var tx = Plasmoid.configuration.ToolBoxButtonX
        var ty = Plasmoid.configuration.ToolBoxButtonY
        var pos;
        const plasmoidItem = main.parent;

        switch (ts) {
        case "top":
            ty = main.y;
            pos = plasmoidItem.adjustToAvailableScreenRegion(tx, ty, toolBoxContent.width, toolBoxContent.height);
            break;
        case "bottom":
            ty = main.height + main.y - toolBoxContent.height;
            pos = plasmoidItem.adjustToAvailableScreenRegion(tx, ty, toolBoxContent.width, toolBoxContent.height);
            break;
        case "bottomcenter":
            tx = main.width / 2 - toolBoxContent.width / 2;
            ty = main.height + main.y - toolBoxContent.height;
            pos = plasmoidItem.adjustToAvailableScreenRegion(tx, ty, toolBoxContent.width, toolBoxContent.height);
            break;
        case "topcenter":
        default:
            tx = main.width / 2 - toolBoxContent.width / 2;
            ty = main.y;
            pos = plasmoidItem.adjustToAvailableScreenRegion(tx, ty, toolBoxContent.width, toolBoxContent.height);
            break;
        }
        //print("XXXY Setting toolbox to: " + ts + " " + tx + "x" + ty + " screen: " + main.width+ "x" + main.height+"");

        toolBoxContent.x = pos.x;
        toolBoxContent.y = pos.y;
    }
}
