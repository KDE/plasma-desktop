/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Window
import QtQuick.Layouts
import QtQml

import org.kde.plasma.core as PlasmaCore
import org.kde.ksvg as KSvg
import org.kde.taskmanager as TaskManager
import org.kde.kwindowsystem
import org.kde.kirigami as Kirigami
import org.kde.plasma.shell.panel as Panel

import org.kde.plasma.plasmoid

Item {
    id: root

    property Item containment

    property bool floatingPrefix: floatingPanelSvg.usedPrefix === "floating"
    readonly property bool verticalPanel: containment?.plasmoid?.formFactor === PlasmaCore.Types.Vertical

    readonly property real spacingAtMinSize: Math.floor(Math.max(1, panel.thickness - Kirigami.Units.iconSizes.smallMedium)/2)
    KSvg.FrameSvgItem {
        id: thickPanelSvg
        visible: false
        prefix: 'thick'
        imagePath: "widgets/panel-background"
    }
    KSvg.FrameSvgItem {
        id: floatingPanelSvg
        visible: false
        prefix: ['floating', '']
        imagePath: "widgets/panel-background"
    }

    readonly property bool rightEdge: containment?.plasmoid?.location === PlasmaCore.Types.RightEdge
    readonly property bool bottomEdge: containment?.plasmoid?.location === PlasmaCore.Types.BottomEdge

    readonly property int bottomFloatingPadding: Math.round(fixedBottomFloatingPadding * floatingness)
    readonly property int leftFloatingPadding: Math.round(fixedLeftFloatingPadding * floatingness)
    readonly property int rightFloatingPadding: Math.round(fixedRightFloatingPadding * floatingness)
    readonly property int topFloatingPadding: Math.round(fixedTopFloatingPadding * floatingness)


    // NOTE: Many of the properties in this file are accessed directly in C++ PanelView!
    // If you change these, make sure to also correct the related code in panelview.cpp.
    readonly property int fixedBottomFloatingPadding: floating && (floatingPrefix ? floatingPanelSvg.fixedMargins.bottom : 8)
    readonly property int fixedLeftFloatingPadding: floating && (floatingPrefix ? floatingPanelSvg.fixedMargins.left   : 8)
    readonly property int fixedRightFloatingPadding: floating && (floatingPrefix ? floatingPanelSvg.fixedMargins.right  : 8)
    readonly property int fixedTopFloatingPadding: floating && (floatingPrefix ? floatingPanelSvg.fixedMargins.top    : 8)

    readonly property int topPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.top + Kirigami.Units.smallSpacing, spacingAtMinSize));
    readonly property int bottomPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.bottom + Kirigami.Units.smallSpacing, spacingAtMinSize));
    readonly property int leftPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.left + Kirigami.Units.smallSpacing, spacingAtMinSize));
    readonly property int rightPadding: Math.round(Math.min(thickPanelSvg.fixedMargins.right + Kirigami.Units.smallSpacing, spacingAtMinSize));

    readonly property int minPanelHeight: translucentItem.minimumDrawingHeight
    readonly property int minPanelWidth: translucentItem.minimumDrawingWidth

    // This value is read from panelview.cpp which needs it to decide which border should be enabled
    property real topShadowMargin: -floatingTranslucentItem.y
    property real leftShadowMargin: -floatingTranslucentItem.x
    property real rightShadowMargin: -(width - floatingTranslucentItem.width - floatingTranslucentItem.x)
    property real bottomShadowMargin: -(height - floatingTranslucentItem.height - floatingTranslucentItem.y)

    property var panelMask: floatingness === 0 ? (panelOpacity === 1 ? opaqueItem.mask : translucentItem.mask) : (panelOpacity === 1 ? floatingOpaqueItem.mask : floatingTranslucentItem.mask)

    // The point is read from panelview.cpp and is used as an offset for the mask
    readonly property point floatingTranslucentItemOffset: Qt.point(floatingTranslucentItem.x, floatingTranslucentItem.y)

    TaskManager.VirtualDesktopInfo {
        id: virtualDesktopInfo
    }

    TaskManager.ActivityInfo {
        id: activityInfo
    }

    // We need to have a little gap between the raw visibleWindowsModel count
    // and actually determining if a window is touching.
    // This is because certain dialog windows start off with a position of (screenwidth/2, screenheight/2)
    // and they register as "touching" in the split-second before KWin can place them correctly.
    // This avoids the panel flashing if it is auto-hide etc and such a window is shown.
    // Examples of such windows: properties of a file on desktop, or portal "open with" dialog
    property bool touchingWindow: false
    property bool touchingWindowDirect: visibleWindowsModel.count > 0
    property bool showingDesktop: KWindowSystem.showingDesktop
    Timer {
        id: touchingWindowDebounceTimer
        interval: 10  // ms, I find that this value is enough while not causing unresponsiveness while dragging windows close
        onTriggered: root.touchingWindow = !KWindowSystem.showingDesktop && root.touchingWindowDirect
    }
    onTouchingWindowDirectChanged: touchingWindowDebounceTimer.start()
    onShowingDesktopChanged: touchingWindowDebounceTimer.start()

    TaskManager.TasksModel {
        id: visibleWindowsModel
        filterByVirtualDesktop: true
        filterByActivity: true
        filterByScreen: false
        filterByRegion: TaskManager.RegionFilterMode.Intersect
        filterHidden: true
        filterMinimized: true

        screenGeometry: panel.screenGeometry
        virtualDesktop: virtualDesktopInfo.currentDesktop
        activity: activityInfo.currentActivity

        groupMode: TaskManager.TasksModel.GroupDisabled

        Binding on regionGeometry {
            delayed: true
            value: panel.width, panel.height, panel.x, panel.y, panel.dogdeGeometryByDistance(panel.visibilityMode === Panel.Global.DodgeWindows ? -1 : 1) // +1 is for overlap detection, -1 is for snapping to panel
        }
    }

    Connections {
        target: root.containment?.plasmoid ?? null
        function onActivated() {
            if (root.containment.plasmoid.status === PlasmaCore.Types.AcceptingInputStatus) {
                root.containment.plasmoid.status = PlasmaCore.Types.PassiveStatus;
            } else {
                root.containment.plasmoid.status = PlasmaCore.Types.AcceptingInputStatus;
            }
        }
    }

    // Floatingness is a value in [0, 1] that's multiplied to the floating margin; 0: not floating, 1: floating, between 0 and 1: animation between the two states
    readonly property int floatingnessAnimationDuration: Kirigami.Units.longDuration
    property double floatingnessTarget: 0.0 // The animation is handled in panelview.cpp for efficiency
    property double floatingness: 0.0

    // PanelOpacity is a value in [0, 1] that's used as the opacity of the opaque elements over the transparent ones; values between 0 and 1 are used for animations
    property double panelOpacity
    Behavior on panelOpacity {
        NumberAnimation {
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }

    KSvg.FrameSvgItem {
        id: translucentItem
        visible: floatingness === 0 && panelOpacity !== 1
        enabledBorders: panel.enabledBorders
        anchors.fill: floatingTranslucentItem
        imagePath: containment?.plasmoid?.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "widgets/panel-background"
    }
    KSvg.FrameSvgItem {
        id: floatingTranslucentItem
        visible: floatingness !== 0 && panelOpacity !== 1
        x: root.rightEdge ? fixedLeftFloatingPadding + fixedRightFloatingPadding * (1 - floatingness) : leftFloatingPadding
        y: root.bottomEdge ? fixedTopFloatingPadding + fixedBottomFloatingPadding * (1 - floatingness) : topFloatingPadding
        width: verticalPanel ? panel.thickness : parent.width - leftFloatingPadding - rightFloatingPadding
        height: verticalPanel ? parent.height - topFloatingPadding - bottomFloatingPadding : panel.thickness

        imagePath: containment?.plasmoid?.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "widgets/panel-background"
    }
    KSvg.FrameSvgItem {
        id: floatingOpaqueItem
        visible: floatingness !== 0 && panelOpacity !== 0
        opacity: panelOpacity
        anchors.fill: floatingTranslucentItem
        imagePath: containment?.plasmoid?.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "solid/widgets/panel-background"
    }
    KSvg.FrameSvgItem {
        id: opaqueItem
        visible: panelOpacity !== 0 && floatingness === 0
        opacity: panelOpacity
        enabledBorders: panel.enabledBorders
        anchors.fill: floatingTranslucentItem
        imagePath: containment?.plasmoid?.backgroundHints === PlasmaCore.Types.NoBackground ? "" : "solid/widgets/panel-background"
    }

    Keys.onEscapePressed: {
        root.parent.focus = false
    }

    property bool isOpaque: panel.opacityMode === Panel.Global.Opaque
    property bool isTransparent: panel.opacityMode === Panel.Global.Translucent
    property bool isAdaptive: panel.opacityMode === Panel.Global.Adaptive
    property bool floating: panel.floating
    property bool hasCompositing: KWindowSystem.isPlatformX11 ? KX11Extras.compositingActive : true
    property var stateTriggers: [floating, touchingWindow, isOpaque, isAdaptive, isTransparent, hasCompositing, containment, panel.floatingApplets]
    onStateTriggersChanged: {
        let opaqueApplets = false
        let floatingApplets = false
        if ((!floating || touchingWindow) && (isOpaque || (touchingWindow && isAdaptive))) {
            panelOpacity = 1
            opaqueApplets = true
            floatingnessTarget = 0
            floatingApplets = (panel.floatingApplets && !floating)
        } else if ((!floating || touchingWindow) && (isTransparent || (!touchingWindow && isAdaptive))) {
            panelOpacity = 0
            floatingnessTarget = 0
            floatingApplets = (panel.floatingApplets && !floating)
        } else if ((floating && !touchingWindow) && (isTransparent || isAdaptive)) {
            panelOpacity = 0
            floatingnessTarget = 1
            floatingApplets = true
        } else if (floating && !touchingWindow && isOpaque) {
            panelOpacity = 1
            opaqueApplets = true
            floatingnessTarget = 1
            floatingApplets = true
        }

        // Exceptions: panels with not NormalPanel visibilityMode
        // should never de-float, and we should not have transparent
        // panels when on X11 with compositing not active.
        if (panel.visibilityMode != Panel.Global.NormalPanel && floating) {
            floatingnessTarget = 1
            floatingApplets = true
        }
        if (!KWindowSystem.isPlatformWayland && !KX11Extras.compositingActive) {
            opaqueApplets = false
            panelOpacity = 0
        }

        // Not using panelOpacity to check as it has a NumberAnimation, and it will thus
        // be still read as the initial value here, before the animation starts.
        if (containment) {
            if (opaqueApplets) {
                containment.plasmoid.containmentDisplayHints |= PlasmaCore.Types.ContainmentPrefersOpaqueBackground
            } else {
                containment.plasmoid.containmentDisplayHints &= ~PlasmaCore.Types.ContainmentPrefersOpaqueBackground
            }
            if (floatingApplets) {
                containment.plasmoid.containmentDisplayHints |= PlasmaCore.Types.ContainmentPrefersFloatingApplets
            } else {
                containment.plasmoid.containmentDisplayHints &= ~PlasmaCore.Types.ContainmentPrefersFloatingApplets
            }
        }
    }

    function adjustPrefix() {
        if (!containment) {
            return "";
        }
        var pre;
        switch (containment.plasmoid.location) {
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
        containment.plasmoid.locationChanged.connect(adjustPrefix);
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
                if (containment.Layout.fillHeight) {
                    if (panel.lengthMode == Panel.Global.Custom) {
                        return panel.maximumHeight
                    } else {
                        return panel.screenGeometry.height
                    }
                }
                return containment.Layout.preferredHeight
            } else {
                if (containment.Layout.fillWidth) {
                    if (panel.lengthMode == Panel.Global.Custom) {
                        return panel.maximumWidth
                    } else {
                        return panel.screenGeometry.width
                    }
                }
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

            return containment.plasmoid.backgroundHints;
        }
        restoreMode: Binding.RestoreBinding
    }

    KSvg.FrameSvgItem {

        Accessible.name: i18n("Panel Focus Indicator")

        x: root.verticalPanel || !panel.activeFocusItem
            ? translucentItem.x
            : Math.max(panel.activeFocusItem.Kirigami.ScenePosition.x, panel.activeFocusItem.Kirigami.ScenePosition.x)
        y: root.verticalPanel && panel.activeFocusItem
            ? Math.max(panel.activeFocusItem.Kirigami.ScenePosition.y, panel.activeFocusItem.Kirigami.ScenePosition.y)
            : translucentItem.y

        width: panel.activeFocusItem
            ? (root.verticalPanel ? translucentItem.width : Math.min(panel.activeFocusItem.width, panel.activeFocusItem.width))
            : 0
        height: panel.activeFocusItem
            ? (root.verticalPanel ?  Math.min(panel.activeFocusItem.height, panel.activeFocusItem.height) : translucentItem.height)
            : 0

        visible: panel.active && panel.activeFocusItem

        imagePath: "widgets/tabbar"
        prefix: {
            if (!root.containment) {
                return "";
            }
            var prefix = ""
            switch (root.containment.plasmoid.location) {
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
        width: root.verticalPanel ? panel.thickness : root.width - fixedLeftFloatingPadding - fixedRightFloatingPadding
        height: root.verticalPanel ? root.height - fixedBottomFloatingPadding - fixedTopFloatingPadding : panel.thickness
    }
}
