/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.1
import QtQml 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.taskmanager 0.1 as TaskManager
import org.kde.kwindowsystem 1.0
import org.kde.kirigami 2.15 as Kirigami

import org.kde.plasma.plasmoid 2.0

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

    Keys.onEscapePressed: {
        root.parent.focus = false
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

    Component.onCompleted: state = Qt.binding(() => panel.opacityMode === 0 ? (visibleWindowsModel.count > 0 && !kwindowsystem.showingDesktop ? "opaque" : "transparent")
                                                                            : (panel.opacityMode === 1 ? "opaque" : "transparent"))
    onStateChanged: {
        if (containment) {
            if (state === 'opaque') {
                containment.containmentDisplayHints |= PlasmaCore.Types.DesktopFullyCovered;

            } else {
                containment.containmentDisplayHints &= ~PlasmaCore.Types.DesktopFullyCovered;
            }
        }
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
        delayed: true
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

    Connections {
        target: containment
        function onActivated() {
            containment.status = PlasmaCore.Types.AcceptingInputStatus;
            // When the containment is set to AcceptingInputStatus he window will be given focus and
            // will try to give focus to an itemof the scene, but not the one we wnant. if we call immediately
            // forceActiveFocus on the one we want we'll have a race condition that won't happen if we do it later
            Qt.callLater(root.nextItemInFocusChain().forceActiveFocus);
        }
    }

    Connections {
        target: parent
        function onActiveFocusChanged() {
            if (!parent.activeFocus) {
                containment.status = PlasmaCore.Types.PassiveStatus
            }
        }
    }

    PlasmaCore.FrameSvgItem {
        x: root.verticalPanel || !panel.activeFocusItem
            ? 0
            : Math.max(panel.activeFocusItem.Kirigami.ScenePosition.x, panel.activeFocusItem.Kirigami.ScenePosition.x)
        y: root.verticalPanel && panel.activeFocusItem
            ? Math.max(panel.activeFocusItem.Kirigami.ScenePosition.y, panel.activeFocusItem.Kirigami.ScenePosition.y)
            : 0

        width: panel.activeFocusItem
            ? (root.verticalPanel ? root.width : Math.min(panel.activeFocusItem.width, panel.activeFocusItem.width))
            : 0
        height: panel.activeFocusItem
            ? (root.verticalPanel ?  Math.min(panel.activeFocusItem.height, panel.activeFocusItem.height) : root.height)
            : 0

        visible: panel.active && panel.activeFocusItem

        imagePath: "widgets/tabbar"
        prefix: {
            var prefix = ""
            switch (root.containment.location) {
                case PlasmaCore.Types.LeftEdge:
                    prefix = "west-active-tab";
                    break;
                case PlasmaCore.Types.TopEdge:
                    prefix = "north-active-tab";
                    break;
                case PlasmaCore.Types.RightEdge:
                    prefix = "east-active-tab";
                    break;
                default:
                    prefix = "south-active-tab";
            }
            if (!hasElementPrefix(prefix)) {
                prefix = "active-tab";
            }
            return prefix;
        }
    }
    Item {
        id: containmentParent
        anchors.fill: parent
    }
}
