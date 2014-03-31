/*
 *  Copyright 2012 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore


PlasmaCore.FrameSvgItem {
    id: root
    width: 640
    height: 32
    imagePath: "widgets/panel-background"
    visible: false //adjust borders is run during setup. We want to avoid painting till completed

    property Item containment

    function adjustBorders() {
        //print("PANEL ADJUST BORDERS");
        var borders = PlasmaCore.FrameSvg.AllBorders;
        if (!containment) {
            // Containment seems to never be set
            // (or not reachable from here at least)
            // FIXME: investigate why containment is null
            print("FIXME Panel borders hardcoded in Panel.qml");
            borders = borders & ~PlasmaCore.FrameSvg.BottomBorder;
            borders = borders & ~PlasmaCore.FrameSvg.LeftBorder;
            borders = borders & ~PlasmaCore.FrameSvg.RightBorder;
            root.enabledBorders = borders;
            return;
        }

        switch (containment.location) {
        case PlasmaCore.Types.TopEdge:
            borders = borders & ~PlasmaCore.FrameSvg.TopBorder;
            break;
        case PlasmaCore.Types.LeftEdge:
            borders = borders & ~PlasmaCore.FrameSvg.LeftBorder;
            break;
        case PlasmaCore.Types.RightEdge:
            borders = borders & ~PlasmaCore.FrameSvg.RightBorder;
            break;
        case PlasmaCore.Types.BottomEdge:
        default:
            print("PANEL disable bottom border");
            borders = borders & ~PlasmaCore.FrameSvg.BottomBorder;
            break;
        }

        if (panel.x <= panel.screen.geometry.x) {
            borders = borders & ~PlasmaCore.FrameSvg.LeftBorder;
        }
        if (panel.x + panel.width >= panel.screen.geometry.x + panel.screen.geometry.width) {
            borders = borders & ~PlasmaCore.FrameSvg.RightBorder;
        }
        if (panel.y <= panel.screen.geometry.y) {
            borders = borders & ~PlasmaCore.FrameSvg.TopBorder;
        }
        if (panel.y + panel.height >= panel.screen.geometry.y + panel.screen.geometry.height) {
            borders = borders & ~PlasmaCore.FrameSvg.BottomBorder;
        }

        root.enabledBorders = borders;

        containmentParent.anchors.topMargin = Math.min(root.margins.top, Math.max(1, root.height - units.iconSizes.smallMedium));

        containmentParent.anchors.bottomMargin = Math.min(root.margins.bottom, Math.max(1, root.height - units.iconSizes.smallMedium));

        //Base the left/right margins on height as well, to have a good radial simmetry
        containmentParent.anchors.leftMargin = Math.min(root.margins.left, Math.max(1, root.height - units.iconSizes.smallMedium));

        containmentParent.anchors.leftMargin = Math.min(root.margins.left, Math.max(1, root.height - units.iconSizes.smallMedium));
    }

    onContainmentChanged: {
        print("New panel Containment: " + containment);
        containment.parent = containmentParent;
        containment.visible = true;
        containment.anchors.fill = containmentParent;

        containment.locationChanged.connect(adjustBorders);
        if (containment.Layout) {
            containment.Layout.minimumWidthChanged.connect(minimumWidthChanged);
            containment.Layout.maximumWidthChanged.connect(maximumWidthChanged);
            containment.Layout.preferredWidthChanged.connect(preferredWidthChanged);

            containment.Layout.minimumHeightChanged.connect(minimumHeightChanged);
            containment.Layout.maximumHeightChanged.connect(maximumHeightChanged);
            containment.Layout.preferredHeightChanged.connect(preferredHeightChanged);
        }
        adjustBorders();
    }

    function minimumWidthChanged() {
        if (containment.formFactor === PlasmaCore.Types.Horizontal) {
            panel.width = Math.max(panel.width, containment.Layout.minimumWidth);
        }
    }
    function maximumWidthChanged() {
        if (containment.formFactor === PlasmaCore.Types.Horizontal) {
            panel.width = Math.min(panel.width, containment.Layout.maximumWidth);
        }
    }
    function preferredWidthChanged() {
        if (containment.formFactor === PlasmaCore.Types.Horizontal) {
            panel.width = Math.min(panel.maximumLength, Math.max(containment.Layout.preferredWidth, panel.minimumLength));
        }
    }

    function minimumHeightChanged() {
        if (containment.formFactor === PlasmaCore.Types.Vertical) {
            panel.height = Math.max(panel.height, containment.Layout.minimumWidth);
        }
    }
    function maximumHeightChanged() {
        if (containment.formFactor === PlasmaCore.Types.Vertical) {
            panel.height = Math.min(panel.height, containment.Layout.maximumWidth);
        }
    }
    function preferredHeightChanged() {
        if (containment.formFactor === PlasmaCore.Types.Vertical) {
            panel.height = Math.min(panel.maximumLength, Math.max(containment.Layout.preferredHeight, panel.minimumLength));
        }
    }

    Item {
        id: containmentParent
        anchors {
            fill: parent
            leftMargin: root.margins.left
            topMargin: root.margins.top
            rightMargin: root.margins.right
            bottomMargin: root.margins.bottom
        }
    }

    Connections {
        target: panel
        onXChanged: {
            adjustBorders();
        }
        onYChanged: {
            adjustBorders();
        }
        onWidthChanged: {
            adjustBorders();
        }
        onHeightChanged: {
            adjustBorders();
        }
    }

    Connections {
        target: panel.screen
        onGeometryChanged: {
            adjustBorders();
        }
    }

    Component.onCompleted: {
        print("PanelView QML loaded")
        adjustBorders();
        visible = true
    }
}
