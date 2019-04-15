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

    imagePath: containment && containment.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "widgets/panel-background"

    enabledBorders: panel.enabledBorders

    property Item containment

    property alias panelMask: root.mask

    readonly property bool verticalPanel: containment && containment.formFactor === PlasmaCore.Types.Vertical

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
            return;
        }
        prefix = [pre, ""];
    }

    onContainmentChanged: {
        if (!containment) {
            return;
        }
        containment.parent = containmentParent;
        containment.visible = true;
        containment.anchors.fill = containmentParent;
        containment.locationChanged.connect(adjustPrefix);
        adjustPrefix();
    }

    Binding {
        target: panel
        property: "length"
        when: containment
        value: {
            if (!containment) {
                return;
            }
            if (verticalPanel) {
                return containment.Layout.preferredHeight
            } else {
                return containment.Layout.preferredWidth
            }
        }
    }

    Binding {
        target: panel
        property: "backgroundHints"
        when: containment
        value: {
            if (!containment) {
                return;
            }

            return containment.backgroundHints; 
        }
    }    

    Item {
        id: containmentParent
        anchors {
            fill: parent

            //Margins are either the size of the margins in the SVG, unless that prevents the panel from being at least half a smallMedium icon + smallSpace) tall at which point we set the margin to whatever allows it to be that...or if it still won't fit, 1.

            //the size a margin should be to force a panel to be the required size above
            readonly property real spacingAtMinSize: Math.max(1, (verticalPanel ? root.width : root.height) - units.iconSizes.smallMedium - units.smallSpacing*2)/2

            topMargin: Math.round(Math.min(root.fixedMargins.top, spacingAtMinSize));
            bottomMargin: Math.round(Math.min(root.fixedMargins.bottom, spacingAtMinSize));

            //Base the left/right fixedMargins on height as well, to have a good radial symmetry
            leftMargin: Math.round(Math.min(root.fixedMargins.left, spacingAtMinSize));
            rightMargin: Math.round(Math.min(root.fixedMargins.right, spacingAtMinSize));
        }
    }
}
