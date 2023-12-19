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

    SliderHandle {
        id: offsetHandle
        anchors {
            right: !root.isHorizontal ? root.right : undefined
            bottom: root.isHorizontal ? root.bottom : undefined
        }
        // On wayland the center handle can't work for technical reasons, hide it everywhere
        visible: panel.alignment !== Qt.AlignCenter
        graphicElementName: "offsetslider"
        description: i18nd("plasma_shell_org.kde.plasma.desktop", "Drag to change position on this screen edge.\nDouble click to reset.")
        onValueChanged: panel.offset = value
        property int position: (dialogRoot.vertical) ? y : x
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
                return -size / 2
            case Qt.AlignRight:
                return leftMaximumLengthHandle.value - size / 2
            default:
                return 0
            }
        }
        //Needed for the same reason as above
        maximumPosition: {
            var size = dialogRoot.vertical ? height : width
            var rootSize = dialogRoot.vertical ? root.height : root.width
            switch(panel.alignment){
            case Qt.AlignLeft:
                return rootSize - rightMaximumLengthHandle.value - size / 2
            case Qt.AlignRight:
                return rootSize - size / 2
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
