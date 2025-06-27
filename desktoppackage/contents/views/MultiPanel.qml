/*
    SPDX-FileCopyrightText: 2025 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Window
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.ksvg as KSvg
import org.kde.kirigami as Kirigami

import org.kde.plasma.plasmoid 2.0

Item {
    id: root

    readonly property Window view: Window.window
    function updateWindowMask() {
        let rects = [];
        root.children.forEach((element) => {
            if (element instanceof KSvg.FrameSvgItem) {
                rects.push(Qt.rect(element.x, element.y, element.width, element.height));
            }
        });
        Window.window.setMaskFromRectangles(rects);
        updateBlurBehindMask()
    }
    function updateBlurBehindMask() {
        let frameSvgs = [];
        root.children.forEach((item) => {
            if (item instanceof KSvg.FrameSvgItem) {
                frameSvgs.push(item);
            }
        });
        Window.window.setBlurBehindMask(frameSvgs);
    }

    Repeater {
        model: Window.window.containments

        delegate: KSvg.FrameSvgItem {
            id: translucentItem
            visible: floatingness === 0 && panelOpacity !== 1
            imagePath: modelData?.plasmoid?.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "widgets/panel-background"

            width: Math.min(root.width, modelData.Layout.preferredWidth)
            height: 35
            x: Math.round(root.width / 2 - width / 2)
            y: modelData.plasmoid.location == PlasmaCore.Types.BottomEdge
                    ? root.height - height : 0

            onXChanged: root.updateWindowMask()
            onYChanged: root.updateWindowMask()
            onWidthChanged: root.updateWindowMask()
            onHeightChanged: root.updateWindowMask()

            Behavior on width {
                NumberAnimation {
                    id: widthAnimation
                    duration: Kirigami.Units.longDuration
                    // Should be InOutQuad but taskmanager has this, and they look a bit more synced
                    easing.type: Easing.OutQuad
                }
            }

            //color: "red"
            Component.onCompleted: {
                modelData.parent = this;
                modelData.anchors.fill = this;
            }

            KSvg.FrameSvgItem {
                anchors {
                    fill: parent
                    leftMargin: -margins.left
                    topMargin: -margins.top
                    rightMargin: -margins.right
                    bottomMargin: -margins.bottom
                }
                visible: floatingness === 0 && panelOpacity !== 1
                imagePath: modelData?.plasmoid?.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "widgets/panel-background"
                prefix: "shadow"
            }
        }
    }
}
