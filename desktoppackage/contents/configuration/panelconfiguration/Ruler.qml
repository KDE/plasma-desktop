/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtQuick.Controls 2.4 as QQC2
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.components 3.0 as PC3
import org.kde.plasma.shell.panel 0.1 as Panel
import org.kde.kquickcontrols 2.0

KSvg.FrameSvgItem {
    id: root

    anchors.fill: parent

    //Those properties get updated by PanelConfiguration.qml whenever a value in the panel changes
    property alias offset: offsetHandle.value
    property alias minimumLength: rightMinimumLengthHandle.value
    property alias maximumLength: rightMaximumLengthHandle.value
    property bool isHorizontal: root.prefix[0] === 'north' || root.prefix[0] === 'south'

    property string maximumText: (dialogRoot.vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Drag to change maximum height.") : i18nd("plasma_shell_org.kde.plasma.desktop", "Drag to change maximum width.")) + "\n" + i18nd("plasma_shell_org.kde.plasma.desktop", "Double click to reset.")
    property string minimumText: (dialogRoot.vertical ? i18nd("plasma_shell_org.kde.plasma.desktop", "Drag to change minimum height.") : i18nd("plasma_shell_org.kde.plasma.desktop", "Drag to change minimum width.")) + "\n" + i18nd("plasma_shell_org.kde.plasma.desktop", "Double click to reset.")

    imagePath: "widgets/containment-controls"
    implicitWidth: Math.max(offsetHandle.width, rightMinimumLengthHandle.width + rightMaximumLengthHandle.width)
    implicitHeight: Math.max(offsetHandle.height, rightMinimumLengthHandle.height + rightMaximumLengthHandle.height)

    onMinimumLengthChanged: rightMinimumLengthHandle.value = leftMinimumLengthHandle.value = minimumLength
    onMaximumLengthChanged: rightMaximumLengthHandle.value = leftMaximumLengthHandle.value = maximumLength

    /* As offset and length have a different meaning in all alignments, the panel shifts on alignment change.
     * This could result in wrong panel positions (e.g. panel shifted over monitor border).
     * The fancy version would be a recalculation of all values, so that the panel stays at it's current position,
     * but this would be error prone and complicated. As the panel alignment is rarely changed, it's not worth it.
     * The more easy approach is just setting the panel offset to zero. This makes sure the panel has a valid position and size.
     */
    Connections {
        target: panel
        function onAlignmentChanged() {
            offset = 0
        }
    }

    Component.onCompleted: {
        offsetHandle.value = panel.offset
        rightMinimumLengthHandle.value = panel.minimumLength
        rightMaximumLengthHandle.value = panel.maximumLength
        leftMinimumLengthHandle.value = panel.minimumLength
        leftMaximumLengthHandle.value = panel.maximumLength
    }

    KSvg.SvgItem {
        id: centerMark
        imagePath: "widgets/containment-controls"
        elementId: dialogRoot.vertical ? "vertical-centerindicator" : "horizontal-centerindicator"
        visible: panel.alignment === Qt.AlignCenter
        width: dialogRoot.vertical ? parent.width : naturalSize.width
        height: dialogRoot.vertical ? naturalSize.height : parent.height
        anchors.centerIn: parent
    }

    SliderHandle {
        id: offsetHandle
        anchors {
            right: !root.isHorizontal ? root.right : undefined
            bottom: root.isHorizontal ? root.bottom : undefined
        }
        graphicElementName: "offsetslider"
        description: i18nd("plasma_shell_org.kde.plasma.desktop", "Drag to change position on this screen edge.\nDouble click to reset.")
        offset: panel.alignment === Qt.AlignCenter ? 0 : (dialogRoot.vertical ? panel.height : panel.width) / 2
        property int position: (dialogRoot.vertical) ? y + height / 2 : x + width / 2
        onPositionChanged: {
            if (!offsetHandle.hasEverBeenMoved) return;
            let panelLength = dialogRoot.vertical ? panel.height : panel.width
            let rootLength = dialogRoot.vertical ? root.height : root.width
            // Snap at the center
            if (Math.abs(position - rootLength / 2) < 5) {
                if (panel.alignment !== Qt.AlignCenter) {
                    panel.alignment = Qt.AlignCenter
                    // Coordinate change: since we switch from measuring the min/max
                    // length from the side of the panel to the center of the panel,
                    // we need to double the distance between the min/max indicators
                    // and the panel side.
                    panel.minimumLength += panel.minimumLength - panelLength
                    panel.maximumLength += panel.maximumLength - panelLength
                }
                panel.offset = 0
            } else if (position > rootLength / 2) {
                if (panel.alignment === Qt.AlignCenter) {
                    // This is the opposite of the previous comment, as we are
                    // cutting in half the distance between the min/max indicators
                    // and the side of the panel.
                    panel.minimumLength -= (panel.minimumLength - panelLength) / 2
                    panel.maximumLength -= (panel.maximumLength - panelLength) / 2
                }
                panel.alignment = Qt.AlignRight
                panel.offset = Math.round(rootLength - position - offset)
            } else if (position <= rootLength / 2) {
                if (panel.alignment === Qt.AlignCenter) {
                    panel.minimumLength -= (panel.minimumLength - panelLength) / 2
                    panel.maximumLength -= (panel.maximumLength - panelLength) / 2
                }
                panel.alignment = Qt.AlignLeft
                panel.offset = Math.round(position - offset)
            }
        }
        /* The maximum/minimumPosition values are needed to prevent the user from moving a panel with
         * center alignment to the left and then drag the position handle to the left.
         * This would make the panel to go off the monitor:
         * |<-        V        ->                         |
         * |  ->      |      <-                           |
         *            ^move this slider to the left
         */
        minimumPosition: {
            var size = dialogRoot.vertical ? height : width
            switch(panel.alignment){
            case Qt.AlignLeft:
                return -size / 2 + offset
            case Qt.AlignRight:
                return leftMaximumLengthHandle.value - size / 2 - offset
            default:
                return panel.maximumLength / 2 - size / 2
            }
        }
        //Needed for the same reason as above
        maximumPosition: {
            var size = dialogRoot.vertical ? height : width
            var rootSize = dialogRoot.vertical ? root.height : root.width
            switch(panel.alignment){
            case Qt.AlignLeft:
                return rootSize - rightMaximumLengthHandle.value - size / 2 + offset
            case Qt.AlignRight:
                return rootSize - size / 2 - offset
            default:
                return rootSize - panel.maximumLength / 2 - size / 2
            }
        }
        function defaultPosition(): int /*override*/ {
            return 0;
        }
    }

    /* The maximumPosition value for the right handles and the minimumPosition value for the left handles are
     * needed to prevent the user from moving a panel with center alignment to the left (right) and then pull one of the
     * right (left) sliders to the right (left).
     * Because the left and right sliders are coupled, this would make the left (right) sliders to go off the monitor.
     *
     * |<-        V        ->                         |
     * |   ->     |     <-                            |
     *                      ^move this slider to the right
     *
     * The other max/min Position values just set a minimum panel size
     */

    SliderHandle {
        id: rightMinimumLengthHandle
        anchors {
            left: !root.isHorizontal ? root.left : undefined
            top: root.isHorizontal ? root.top : undefined
        }
        description: root.minimumText
        alignment: panel.alignment | Qt.AlignLeft
        visible: panel.alignment !== Qt.AlignRight
        offset: panel.offset
        graphicElementName: "minslider"
        onValueChanged: panel.minimumLength = value
        minimumPosition: offsetHandle.position + Kirigami.Units.gridUnit * 3
        maximumPosition: {
            var rootSize = dialogRoot.vertical ? root.height : root.width
            var size = dialogRoot.vertical ? height : width
            panel.alignment === Qt.AlignCenter ? Math.min(rootSize - size/2, rootSize + offset * 2 - size/2) : rootSize - size/2
        }
    }

    SliderHandle {
        id: rightMaximumLengthHandle
        anchors {
            right: !root.isHorizontal ? root.right : undefined
            bottom: root.isHorizontal ? root.bottom : undefined
        }
        description: root.maximumText
        alignment: panel.alignment | Qt.AlignLeft
        visible: panel.alignment !== Qt.AlignRight
        offset: panel.offset
        graphicElementName: "maxslider"
        onValueChanged: panel.maximumLength = value
        minimumPosition: offsetHandle.position + Kirigami.Units.gridUnit * 3
        maximumPosition: {
            var rootSize = dialogRoot.vertical ? root.height : root.width
            var size = dialogRoot.vertical ? height : width
            panel.alignment === Qt.AlignCenter ? Math.min(rootSize - size/2, rootSize + offset * 2 - size/2) : rootSize - size/2
        }
    }

    SliderHandle {
        id: leftMinimumLengthHandle
        anchors {
            left: !root.isHorizontal ? root.left : undefined
            top: root.isHorizontal ? root.top : undefined
        }
        description: root.minimumText
        alignment: panel.alignment | Qt.AlignRight
        visible: panel.alignment !== Qt.AlignLeft
        offset: panel.offset
        graphicElementName: "maxslider"
        onValueChanged: panel.minimumLength = value
        maximumPosition: offsetHandle.position - Kirigami.Units.gridUnit * 3
        minimumPosition: {
            var size = dialogRoot.vertical ? height : width
            panel.alignment === Qt.AlignCenter ? Math.max(-size/2, offset*2 - size/2) : -size/2
        }
    }

    SliderHandle {
        id: leftMaximumLengthHandle
        anchors {
            right: !root.isHorizontal ? root.right : undefined
            bottom: root.isHorizontal ? root.bottom : undefined
        }
        description: root.maximumText
        alignment: panel.alignment | Qt.AlignRight
        visible: panel.alignment !== Qt.AlignLeft
        offset: panel.offset
        graphicElementName: "minslider"
        onValueChanged: panel.maximumLength = value
        maximumPosition: offsetHandle.position - Kirigami.Units.gridUnit * 3
        minimumPosition: {
            var size = dialogRoot.vertical ? height : width
            panel.alignment === Qt.AlignCenter ? Math.max(-size/2, offset*2 - size/2) : -size/2
        }
    }
}
