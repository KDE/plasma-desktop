/*
    Copyright (C) 2011  Martin Gräßlin <mgraesslin@kde.org>

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
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: crumbRoot

    height: crumb.implicitHeight
    width: crumb.implicitWidth + arrowSvg.width

    property string text
    property bool root: false
    property int depth: model.depth

    function clickCrumb() {
        crumb.clicked();
    }

    PlasmaComponents.ToolButton {
        id: crumb

        anchors.left: arrowSvg.right

        text: crumbRoot.text
        enabled: crumbRoot.depth < crumbModel.count

        onClicked: {
            // Remove all the breadcrumbs in front of the clicked one
            while (crumbModel.count > 0 && crumbRoot.depth < crumbModel.get(crumbModel.count-1).depth) {
                crumbModel.remove(crumbModel.count-1)
                crumbModel.models.pop()
            }

            if (crumbRoot.root) {
                applicationsView.newModel = rootModel;
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
        height: crumbRoot.height / 2
        width: visible ? height : 0

        svg: PlasmaCore.Svg {
            imagePath: "icons/go"
        }
        elementId: LayoutMirroring.enabled ? "go-previous" : "go-next"
        visible: !crumbRoot.root
    }
} // crumbRoot
