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

    property bool floatingPrefix: floatingPanelSvg.usedPrefix === "floating"
    readonly property bool verticalPanel: containment && containment.formFactor === PlasmaCore.Types.Vertical

    readonly property real spacingAtMinSize: Math.round(Math.max(1, (verticalPanel ? root.width : root.height) - units.iconSizes.smallMedium)/2)
    PlasmaCore.FrameSvgItem {
        id: thickPanelSvg
        visible: false
        prefix: 'thick'
        imagePath: "widgets/panel-background"
    }
    PlasmaCore.FrameSvgItem {
        id: floatingPanelSvg
        visible: false
        prefix: ['floating', '']
        imagePath: "widgets/panel-background"
    }
    readonly property int topPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.top, spacingAtMinSize));
    readonly property int bottomPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.bottom, spacingAtMinSize));
    readonly property int leftPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.left, spacingAtMinSize));
    readonly property int rightPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.right, spacingAtMinSize));

    readonly property int bottomFloatingPadding: floating  ? (floatingPrefix ? floatingPanelSvg.fixedMargins.bottom : 8) : 0
    readonly property int leftFloatingPadding: floating    ? (floatingPrefix ? floatingPanelSvg.fixedMargins.left   : 8) : 0
    readonly property int rightFloatingPadding: floating   ? (floatingPrefix ? floatingPanelSvg.fixedMargins.right  : 8) : 0
    readonly property int topFloatingPadding: floating     ? (floatingPrefix ? floatingPanelSvg.fixedMargins.top    : 8) : 0

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

    // Floatingness is a value in [0, 1] that's multiplied to the floating margin; 0: not floating, 1: floating, between 0 and 1: animation between the two states
    property double floatingness
    // PanelOpacity is a value in [0, 1] that's used as the opacity of the opaque elements over the transparent ones; values between 0 and 1 are used for animations
    property double panelOpacity
    Behavior on floatingness {
        NumberAnimation {
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }
    Behavior on panelOpacity {
        NumberAnimation {
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }

    // This value is read from panelview.cpp and disables shadow for floating panels, as they'd be detached from the panel
    property bool hasShadows: floatingness === 0
    property var panelMask: floatingness === 0 ? (panelOpacity === 1 ? opaqueItem.mask : translucentItem.mask) : (panelOpacity === 1 ? floatingOpaqueItem.mask : floatingTranslucentItem.mask)

    // These two values are read from panelview.cpp and are used as an offset for the mask
    property int maskOffsetX: Math.round(leftFloatingPadding * floatingness)
    property int maskOffsetY: Math.round(topFloatingPadding * floatingness)

    PlasmaCore.FrameSvgItem {
        id: translucentItem
        visible: floatingness === 0 && panelOpacity !== 1
        enabledBorders: panel.enabledBorders
        anchors.fill: parent
        imagePath: containment && containment.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "widgets/panel-background"
    }
    PlasmaCore.FrameSvgItem {
        id: floatingTranslucentItem
        visible: floatingness !== 0 && panelOpacity !== 1
        anchors {
            fill: parent
            bottomMargin: Math.round(bottomFloatingPadding * floatingness)
            leftMargin: Math.round(leftFloatingPadding * floatingness)
            rightMargin: Math.round(rightFloatingPadding * floatingness)
            topMargin: Math.round(topFloatingPadding * floatingness)
        }
        imagePath: containment && containment.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "widgets/panel-background"
    }
    PlasmaCore.FrameSvgItem {
        id: floatingOpaqueItem
        visible: floatingness !== 0 && panelOpacity !== 0
        opacity: panelOpacity
        anchors {
            fill: parent
            bottomMargin: Math.round(bottomFloatingPadding * floatingness)
            leftMargin: Math.round(leftFloatingPadding * floatingness)
            rightMargin: Math.round(rightFloatingPadding * floatingness)
            topMargin: Math.round(topFloatingPadding * floatingness)
        }
        imagePath: containment && containment.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "solid/widgets/panel-background"
    }
    PlasmaCore.FrameSvgItem {
        id: opaqueItem
        visible: panelOpacity !== 0 && floatingness === 0
        opacity: panelOpacity
        enabledBorders: panel.enabledBorders
        anchors.fill: parent
        imagePath: containment && containment.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "solid/widgets/panel-background"
    }

    Keys.onEscapePressed: {
        root.parent.focus = false
    }

    property bool isOpaque: panel.opacityMode === 1
    property bool isTransparent: panel.opacityMode === 2
    property bool isAdaptive: panel.opacityMode === 0
    property bool floating: panel.floating
    readonly property bool screenCovered: visibleWindowsModel.count > 0 && !kwindowsystem.showingDesktop
    property var stateTriggers: [floating, screenCovered, isOpaque, isAdaptive, isTransparent]
    onStateTriggersChanged: {
        let oldState = root.state
        if ((!floating || screenCovered) && (isOpaque || (screenCovered && isAdaptive))) {
            panelOpacity = 1
            floatingness = 0
        } else if ((!floating || screenCovered) && (isTransparent || (!screenCovered && isAdaptive))) {
            panelOpacity = 0
            floatingness = 0
        } else if ((floating && !screenCovered) && (isTransparent || isAdaptive)) {
            panelOpacity = 0
            floatingness = 1
        } else if (floating && !screenCovered && isOpaque) {
            panelOpacity = 1
            floatingness = 1
        }
        if ((panelOpacity == 1) && containment) {
            containment.containmentDisplayHints |= PlasmaCore.Types.DesktopFullyCovered
        } else {
            containment.containmentDisplayHints &= ~PlasmaCore.Types.DesktopFullyCovered
        }
    }

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
        translucentItem.prefix = opaqueItem.prefix = floatingTranslucentItem.prefix = floatingOpaqueItem.prefix = [pre, ""];
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
        anchors.centerIn: isOpaque ? floatingOpaqueItem : floatingTranslucentItem
        width: root.width - leftFloatingPadding - rightFloatingPadding
        height: root.height - topFloatingPadding - bottomFloatingPadding
    }
}
