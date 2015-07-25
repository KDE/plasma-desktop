/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
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
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.configuration 2.0
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons


PlasmaCore.SvgItem {
    id: root
    svg: containmentControlsSvg
    state: parent.state
    width: naturalSize.width
    height: naturalSize.height

    //value expressed by this slider
    property int value
    //name of the graphics to load
    property string graphicElementName
    //where the point "0" is
    property int offset: 0
    //handle type: behave in different ways based on the alignment
    property int alignment: panel.alignment

    property int minimumValue: (dialogRoot.vertical) ? -root.height/2 : -root.width/2
    property int maximumValue: (dialogRoot.vertical) ? root.parent.height - root.height/2+1 : root.parent.width - root.width/2+1

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
        onWidthChanged: syncPos()
        onHeightChanged: syncPos()
    }

    MouseArea {
        drag {
            target: parent
            axis: (dialogRoot.vertical) ? Drag.YAxis : Drag.XAxis
            minimumX: root.minimumValue
            minimumY: root.minimumValue
            maximumX: root.maximumValue
            maximumY: root.maximumValue
        }
        anchors {
            fill: parent
            leftMargin: (dialogRoot.vertical) ? 0 : -units.gridUnit
            rightMargin: (dialogRoot.vertical) ? 0 : -units.gridUnit
            topMargin: (dialogRoot.vertical) ? -units.gridUnit : 0
            bottomMargin: (dialogRoot.vertical) ? -units.gridUnit : 0
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
