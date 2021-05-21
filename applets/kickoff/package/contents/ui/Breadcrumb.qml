/*
    SPDX-FileCopyrightText: 2011 Martin Gräßlin <mgraesslin@kde.org>
    SPDX-FileCopyrightText: 2021 Mikel Johnson <mikel5764@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
