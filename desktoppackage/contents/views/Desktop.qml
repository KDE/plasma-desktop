/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kwindowsystem 1.0
import org.kde.plasma.activityswitcher 1.0 as ActivitySwitcher
import org.kde.plasma.shell 2.0 as Shell
import "../activitymanager"
import "../explorer"


Item {
    id: root

    property Item containment

    property QtObject widgetExplorer

    Connections {
        target: ActivitySwitcher.Backend
        function onShouldShowSwitcherChanged() {
            if (ActivitySwitcher.Backend.shouldShowSwitcher) {
                if (sidePanelStack.state != "activityManager") {
                    root.toggleActivityManager();
                }

            } else {
                if (sidePanelStack.state == "activityManager") {
                    root.toggleActivityManager();
                }

            }
        }
    }

    function toggleWidgetExplorer(containment) {
//         console.log("Widget Explorer toggled");

        if (sidePanelStack.state == "widgetExplorer") {
            sidePanelStack.state = "closed";
        } else {
            sidePanelStack.state = "widgetExplorer";
            sidePanelStack.setSource(Qt.resolvedUrl("../explorer/WidgetExplorer.qml"), {"containment": containment})
        }
    }

    // Qt has a bug where invoking a global shortcut steal focus from the
    // current window, which makes the activity switcher hide itself and
    // popup again when the user presses meta+q (instead of just hiding).
    // This 'timestamp' forbids the switcher to be toggled consecutively.
    //
    // The relevant patch to Qt is here:
    //     https://codereview.qt-project.org/#/c/143658/
    property int lastToggleActivityManagerTimestamp: 0

    function toggleActivityManager() {
        if (sidePanelStack.state == "activityManager") {
            sidePanelStack.state = "closed";
        } else {
            var currentTimestamp = new Date().getTime() / 1000;

            if (currentTimestamp - lastToggleActivityManagerTimestamp > 1) {
                sidePanelStack.state = "activityManager";
                sidePanelStack.setSource(Qt.resolvedUrl("../activitymanager/ActivityManager.qml"))
                lastToggleActivityManagerTimestamp = currentTimestamp;
            }
        }
    }

    KWindowSystem {
        id: kwindowsystem
    }

    Timer {
        id: pendingUninstallTimer
        // keeps track of the applets the user wants to uninstall
        property var applets: []

        interval: 60000 // one minute
        onTriggered: {
            for (var i = 0, length = applets.length; i < length; ++i) {
                widgetExplorer.uninstall(applets[i])
            }
            applets = []

            if (sidePanelStack.state !== "widgetExplorer" && widgetExplorer) {
                widgetExplorer.destroy()
                widgetExplorer = null
            }
        }
    }

    PlasmaCore.Dialog {
        id: sidePanel
        location: Qt.application.layoutDirection === Qt.RightToLeft ? PlasmaCore.Types.RightEdge : PlasmaCore.Types.LeftEdge
        type: PlasmaCore.Dialog.Dock
        flags: Qt.WindowStaysOnTopHint

        hideOnWindowDeactivate: true

        x: {
            var result = desktop.x;
            if (!containment) {
                return result;
            }

            var rect = containment.availableScreenRect;
            result += rect.x;

            if (Qt.application.layoutDirection === Qt.RightToLeft) {
                result += rect.width - sidePanel.width;
            }

            return result;
        }
        y: desktop.y + (containment ? containment.availableScreenRect.y : 0)

        onVisibleChanged: {
            if (!visible) {
                sidePanelStack.state = "closed";
                ActivitySwitcher.Backend.shouldShowSwitcher = false;
            }

            lastToggleActivityManagerTimestamp = new Date().getTime() / 1000;
        }

        mainItem: Loader {
            id: sidePanelStack
            asynchronous: true
            width: item ? item.width: 0
            height: containment ? containment.availableScreenRect.height - sidePanel.margins.top - sidePanel.margins.bottom : 1000
            state: "closed"

            LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
            LayoutMirroring.childrenInherit: true

            onLoaded: {
                if (sidePanelStack.item) {
                    item.closed.connect(function(){sidePanelStack.state = "closed";});

                    if (sidePanelStack.state == "activityManager") {
                        sidePanelStack.item.showSwitcherOnly =
                            ActivitySwitcher.Backend.shouldShowSwitcher
                        sidePanel.hideOnWindowDeactivate = Qt.binding(function() {
                            return !ActivitySwitcher.Backend.shouldShowSwitcher
                                && !sidePanelStack.item.showingDialog;
                        })
                        sidePanelStack.item.forceActiveFocus();
                    } else if (sidePanelStack.state == "widgetExplorer"){
                        sidePanel.hideOnWindowDeactivate = Qt.binding(function() { return sidePanelStack.item && !sidePanelStack.item.preventWindowHide; })
                        sidePanel.opacity = Qt.binding(function() { return sidePanelStack.item ? sidePanelStack.item.opacity : 1 })
                        sidePanel.outputOnly = Qt.binding(function() { return sidePanelStack.item && sidePanelStack.item.outputOnly })
                    } else {
                        sidePanel.hideOnWindowDeactivate = true;
                    }
                }
                sidePanel.visible = true;
                kwindowsystem.forceActivateWindow(sidePanel)
            }
            onStateChanged: {
                if (sidePanelStack.state == "closed") {
                    sidePanel.visible = false;
                    sidePanelStack.source = ""; //unload all elements
                }
            }
        }
    }

    Connections {
        target: containment
        function onAvailableScreenRectChanged() {
            sidePanel.requestActivate();
        }
    }

    onContainmentChanged: {
        if (containment == null) {
            return;
        }

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

        if (internal.oldContainment != null && internal.oldContainment != containment) {
            switchAnim.running = true;
        } else {
            containment.anchors.left = root.left;
            containment.anchors.top = root.top;
            containment.anchors.right = root.right;
            containment.anchors.bottom = root.bottom;
            if (internal.oldContainment) {
                internal.oldContainment.visible = false;
            }
            internal.oldContainment = containment;
        }
    }

    //some properties that shouldn't be accessible from elsewhere
    QtObject {
        id: internal;

        property Item oldContainment: null;
        property Item newContainment: null;
    }

    SequentialAnimation {
        id: switchAnim
        ScriptAction {
            script: {
                if (containment) {
                    containment.anchors.left = undefined;
                    containment.anchors.top = undefined;
                    containment.anchors.right = undefined;
                    containment.anchors.bottom = undefined;
                    containment.z = 1;
                    containment.x = root.width;
                }
                if (internal.oldContainment) {
                    internal.oldContainment.anchors.left = undefined;
                    internal.oldContainment.anchors.top = undefined;
                    internal.oldContainment.anchors.right = undefined;
                    internal.oldContainment.anchors.bottom = undefined;
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
                duration: PlasmaCore.Units.veryLongDuration
                easing.type: Easing.InOutQuad
            }
            NumberAnimation {
                target: internal.newContainment
                properties: "x"
                to: 0
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
        ScriptAction {
            script: {
                if (internal.oldContainment) {
                    internal.oldContainment.visible = false;
                }
                if (containment) {
                    containment.anchors.left = root.left;
                    containment.anchors.top = root.top;
                    containment.anchors.right = root.right;
                    containment.anchors.bottom = root.bottom;
                    internal.oldContainment = containment;
                }
            }
        }
    }


    Component.onCompleted: {
        //configure the view behavior
        desktop.windowType = Shell.Desktop.Desktop;
    }
}
