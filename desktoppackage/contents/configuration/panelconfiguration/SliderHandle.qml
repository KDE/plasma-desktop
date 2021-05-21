/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.configuration 2.0
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons


PlasmaCore.SvgItem {
    id: root

    //Those properties get updated by PanelConfiguration.qml whenever a value changes
    svg: containmentControlsSvg
    state: parent.state
    width: naturalSize.width
    height: naturalSize.height

    //value expressed by this slider, this is the distance to offset
    property int value

    //name of the graphics to load
    property string graphicElementName

    //where the point "0" is
    property int offset: 0

    /*handle type: behave in different ways based on the alignment:
     * alignment == Qt.AlignRight: Panel aligned to right and handle value relative to the right
     * alignment == Qt.AlignLeft: Panel aligned to left and handle relative to the left
     * (alignment != Qt.AlignRight) && (alignment & Qt.AlignRight): Panel aligned to the center and handle right of offset and value doubled
     * (alignment != Qt.AlignLeft) && (alignment & Qt.AlignLeft): Panel aligned to the center and handle left of offset and value doubled
     * else: Panel aligned to center and handle relative to the center
     * Note that right/left and top/bottom are interchangeable
     */
    property int alignment: panel.alignment

    //The maximum/minimum Position (X/Y) the silder can be moved to
    property int minimumPosition
    property int maximumPosition

    function syncPos() {
        if (dialogRoot.vertical) {
            if (alignment == Qt.AlignRight) {
                y = root.parent.height - (value + offset + root.height/2)
            } else if (alignment == Qt.AlignLeft) {
                y = value + offset - root.height/2
            } else {
                if (root.alignment & Qt.AlignRight) {
                    y = root.parent.height/2 - value/2 + offset - root.height/2
                } else if (root.alignment & Qt.AlignLeft) {
                    y = root.parent.height/2 + value/2 + offset - root.height/2
                } else {
                    y = root.parent.height/2 + value + offset -root.height/2
                }
            }
        } else {
            if (alignment == Qt.AlignRight) {
                x = root.parent.width - (value + offset + root.width/2)
            } else if (alignment == Qt.AlignLeft) {
                x = value + offset - root.width/2
            } else {
                if (root.alignment & Qt.AlignRight) {
                    x = root.parent.width/2 - value/2 + offset - root.width/2
                } else if (root.alignment & Qt.AlignLeft) {
                    x = root.parent.width/2 + value/2 + offset -root.width/2
                } else {
                    x = root.parent.width/2 + value + offset -root.width/2
                }
            }
        }
    }
    onValueChanged: syncPos()
    onOffsetChanged: syncPos()
    onAlignmentChanged: syncPos()
    Connections {
        target: root.parent
        function onWidthChanged() {
            syncPos()
        }
        function onHeightChanged() {
            syncPos()
        }
    }

    MouseArea {
        drag {
            target: parent
            axis: (dialogRoot.vertical) ? Drag.YAxis : Drag.XAxis
            minimumX: root.minimumPosition
            minimumY: root.minimumPosition
            maximumX: root.maximumPosition
            maximumY: root.maximumPosition
        }
        anchors {
            fill: parent
            leftMargin: (dialogRoot.vertical) ? 0 : -PlasmaCore.Units.gridUnit
            rightMargin: (dialogRoot.vertical) ? 0 : -PlasmaCore.Units.gridUnit
            topMargin: (dialogRoot.vertical) ? -PlasmaCore.Units.gridUnit : 0
            bottomMargin: (dialogRoot.vertical) ? -PlasmaCore.Units.gridUnit : 0
        }
        cursorShape: dialogRoot.vertical ? Qt.SizeVerCursor : Qt.SizeHorCursor
        onPositionChanged: {
            if (dialogRoot.vertical) {
                if (root.alignment == Qt.AlignRight) {
                    root.value = root.parent.height - (parent.y + offset + root.height/2)
                } else if (alignment == Qt.AlignLeft) {
                    root.value = parent.y - offset + root.height/2
                //Center
                } else {
                    if (root.alignment & Qt.AlignRight) {
                        root.value = (root.parent.height/2 - parent.y + offset)*2  - root.height
                    } else if (root.alignment & Qt.AlignLeft) {
                        root.value = (parent.y - offset - root.parent.height/2)*2  + root.height
                    } else {
                        var value = parent.y - root.parent.height/2 - offset + root.height/2
                        //Snap
                        if (Math.abs(value) < 5) {
                            root.value = 0
                        } else {
                            root.value = value
                        }
                    }
                }
            } else {
                if (root.alignment == Qt.AlignRight) {
                    root.value = root.parent.width - (parent.x + offset + root.width/2)
                } else if (alignment == Qt.AlignLeft) {
                    root.value = parent.x - offset + root.width/2
                //Center
                } else {
                    if (root.alignment & Qt.AlignRight) {
                        root.value = (root.parent.width/2 - parent.x + offset)*2 - root.width
                    } else if (root.alignment & Qt.AlignLeft) {
                        root.value = (parent.x - offset - root.parent.width/2)*2  + root.width
                    } else {
                        var value = parent.x - root.parent.width/2 - offset + root.width/2
                        //Snap
                        if (Math.abs(value) < 5) {
                            root.value = 0
                        } else {
                            root.value = value
                        }
                    }
                }
            }
        }
    }

    states: [
        State {
            name: "TopEdge"
            PropertyChanges {
                target: root
                elementId: "north-" + graphicElementName
            }
        },
        State {
            name: "BottomEdge"
            PropertyChanges {
                target: root
                elementId: "south-" + graphicElementName
            }
        },
        State {
            name: "LeftEdge"
            PropertyChanges {
                target: root
                elementId: "west-" + graphicElementName
            }
        },
        State {
            name: "RightEdge"
            PropertyChanges {
                target: root
                elementId: "east-" + graphicElementName
            }
        }
    ]
}
