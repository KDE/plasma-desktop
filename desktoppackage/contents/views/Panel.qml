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

    imagePath: "widgets/panel-background"
    onPrefixChanged: adjustBorders();
    onRepaintNeeded: adjustPrefix();

    visible: false //adjust borders is run during setup. We want to avoid painting till completed

    property Item containment

    property bool veticalPanel: containment && containment.formFactor === PlasmaCore.Types.Vertical

    function adjustPrefix() {
        if (!containment) {
            return "";
        }
        var pre;
        switch (containment.location) {
        case PlasmaCore.Types.LeftEdge:
            pre = "west";
            break;
        case PlasmaCore.Types.TopEdge:
            pre = "north";
            break;
        case PlasmaCore.Types.RightEdge:
            pre = "east";
            break;
        case PlasmaCore.Types.BottomEdge:
            pre = "south";
            break;
        default:
            prefix = "";
        }
        if (hasElementPrefix(pre)) {
            prefix = pre;
        } else {
            prefix = "";
        }
    }

    function adjustBorders() {
        var borders = PlasmaCore.FrameSvg.AllBorders;
        if (!containment) {
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
    }

    onContainmentChanged: {
        if (!containment) {
            return;
        }
        containment.parent = containmentParent;
        containment.visible = true;
        containment.anchors.fill = containmentParent;

        containment.locationChanged.connect(adjustBorders);
        adjustBorders();
    }

    Binding {
        target: panel
        property: "length"
        value: containment.formFactor == PlasmaCore.Types.Vertical ?
                        panel.length = containment.Layout.preferredHeight : panel.length = containment.Layout.preferredWidth
    }


    Item {
        id: containmentParent
        anchors {
            fill: parent
            topMargin: Math.round(Math.min(root.fixedMargins.top, Math.max(1, (veticalPanel ? root.width : root.height) - units.iconSizes.smallMedium - units.smallSpacing*2)/2));
            bottomMargin: Math.round(Math.min(root.fixedMargins.bottom, Math.max(1, (veticalPanel ? root.width : root.height) - units.iconSizes.smallMedium - units.smallSpacing*2)/2));

            //Base the left/right fixedMargins on height as well, to have a good radial symmetry
            leftMargin: Math.round(Math.min(root.fixedMargins.left, Math.max(1, (veticalPanel ? root.width : root.height) - units.iconSizes.smallMedium - units.smallSpacing*2)/2));
            rightMargin: Math.round(Math.min(root.fixedMargins.right, Math.max(1, (veticalPanel ? root.width : root.height) - units.iconSizes.smallMedium - units.smallSpacing*2)/2));
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

    Connections {
        target: containment
        onUserConfiguringChanged: {
            if (!containment.userConfiguring) {
                containment.Layout.minimumWidthChanged();
                containment.Layout.maximumWidthChanged();
                containment.Layout.preferredWidthChanged();
                containment.Layout.minimumHeightChanged();
                containment.Layout.maximumHeightChanged();
                containment.Layout.preferredHeightChanged();
            }
        }
    }

    Component.onCompleted: {
        adjustBorders();
        visible = true
    }
}
