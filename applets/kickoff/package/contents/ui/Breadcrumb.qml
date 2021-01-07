/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>
    Copyright (C) 2021 by Mikel Johnson <mikel5764@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3

Item {
    id: crumbRoot

    height: crumb.implicitHeight
    width: crumb.implicitWidth + crumb.anchors.leftMargin + arrowSvg.width

    property string text
    property bool root: false
    property int depth: model.depth
    signal rootClick()

    function clickCrumb() {
        crumb.clicked();
    }

    PlasmaComponents3.ToolButton {
        id: crumb

        anchors.left: arrowSvg.right
        anchors.leftMargin: crumbRoot.root ? 0 : PlasmaCore.Units.smallSpacing * 2

        text: crumbRoot.text
        enabled: crumbRoot.depth < crumbModel.count

        // We want breadcrumbs large to keep them touch friendly
        implicitWidth: Math.max(PlasmaCore.Units.gridUnit * 6 + PlasmaCore.Units.smallSpacing * 3,
                                implicitContentWidth + leftPadding + rightPadding)
        implicitHeight: PlasmaCore.Units.gridUnit + PlasmaCore.Units.smallSpacing * 3

        onClicked: {
            // Remove all the breadcrumbs in front of the clicked one
            while (crumbModel.count > 0 && crumbRoot.depth < crumbModel.get(crumbModel.count-1).depth) {
                crumbModel.remove(crumbModel.count-1)
                crumbModel.models.pop()
            }

            if (crumbRoot.root) {
                rootClick()
            } else {
                applicationsView.newModel = crumbModel.models[index];
            }
            applicationsView.state = "OutgoingRight";
        }
    }

    PlasmaCore.SvgItem {
        id: arrowSvg

        anchors.left: parent.left
        anchors.verticalCenter: parent.verticalCenter
        height: Math.round(crumbRoot.height / 2)
        width: visible ? height : 0

        svg: PlasmaCore.Svg {
            imagePath: "icons/go"
        }
        elementId: LayoutMirroring.enabled ? "go-previous" : "go-next"
        visible: !crumbRoot.root
    }
} // crumbRoot
