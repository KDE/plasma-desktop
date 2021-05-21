/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.configuration 2.0
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons


PlasmaCore.FrameSvgItem {
    id: root

    //Those properties get updated by PanelConfiguration.qml whenever a value in the panel changes
    property alias offset: offsetHandle.value
    property alias minimumLength: minimumLengthHandle.value
    property alias maximumLength: maximumLengthHandle.value

    imagePath: "widgets/containment-controls"
    state: "BottomEdge"
    implicitWidth: offsetHandle.width + minimumLengthHandle.width
    implicitHeight: offsetHandle.height + minimumLengthHandle.height

    onMinimumLengthChanged: leftMinimumLengthHandle.value = minimumLength
    onMaximumLengthChanged: leftMaximumLengthHandle.value = maximumLength

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
        minimumLengthHandle.value = panel.minimumLength
        maximumLengthHandle.value = panel.maximumLength
        leftMinimumLengthHandle.value = panel.minimumLength
        leftMaximumLengthHandle.value = panel.maximumLength
    }

    PlasmaCore.Svg {
        id: containmentControlsSvg
        imagePath: "widgets/containment-controls"
    }
    PlasmaCore.SvgItem {
        id: centerMark
        svg: containmentControlsSvg
        elementId: dialogRoot.vertical ? "vertical-centerindicator" : "horizontal-centerindicator"
        visible: panel.alignment === Qt.AlignCenter
        width: dialogRoot.vertical ? parent.width : naturalSize.width
        height: dialogRoot.vertical ? naturalSize.height : parent.height
        anchors.centerIn: parent
    }

    SliderHandle {
        id: offsetHandle
        graphicElementName: "offsetslider"
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
                    return panel.maximumLength / 2 - size / 2
            }
        }
        //Needed for the same reason as above
        maximumPosition: {
            var size = dialogRoot.vertical ? height : width
            var dialogRootSize = dialogRoot.vertical ? dialogRoot.height : dialogRoot.width
            switch(panel.alignment){
            case Qt.AlignLeft:
                    return dialogRootSize - maximumLengthHandle.value - size / 2
            case Qt.AlignRight:
                    return dialogRootSize - size / 2
            default:
                    return dialogRootSize - panel.maximumLength / 2 - size / 2
            }
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
        id: minimumLengthHandle
        alignment: panel.alignment | Qt.AlignLeft
        visible: panel.alignment !== Qt.AlignRight
        offset: panel.offset
        graphicElementName: "minslider"
        onValueChanged: panel.minimumLength = value
        minimumPosition: offsetHandle.position + PlasmaCore.Units.gridUnit * 3
        maximumPosition: {
            var dialogRootSize = dialogRoot.vertical ? dialogRoot.height : dialogRoot.width
            var size = dialogRoot.vertical ? height : width
            panel.alignment === Qt.AlignCenter ? Math.min(dialogRootSize - size/2, dialogRootSize + offset * 2 - size/2) : dialogRootSize - size/2
        }
    }

    SliderHandle {
        id: maximumLengthHandle
        alignment: panel.alignment | Qt.AlignLeft
        visible: panel.alignment !== Qt.AlignRight
        offset: panel.offset
        graphicElementName: "maxslider"
        onValueChanged: panel.maximumLength = value
        minimumPosition: offsetHandle.position + PlasmaCore.Units.gridUnit * 3
        maximumPosition: {
            var dialogRootSize = dialogRoot.vertical ? dialogRoot.height : dialogRoot.width
            var size = dialogRoot.vertical ? height : width
            panel.alignment === Qt.AlignCenter ? Math.min(dialogRootSize - size/2, dialogRootSize + offset * 2 - size/2) : dialogRootSize - size/2
        }
    }
    SliderHandle {
        id: leftMinimumLengthHandle
        alignment: panel.alignment | Qt.AlignRight
        visible: panel.alignment !== Qt.AlignLeft
        offset: panel.offset
        graphicElementName: "minslider"
        onValueChanged: panel.minimumLength = value
        maximumPosition: offsetHandle.position - PlasmaCore.Units.gridUnit * 3
        minimumPosition: {
            var size = dialogRoot.vertical ? height : width
            panel.alignment === Qt.AlignCenter ? Math.max(-size/2, offset*2 - size/2) : -size/2
        }
    }
    SliderHandle {
        id: leftMaximumLengthHandle
        alignment: panel.alignment | Qt.AlignRight
        visible: panel.alignment !== Qt.AlignLeft
        offset: panel.offset
        graphicElementName: "maxslider"
        onValueChanged: panel.maximumLength = value
        maximumPosition: offsetHandle.position - PlasmaCore.Units.gridUnit * 3
        minimumPosition: {
            var size = dialogRoot.vertical ? height : width
            panel.alignment === Qt.AlignCenter ? Math.max(-size/2, offset*2 - size/2) : -size/2
        }
    }

    states: [
        State {
            name: "TopEdge"
            PropertyChanges {
                target: root
                prefix: "north"
                height: root.implicitHeight
            }
            AnchorChanges {
                target: root
                anchors {
                    top: root.parent.top
                    bottom: undefined
                    left: root.parent.left
                    right: root.parent.right
                }
            }
            AnchorChanges {
                target: offsetHandle
                anchors {
                    top: undefined
                    bottom: root.bottom
                    left: undefined
                    right: undefined
                }
            }
            AnchorChanges {
                target: minimumLengthHandle
                anchors {
                    top: root.top
                    bottom: undefined
                    left: undefined
                    right: undefined
                }
            }
            AnchorChanges {
                target: maximumLengthHandle
                anchors {
                    top: undefined
                    bottom: root.bottom
                    left: undefined
                    right: undefined
                }
            }
            AnchorChanges {
                target: leftMinimumLengthHandle
                anchors {
                    top: root.top
                    bottom: undefined
                    left: undefined
                    right: undefined
                }
            }
            AnchorChanges {
                target: leftMaximumLengthHandle
                anchors {
                    top: undefined
                    bottom: root.bottom
                    left: undefined
                    right: undefined
                }
            }
        },
        State {
            name: "BottomEdge"
            PropertyChanges {
                target: root
                prefix: "south"
                height: root.implicitHeight
            }
            AnchorChanges {
                target: root
                anchors {
                    top: undefined
                    bottom: root.parent.bottom
                    left: root.parent.left
                    right: root.parent.right
                }
            }
            AnchorChanges {
                target: offsetHandle
                anchors {
                    top: root.top
                    bottom: undefined
                    left: undefined
                    right: undefined
                }
            }
            AnchorChanges {
                target: minimumLengthHandle
                anchors {
                    top: undefined
                    bottom: root.bottom
                    left: undefined
                    right: undefined
                }
            }
            AnchorChanges {
                target: maximumLengthHandle
                anchors {
                    top: root.top
                    bottom: undefined
                    left: undefined
                    right: undefined
                }
            }
            AnchorChanges {
                target: leftMinimumLengthHandle
                anchors {
                    top: undefined
                    bottom: root.bottom
                    left: undefined
                    right: undefined
                }
            }
            AnchorChanges {
                target: leftMaximumLengthHandle
                anchors {
                    top: root.top
                    bottom: undefined
                    left: undefined
                    right: undefined
                }
            }
        },
        State {
            name: "LeftEdge"
            PropertyChanges {
                target: root
                prefix: "west"
                width: root.implicitWidth
            }
            AnchorChanges {
                target: root
                anchors {
                    top: root.parent.top
                    bottom: root.parent.bottom
                    left: root.parent.left
                    right: undefined
                }
            }
            AnchorChanges {
                target: offsetHandle
                anchors {
                    top: undefined
                    bottom: undefined
                    left: undefined
                    right: root.right
                }
            }
            AnchorChanges {
                target: minimumLengthHandle
                anchors {
                    top: undefined
                    bottom: undefined
                    left: root.left
                    right: undefined
                }
            }
            AnchorChanges {
                target: maximumLengthHandle
                anchors {
                    top: undefined
                    bottom: undefined
                    left: undefined
                    right: root.right
                }
            }
            AnchorChanges {
                target: leftMinimumLengthHandle
                anchors {
                    top: undefined
                    bottom: undefined
                    left: root.left
                    right: undefined
                }
            }
            AnchorChanges {
                target: leftMaximumLengthHandle
                anchors {
                    top: undefined
                    bottom: undefined
                    left: undefined
                    right: root.right
                }
            }
        },
        State {
            name: "RightEdge"
            PropertyChanges {
                target: root
                prefix: "east"
                width: root.implicitWidth
            }
            AnchorChanges {
                target: root
                anchors {
                    top: root.parent.top
                    bottom: root.parent.bottom
                    left: undefined
                    right: root.parent.right
                }
            }
            AnchorChanges {
                target: offsetHandle
                anchors {
                    top: undefined
                    bottom: undefined
                    left: parent.left
                    right: undefined
                }
            }
            AnchorChanges {
                target: minimumLengthHandle
                anchors {
                    top: undefined
                    bottom: undefined
                    left: undefined
                    right: parent.right
                }
            }
            AnchorChanges {
                target: maximumLengthHandle
                anchors {
                    top: undefined
                    bottom: undefined
                    left: parent.left
                    right: undefined
                }
            }
            AnchorChanges {
                target: leftMinimumLengthHandle
                anchors {
                    top: undefined
                    bottom: undefined
                    left: undefined
                    right: parent.right
                }
            }
            AnchorChanges {
                target: leftMaximumLengthHandle
                anchors {
                    top: undefined
                    bottom: undefined
                    left: parent.left
                    right: undefined
                }
            }
        }
    ]
}
