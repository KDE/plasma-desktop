/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.configuration 2.0
import org.kde.plasma.extras 2.0 as PlasmaExtras


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
     * alignment === Qt.AlignRight: Panel aligned to right and handle value relative to the right
     * alignment === Qt.AlignLeft: Panel aligned to left and handle relative to the left
     * (alignment !== Qt.AlignRight) && (alignment & Qt.AlignRight): Panel aligned to the center and handle right of offset and value doubled
     * (alignment !== Qt.AlignLeft) && (alignment & Qt.AlignLeft): Panel aligned to the center and handle left of offset and value doubled
     * else: Panel aligned to center and handle relative to the center
     * Note that right/left and top/bottom are interchangeable
     */
    property int alignment: panel.alignment

    //The maximum/minimum Position (X/Y) the silder can be moved to
    property int minimumPosition
    property int maximumPosition
    //Provide default position for "reset" action.
    function defaultPosition(): int {
        var dialogSize, panelSize;
        if (dialogRoot.vertical) {
            dialogSize = dialogRoot.height;
            panelSize = panel.height;
        } else {
            dialogSize = dialogRoot.width;
            panelSize = panel.width;
        }
        return (value === panelSize) ? dialogSize : panelSize;
    }

    // Handle name displayed as a tooltip.
    property string description

    function syncPos() {
        if (dialogRoot.vertical) {
            if (alignment === Qt.AlignRight) {
                y = root.parent.height - (value + offset + root.height/2)
            } else if (alignment === Qt.AlignLeft) {
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
            if (alignment === Qt.AlignRight) {
                x = root.parent.width - (value + offset + root.width/2)
            } else if (alignment === Qt.AlignLeft) {
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

    PlasmaComponents3.ToolTip {
        text: root.description
        visible: root.description !== "" && ((area.containsMouse && !area.containsPress) || area.activeFocus)
    }

    MouseArea {
        id: area
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

        readonly property int keyboardMoveStepSize: Math.ceil((root.maximumPosition - root.minimumPosition) / 20)

        activeFocusOnTab: true
        hoverEnabled: true
        cursorShape: dialogRoot.vertical ? Qt.SizeVerCursor : Qt.SizeHorCursor

        Accessible.description: root.description

        Keys.onEnterPressed: doubleClicked(null);
        Keys.onReturnPressed: doubleClicked(null);
        Keys.onSpacePressed: doubleClicked(null);

        // BEGIN Arrow keys
        Keys.onUpPressed: if (dialogRoot.vertical) {
            root.y = Math.max(root.minimumPosition, root.y - ((event.modifiers & Qt.ShiftModifier) ? 1 : keyboardMoveStepSize));
            changePosition();
        } else {
            event.accepted = false;
        }
        Keys.onDownPressed: if (dialogRoot.vertical) {
            root.y = Math.min(root.maximumPosition, root.y + ((event.modifiers & Qt.ShiftModifier) ? 1 : keyboardMoveStepSize));
            changePosition();
        } else {
            event.accepted = false;
        }
        Keys.onLeftPressed: if (!dialogRoot.vertical) {
            root.x = Math.max(root.minimumPosition, root.x - ((event.modifiers & Qt.ShiftModifier) ? 1 : keyboardMoveStepSize));
            changePosition();
        } else {
            event.accepted = false;
        }
        Keys.onRightPressed: if (!dialogRoot.vertical) {
            root.x = Math.min(root.maximumPosition, root.x + ((event.modifiers & Qt.ShiftModifier) ? 1 : keyboardMoveStepSize));
            changePosition();
        } else {
            event.accepted = false;
        }
        // END Arrow keys

        onPositionChanged: {
            if (!drag.active) {
                return;
            }
            changePosition();
        }
        onDoubleClicked: {
            root.value = root.defaultPosition();
        }

        function changePosition() {
            if (dialogRoot.vertical) {
                if (root.alignment === Qt.AlignRight) {
                    root.value = root.parent.height - (root.y + offset + root.height/2)
                } else if (alignment === Qt.AlignLeft) {
                    root.value = root.y - offset + root.height/2
                //Center
                } else {
                    if (root.alignment & Qt.AlignRight) {
                        root.value = (root.parent.height/2 - root.y + offset)*2  - root.height
                    } else if (root.alignment & Qt.AlignLeft) {
                        root.value = (root.y - offset - root.parent.height/2)*2  + root.height
                    } else {
                        var value = root.y - root.parent.height/2 - offset + root.height/2
                        //Snap
                        if (Math.abs(value) < 5) {
                            root.value = 0
                        } else {
                            root.value = value
                        }
                    }
                }
            } else {
                if (root.alignment === Qt.AlignRight) {
                    root.value = root.parent.width - (root.x + offset + root.width/2)
                } else if (alignment === Qt.AlignLeft) {
                    root.value = root.x - offset + root.width/2
                //Center
                } else {
                    if (root.alignment & Qt.AlignRight) {
                        root.value = (root.parent.width/2 - root.x + offset)*2 - root.width
                    } else if (root.alignment & Qt.AlignLeft) {
                        root.value = (root.x - offset - root.parent.width/2)*2  + root.width
                    } else {
                        var value = root.x - root.parent.width/2 - offset + root.width/2
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

        PlasmaExtras.Highlight {
            anchors.fill: parent
            visible: parent.activeFocus
            hovered: true
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
