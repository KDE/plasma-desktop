/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Effects
import QtQuick.Layouts

import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components as PC
import org.kde.kwindowsystem
import org.kde.plasma.activityswitcher as ActivitySwitcher
import "../activitymanager"
import "../explorer"
import org.kde.kirigami as Kirigami

Item {
    id: root

    property Item containment

    property QtObject widgetExplorer

    Connections {
        target: ActivitySwitcher.Backend
        function onShouldShowSwitcherChanged(): void {
            if (ActivitySwitcher.Backend.shouldShowSwitcher) {
                if (sidePanelStack.state !== "activityManager") {
                    root.toggleActivityManager();
                }

            } else {
                if (sidePanelStack.state === "activityManager") {
                    root.toggleActivityManager();
                }

            }
        }
    }

    function toggleWidgetExplorer(containment) {

        if (sidePanelStack.state === "widgetExplorer") {
            sidePanelStack.state = "closed";
        } else {
            sidePanelStack.state = "widgetExplorer";
            sidePanelStack.setSource(Qt.resolvedUrl("../explorer/WidgetExplorer.qml"), {
                containment,
                sidePanel,
            });
        }
    }

    function toggleActivityManager() {
        if (sidePanelStack.state === "activityManager") {
            sidePanelStack.state = "closed";
        } else {
            sidePanelStack.state = "activityManager";
            sidePanelStack.setSource(Qt.resolvedUrl("../activitymanager/ActivityManager.qml"));
        }
    }


    readonly property rect editModeRect: {
        if (!containment) {
            return Qt.rect(0,0,0,0);
        }
        let screenRect = desktop.strictAvailableScreenRect;
        let panelConfigRect = Qt.rect(0,0,0,0);

        if (containment.plasmoid.corona.panelBeingConfigured
            && containment.plasmoid.corona.panelBeingConfigured.screenToFollow === desktop.screenToFollow) {
            panelConfigRect = containment.plasmoid.corona.panelBeingConfigured.relativeConfigRect;
        }

        if (panelConfigRect.width <= 0) {
            ; // Do nothing
        } else if (panelConfigRect.x > width - (panelConfigRect.x + panelConfigRect.width)) {
            screenRect = Qt.rect(screenRect.x, screenRect.y, panelConfigRect.x - screenRect.x, screenRect.height);
        } else {
            const diff = Math.max(0, panelConfigRect.x + panelConfigRect.width - screenRect.x);
            screenRect = Qt.rect(Math.max(screenRect.x, panelConfigRect.x + panelConfigRect.width), screenRect.y, screenRect.width - diff, screenRect.height);
        }

        if (sidePanel.visible) {
            if (sidePanel.sideBarOnRightEdge) {
                screenRect = Qt.rect(screenRect.x, screenRect.y, screenRect.width - sidePanel.width, screenRect.height);
            } else {
                screenRect = Qt.rect(screenRect.x + sidePanel.width, screenRect.y, screenRect.width - sidePanel.width, screenRect.height);
            }
        }
        return screenRect;
    }

    MouseArea {
        id: desktopMouseArea
        anchors.fill: parent
        onClicked: containment.plasmoid.corona.editMode = false
    }

    MouseArea {
        id: containmentParent
        x: editModeLoader.item ? editModeLoader.item.centerX - width / 2 : 0
        y: editModeLoader.item ? editModeLoader.item.centerY - height / 2 : 0
        width: root.width
        height: root.height
        readonly property real extraScale: desktop.configuredPanel || sidePanel.visible ? 0.95 : 0.9
        property real scaleFactor: Math.min(editModeRect.width / root.width, editModeRect.height / root.height) * extraScale
        scale: containment?.plasmoid.corona.editMode ? scaleFactor : 1
    }

    Loader {
        id: editModeLoader
        anchors.fill: parent
        sourceComponent: DesktopEditMode {}
        active: containment?.plasmoid.corona.editMode || editModeUiTimer.running
        Timer {
            id: editModeUiTimer
            property bool editMode: containment?.plasmoid.corona.editMode ?? false
            onEditModeChanged: restart()
            interval: Kirigami.Units.longDuration
        }
    }

    Loader {
        id: wallpaperColors

        active: root.containment && root.containment.wallpaper && desktop.usedInAccentColor
        asynchronous: true

        sourceComponent: Kirigami.ImageColors {
            id: imageColors
            source: root.containment.wallpaper

            readonly property color backgroundColor: Kirigami.Theme.backgroundColor
            readonly property color textColor: Kirigami.Theme.textColor

            // Ignore the initial dominant color
            onPaletteChanged: {
                if (!Qt.colorEqual(root.containment.wallpaper.accentColor, "transparent")) {
                    desktop.accentColor = root.containment.wallpaper.accentColor;
                }
                if (this.palette.length === 0) {
                    desktop.accentColor = "transparent";
                } else {
                    desktop.accentColor = this.dominant;
                }
            }

            Kirigami.Theme.inherit: false
            Kirigami.Theme.backgroundColor: backgroundColor
            Kirigami.Theme.textColor: textColor

            onBackgroundColorChanged: Qt.callLater(update)
            onTextColorChanged: Qt.callLater(update)

            readonly property Connections __repaintConnection: Connections {
                target: root.containment.wallpaper
                function onAccentColorChanged() {
                    if (Qt.colorEqual(root.containment.wallpaper.accentColor, "transparent")) {
                        imageColors.update();
                    } else {
                        imageColors.paletteChanged();
                    }
                }
            }
        }
    }

    Timer {
        id: pendingUninstallTimer
        // keeps track of the applets the user wants to uninstall
        property var applets: []
        function uninstall() {
            for (const applet of applets) {
                widgetExplorer.uninstall(applet);
            }
            applets = [];

            if (sidePanelStack.state !== "widgetExplorer" && widgetExplorer) {
                widgetExplorer.destroy();
                widgetExplorer = null;
            }
        }

        interval: 60000 // one minute
        onTriggered: uninstall()
    }

    PlasmaCore.Dialog {
        id: sidePanel

        readonly property bool sideBarOnRightEdge: {
            if (!sidePanelStack.active) {
                return false;
            }

            const item = sidePanelStack.item;
            if (!item) {
                return false;
            }

            const rightEdgeParent = (item.containment
                                     && item.containment !== containment.plasmoid
                                     && item.containment.location == PlasmaCore.Types.RightEdge);

            return rightEdgeParent || Qt.application.layoutDirection === Qt.RightToLeft;
        }

        location: sideBarOnRightEdge ? PlasmaCore.Types.RightEdge : PlasmaCore.Types.LeftEdge
        type: PlasmaCore.Dialog.Dock
        flags: Qt.WindowStaysOnTopHint

        hideOnWindowDeactivate: true

        x: {
            let result = desktop.x;
            if (!containment) {
                return result;
            }

            const rect = containment.plasmoid.availableScreenRect;
            result += rect.x;

            if (sideBarOnRightEdge) {
                result += rect.width - sidePanel.width;
            }

            return result;
        }
        y: desktop.y + (containment ? containment.plasmoid.availableScreenRect.y : 0)

        onVisibleChanged: {
            if (!visible) {
                sidePanelStack.state = "closed";
                ActivitySwitcher.Backend.shouldShowSwitcher = false;
            }
        }

        mainItem: Loader {
            id: sidePanelStack
            asynchronous: true
            width: item ? item.width : 0
            height: containment ? containment.plasmoid.availableScreenRect.height - sidePanel.margins.top - sidePanel.margins.bottom : 1000
            state: "closed"

            function bindingWithItem(callback: var, defaults: var): var {
                return Qt.binding(() => {
                    const item = this.item;
                    return item !== null ? callback(item) : defaults;
                });
            }

            LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
            LayoutMirroring.childrenInherit: true

            onLoaded: {
                if (item) {
                    item.closed.connect(() => {
                        state = "closed";
                    });

                    switch (state) {
                    case "activityManager":
                        item.showSwitcherOnly = ActivitySwitcher.Backend.shouldShowSwitcher;
                        sidePanel.hideOnWindowDeactivate = bindingWithItem(
                            item => !ActivitySwitcher.Backend.shouldShowSwitcher && !item.showingDialog,
                            false,
                        );
                        item.forceActiveFocus();
                        break;
                    case "widgetExplorer":
                        sidePanel.hideOnWindowDeactivate = bindingWithItem(item => !item.preventWindowHide, false);
                        sidePanel.opacity = bindingWithItem(item => item.opacity, 1);
                        sidePanel.outputOnly = bindingWithItem(item => item.outputOnly, false);
                        break;
                    default:
                        sidePanel.hideOnWindowDeactivate = true;
                        break;
                    }
                }
                sidePanel.visible = true;
                if (KWindowSystem.isPlatformX11) {
                    KX11Extras.forceActiveWindow(sidePanel);
                }
            }
            onStateChanged: {
                if (state === "closed") {
                    sidePanel.visible = false;
                    source = ""; //unload all elements
                }
            }
        }
    }

    Connections {
        target: containment?.plasmoid ?? null
        function onAvailableScreenRectChanged() {
            if (sidePanel.visible) {
                sidePanel.requestActivate();
            }
        }
    }

    onContainmentChanged: {
        if (containment === null) {
            return;
        }

        containment.parent = containmentParent

        if (switchAnim.running) {
            //If the animation was still running, stop it and reset
            //everything so that a consistent state can be kept
            switchAnim.running = false;
            internal.newContainment.visible = false;
            internal.oldContainment.visible = false;
            internal.oldContainment = null;
        }

        internal.newContainment = containment;
        containment.visible = true;

        if (internal.oldContainment !== null && internal.oldContainment !== containment) {
            switchAnim.running = true;
        } else {
            containment.anchors.left = containmentParent.left;
            containment.anchors.top = containmentParent.top;
            containment.anchors.right = containmentParent.right;
            containment.anchors.bottom = containmentParent.bottom;
            if (internal.oldContainment) {
                internal.oldContainment.visible = false;
            }
            internal.oldContainment = containment;
        }
    }

    //some properties that shouldn't be accessible from elsewhere
    QtObject {
        id: internal

        property Item oldContainment: null
        property Item newContainment: null
    }

    SequentialAnimation {
        id: switchAnim
        ScriptAction {
            script: {
                if (containment) {
                    containment.z = 1;
                    containment.x = root.width;
                }
                if (internal.oldContainment) {
                    internal.oldContainment.z = 0;
                    internal.oldContainment.x = 0;
                }
            }
        }
        ParallelAnimation {
            NumberAnimation {
                target: internal.oldContainment
                properties: "x"
                to: internal.newContainment != null ? -root.width : 0
                duration: Kirigami.Units.veryLongDuration
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: internal.newContainment
                properties: "x"
                to: 0
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        ScriptAction {
            script: {
                if (internal.oldContainment) {
                    internal.oldContainment.visible = false;
                }
                if (containment) {
                    internal.oldContainment = containment;
                }
            }
        }
    }

    Loader {
        id: previewBannerLoader

        function shouldBeActive(): bool {
            // Loader::active is true by default at the time of creation, so
            // it shouldn't be used in other bindings as a guard.
            return root.containment !== null && (desktop.showPreviewBanner ?? false);
        }

        readonly property point pos: root.containment?.plasmoid.availableScreenRegion, shouldBeActive() && item !== null
            ? root.containment.adjustToAvailableScreenRegion(
                root.containment.width + root.containment.x - item.width - Kirigami.Units.largeSpacing,
                root.containment.height + root.containment.y - item.height - Kirigami.Units.largeSpacing,
                item.width + Kirigami.Units.largeSpacing,
                item.height + Kirigami.Units.largeSpacing)
            : Qt.point(0, 0)

        x: pos.x
        y: pos.y
        z: (root.containment?.z ?? 0) + 1
        active: shouldBeActive()
        visible: active
        source: "PreviewBanner.qml"
    }
}
