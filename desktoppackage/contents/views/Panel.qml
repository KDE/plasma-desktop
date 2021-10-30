/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQml 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.taskmanager 0.1 as TaskManager
import org.kde.kwindowsystem 1.0

Item {
    id: root

    property Item containment

    property alias panelMask: privateSwapper.mask

    QtObject {
        id: privateSwapper
        property string completedState: ""
        // Work around the fact that we can't use a ternary if in an alias
        readonly property var mask: {
            if (completedState == "opaque") {
                return opaqueItem.mask
            } else {
                return translucentItem.mask
            }
        }
    }

    readonly property bool verticalPanel: containment && containment.formFactor === PlasmaCore.Types.Vertical

    readonly property real spacingAtMinSize: Math.round(Math.max(1, (verticalPanel ? root.width : root.height) - units.iconSizes.smallMedium)/2)
    PlasmaCore.FrameSvgItem {
        id: thickPanelSvg
        visible: false
        prefix: 'thick'
        imagePath: "widgets/panel-background"
    }
    readonly property int topPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.top, spacingAtMinSize));
    readonly property int bottomPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.bottom, spacingAtMinSize));
    readonly property int leftPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.left, spacingAtMinSize));
    readonly property int rightPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.right, spacingAtMinSize));

    TaskManager.VirtualDesktopInfo {
        id: virtualDesktopInfo
    }

    TaskManager.ActivityInfo {
        id: activityInfo
    }

    PlasmaCore.SortFilterModel {
        id: visibleWindowsModel
        filterRole: 'IsMinimized'
        filterRegExp: 'false'
        sourceModel: TaskManager.TasksModel {
            filterByVirtualDesktop: true
            filterByActivity: true
            filterNotMaximized: true
            filterByScreen: true
            filterHidden: true

            screenGeometry: panel.screenGeometry
            virtualDesktop: virtualDesktopInfo.currentDesktop
            activity: activityInfo.currentActivity

            id: tasksModel
            groupMode: TaskManager.TasksModel.GroupDisabled
        }
    }

    KWindowSystem {
        id: kwindowsystem
    }

    PlasmaCore.FrameSvgItem {
        id: translucentItem
        enabledBorders: panel.enabledBorders
        anchors.fill: parent

        imagePath: containment && containment.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "widgets/panel-background"
    }

    PlasmaCore.FrameSvgItem {
        id: opaqueItem
        enabledBorders: panel.enabledBorders
        anchors.fill: parent

        imagePath: containment && containment.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "solid/widgets/panel-background"
    }

    transitions: [
        Transition {
            from: "*"
            to: "transparent"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        translucentItem.visible = true
                    }
                }
                NumberAnimation {
                    target: opaqueItem
                    properties: "opacity"
                    to: 0
                    duration: PlasmaCore.Units.veryLongDuration
                    easing.type: Easing.InOutQuad
                }
                ScriptAction {
                    script: {
                        opaqueItem.visible = false
                        privateSwapper.completedState = "transparent"
                        root.panelMaskChanged()
                    }
                }
            }
        },
        Transition {
            from: "*"
            to: "opaque"
            SequentialAnimation {
                ScriptAction {
                    script: {
                        opaqueItem.visible = true
                    }
                }
                NumberAnimation {
                    target: opaqueItem
                    properties: "opacity"
                    to: 1
                    duration: PlasmaCore.Units.veryLongDuration
                    easing.type: Easing.InOutQuad
                }
                ScriptAction {
                    script: {
                        translucentItem.visible = false
                        privateSwapper.completedState = "opaque"
                        root.panelMaskChanged()
                    }
                }
            }
        }
    ]

    Component.onCompleted: {
        state = Qt.binding(function() {
            let mstate = '';
            if (panel.opacityMode == 0) {
                mstate = visibleWindowsModel.count > 0 && !kwindowsystem.showingDesktop? "opaque" : "transparent"
            } else if (panel.opacityMode == 1) {
                mstate = "opaque"
            } else {
                mstate = "transparent"
            }
            if (containment) {
                if (mstate == 'opaque') {
                    containment.containmentDisplayHints |= PlasmaCore.Types.DesktopFullyCovered;
                } else {
                    containment.containmentDisplayHints &= ~PlasmaCore.Types.DesktopFullyCovered;
                }
            }

            return mstate;
        })
    }
    state: ""
    states: [
        State { name: "opaque" },
        State { name: "transparent" }
    ]

    function adjustPrefix() {
        if (!containment) {
            return "";
        }
        var pre;
        switch (containment.location) {
        case PlasmaCore.Types.LeftEdge:
            pre = "west";
            break;
        case PlasmaCore.Types.TopEdge:
            pre = "north";
            break;
        case PlasmaCore.Types.RightEdge:
            pre = "east";
            break;
        case PlasmaCore.Types.BottomEdge:
            pre = "south";
            break;
        default:
            pre = "";
            break;
        }
        translucentItem.prefix = opaqueItem.prefix = [pre, ""];
    }

    onContainmentChanged: {
        if (!containment) {
            return;
        }
        containment.parent = containmentParent;
        containment.visible = true;
        containment.anchors.fill = containmentParent;
        containment.locationChanged.connect(adjustPrefix);
        adjustPrefix();
    }

    Binding {
        target: panel
        property: "length"
        when: containment
        value: {
            if (!containment) {
                return;
            }
            if (verticalPanel) {
                return containment.Layout.preferredHeight
            } else {
                return containment.Layout.preferredWidth
            }
        }
        restoreMode: Binding.RestoreBinding
    }

    Binding {
        target: panel
        property: "backgroundHints"
        when: containment
        value: {
            if (!containment) {
                return;
            }

            return containment.backgroundHints; 
        }
        restoreMode: Binding.RestoreBinding
    }

    Item {
        id: containmentParent
        anchors.fill: parent
    }
}
